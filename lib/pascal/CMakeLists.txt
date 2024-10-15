set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(pascal
)

target_link_libraries(pascal PRIVATE asm)

# target_include_directories(asm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
