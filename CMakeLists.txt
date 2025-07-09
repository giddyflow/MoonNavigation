cmake_minimum_required(VERSION 3.8)
project(MoonNavigation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Автоматическое определение пути к данным
set(INPUT_FOLDER "${CMAKE_SOURCE_DIR}/MoonNavigation")
file(TO_NATIVE_PATH "${INPUT_FOLDER}" NATIVE_INPUT_FOLDER)

file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/MoonNavigation/src/*.cpp")
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/MoonNavigation/header/*.h*")

add_executable(MoonNavigation ${SOURCES} ${HEADERS})

target_include_directories(MoonNavigation PRIVATE
    "${CMAKE_SOURCE_DIR}/MoonNavigation/header"
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/include/eigen-3.4.0"
)

if(MSVC)
    target_compile_options(MoonNavigation PRIVATE /W4) 
    
    # Автоматическая настройка аргументов для VS
    set_target_properties(MoonNavigation PROPERTIES
        VS_DEBUGGER_COMMAND_ARGUMENTS "\"${NATIVE_INPUT_FOLDER}\""
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
else()
    target_compile_options(MoonNavigation PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Дополнительно: вывод информации о путях
message(STATUS "Input folder path: ${NATIVE_INPUT_FOLDER}")
message(STATUS "Input file path: ${NATIVE_INPUT_FOLDER}/in.json")