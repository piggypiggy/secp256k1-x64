cmake_minimum_required(VERSION 3.1.0)

project(speed)

include_directories(${PROJECT_ABS_TOP_DIR}/)
include_directories(${PROJECT_ABS_TOP_DIR}/include/)
include_directories(${SECP256K1_X64_DIR}/include/)

SET(SPEED_HEADER
    ${SECP256K1_X64_HEADER}
    ${PROJECT_ABS_TOP_DIR}/config.h
    ${PROJECT_ABS_TOP_DIR}/speed/speed_lcl.h
    ${PROJECT_ABS_TOP_DIR}/speed/speed.h
)

set(SPEED_SRC
    ${SPEED_HEADER}
    speed.c
    ../test/simple_thread.h
    ../test/simple_thread.c
    ../test/test.c
)

add_executable(speed ${SPEED_SRC})

set(static_lib secp256k1_x64_static)
set(shared_lib secp256k1_x64_shared)

# dependency
# prefer static library :)
if(ENABLE_STATIC)
    set(dep_lib ${static_lib})
    target_compile_definitions(speed PRIVATE BUILD_STATIC)
elseif(ENABLE_SHARED)
    set(dep_lib ${shared_lib})
    target_compile_definitions(speed PRIVATE BUILD_SHARED)
else()
    message(FATAL_ERROR "no library compiled")
endif()

set(speed_DEP ${dep_lib})
if(HAVE_PTHREAD)
    list(APPEND speed_DEP pthread)
endif()

target_link_libraries(speed ${speed_DEP})