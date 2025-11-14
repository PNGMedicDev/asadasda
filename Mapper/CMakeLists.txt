cmake_minimum_required(VERSION 3.15)
project(AdvancedMapper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Executable
add_executable(mapper
    src/main.cpp
    src/vuln_driver.cpp
    src/pe_parser.cpp
    src/mapper.cpp
    src/utils.cpp
    src/downloader.cpp
)

target_include_directories(mapper PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(mapper
    ntdll
    comdlg32
    wininet
)

# Output to build directory
set_target_properties(mapper PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build
)

# Copy to build directory
add_custom_command(TARGET mapper POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:mapper> ${CMAKE_SOURCE_DIR}/build/mapper.exe
    COMMENT "Copying mapper.exe to build directory"
)
