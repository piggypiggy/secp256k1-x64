cmake_minimum_required(VERSION 3.1.0)

if(x86_64)
    if(MSVC)
        set(SECP256K1_x86_64 ${SECP256K1_X64_DIR}/secp256k1/secp256k1-x86_64.asm)
        set(FP256_x86_64 ${SECP256K1_X64_DIR}/fp256/fp256-x86_64.asm)
    else()
        set(SECP256K1_x86_64 ${SECP256K1_X64_DIR}/secp256k1/secp256k1-x86_64.s)
        set(FP256_x86_64 ${SECP256K1_X64_DIR}/fp256/fp256-x86_64.s)
    endif()
endif()

message("C COMPILER   : ${CMAKE_C_COMPILER}")
message("ASM_COMPILER : ${CMAKE_ASM_COMPILER}")

# generate secp256k1 assembly code
add_custom_command (
    OUTPUT ${SECP256K1_x86_64}
    COMMAND ${PERL} ${SECP256K1_X64_DIR}/secp256k1/asm/secp256k1-x86_64.pl ${FLAVOUR} ${SECP256K1_x86_64}
)

# generate fp256 assembly code
add_custom_command (
    OUTPUT ${FP256_x86_64}
    COMMAND ${PERL} ${SECP256K1_X64_DIR}/fp256/asm/fp256-x86_64.pl ${FLAVOUR} ${FP256_x86_64}
)

# generate config.h
configure_file(${PROJECT_ABS_TOP_DIR}/config.h.in ${PROJECT_ABS_TOP_DIR}/config.h @ONLY)

# source file
set(RAND_SRC
    ${SECP256K1_X64_DIR}/rand/rand.c
    ${SECP256K1_X64_DIR}/rand/sys_rand.c
)

set(SECP256K1_SRC
    ${SECP256K1_X64_DIR}/secp256k1/secp256k1.c
    ${SECP256K1_x86_64}
)

set(FP256_SRC
    ${FP256_x86_64}
    ${SECP256K1_X64_DIR}/fp256/fp256.c
)

set(SECP256K1_X64_SRC
    ${FP256_SRC}
    ${RAND_SRC}
    ${SECP256K1_SRC}
)

set(SECP256K1_X64_HEADER
    ${PROJECT_ABS_TOP_DIR}/config.h
    ${PROJECT_ABS_TOP_DIR}/include/secp256k1_x64/common.h
    ${PROJECT_ABS_TOP_DIR}/include/secp256k1_x64/cpuid.h
    ${PROJECT_ABS_TOP_DIR}/include/secp256k1_x64/crypto.h
    ${PROJECT_ABS_TOP_DIR}/include/secp256k1_x64/fp256.h
    ${PROJECT_ABS_TOP_DIR}/include/secp256k1_x64/secp256k1.h
)

set(SECP256K1_X64_SRC
    ${SECP256K1_X64_HEADER}
    init.c
    common.c
    mem.c
    cpuid.c
    ${SECP256K1_X64_SRC}
)

# header file directory
include_directories(${PROJECT_ABS_TOP_DIR}/)
include_directories(${PROJECT_ABS_TOP_DIR}/include/)
include_directories(${SECP256K1_X64_DIR}/include/)

if(ENABLE_STATIC)
    # build static object
    add_library(secp256k1_x64_static_object OBJECT ${SECP256K1_X64_SRC})
    target_compile_definitions(secp256k1_x64_static_object PRIVATE BUILD_STATIC)
    #
    add_library(secp256k1_x64_static STATIC $<TARGET_OBJECTS:secp256k1_x64_static_object>)
    target_compile_definitions(secp256k1_x64_static PRIVATE BUILD_STATIC)
    # output directory
    if(WIN32)
        set_target_properties(secp256k1_x64_static PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY_STATIC})
    else()
        set_target_properties(secp256k1_x64_static PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
    endif()
    # rename
    set_target_properties(secp256k1_x64_static PROPERTIES OUTPUT_NAME "secp256k1_x64")
    # install library
    INSTALL(TARGETS secp256k1_x64_static ARCHIVE DESTINATION lib)
endif()

if(ENABLE_SHARED)
    # build shared object
    add_library(secp256k1_x64_shared_object OBJECT ${SECP256K1_X64_SRC})
    target_compile_definitions(secp256k1_x64_shared_object PRIVATE BUILD_SHARED)
    # PIC
    set_property(TARGET secp256k1_x64_shared_object PROPERTY POSITION_INDEPENDENT_CODE 1)
    
    #
    add_library(secp256k1_x64_shared SHARED $<TARGET_OBJECTS:secp256k1_x64_shared_object>)
    target_compile_definitions(secp256k1_x64_shared PRIVATE BUILD_SHARED)
    # output directory
    # TODO : LIBRARY_OUTPUT_DIRECTORY does not work in msvc
    if(WIN32)
        set_target_properties(secp256k1_x64_shared PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY_STATIC})
    else()
        set_target_properties(secp256k1_x64_shared PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
    endif()
    # rename
    set_target_properties(secp256k1_x64_shared PROPERTIES OUTPUT_NAME "secp256k1_x64")
    # install library
    INSTALL(TARGETS secp256k1_x64_shared LIBRARY DESTINATION lib)
endif()

# install headers
INSTALL(DIRECTORY ${PROJECT_ABS_TOP_DIR}/include/secp256k1_x64 DESTINATION include)
INSTALL(FILES ${PROJECT_ABS_TOP_DIR}/config.h DESTINATION include/secp256k1_x64)

