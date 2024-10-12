set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(asm
    ast.cpp
    parser.cpp
    tokenizer.cpp
)

target_link_libraries(asm PRIVATE labels)

# target_include_directories(asm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
