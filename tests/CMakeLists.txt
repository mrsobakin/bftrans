include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    ${PROJECT_NAME}_tests
    bflabels_parser.cpp
)

target_link_libraries(
    ${PROJECT_NAME}_tests
    GTest::gtest_main
    asm
    labels
)

target_include_directories(${PROJECT_NAME}_tests PRIVATE ..)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_tests)
