cmake_minimum_required(VERSION 3.1.0)

# header file directory
include_directories(${PROJECT_ABS_TOP_DIR}/)
include_directories(${PROJECT_ABS_TOP_DIR}/include/)
include_directories(${SECP256K1_X64_DIR}/include/)

SET(TEST_HEADER
    ${PROJECT_ABS_TOP_DIR}/test/test.h
)

set(TEST_SRC
    simple_thread.h
    simple_thread.c
    test.c
    ${TEST_HEADER}
)

add_executable(secp256k1_test secp256k1_test.c ${TEST_SRC})
add_test(SECP256K1_TEST secp256k1_test)

set(static_lib secp256k1_x64_static)
set(shared_lib secp256k1_x64_shared)

# dependency
# prefer shared library
if(ENABLE_SHARED)
    set(dep_lib ${shared_lib})
    target_compile_definitions(secp256k1_test PRIVATE BUILD_SHARED)
elseif(ENABLE_STATIC)
    set(dep_lib ${static_lib})
    target_compile_definitions(secp256k1_test PRIVATE BUILD_STATIC)
else()
    message(FATAL_ERROR "no library compiled")
endif()

set(test_DEP ${dep_lib})
if(HAVE_PTHREAD)
    list(APPEND test_DEP pthread)
endif()

target_link_libraries(secp256k1_test ${test_DEP})