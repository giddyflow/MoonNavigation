cmake_minimum_required(VERSION 3.16)
project(Jasmine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(eigen INTERFACE)
target_include_directories(eigen INTERFACE
    "include/eigen-3.4.0"
)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE
    "include"
)

set(SOURCES
    # Основные файлы
    "Jasmine/src/main.cpp"
    "Jasmine/src/Service.cpp"
    "Jasmine/src/Engine.cpp"
    "Jasmine/src/ModelEnvironment.cpp"

    # Модуль Приемников
    "Jasmine/Receiver/ReceiverObject.cpp"
    "Jasmine/Receiver/StaticReceiver.cpp"
    "Jasmine/Receiver/DynamicReceiver.cpp"

    # Модуль Спутников
    "Jasmine/Satellite/SatelliteObject.cpp"
    "Jasmine/Satellite/MediumSatellite.cpp"
    "Jasmine/Satellite/LowSatellite.cpp"
    "Jasmine/Satellite/GeoSatellite.cpp"

    # Модуль Помех
    "Jasmine/Jam/JamObject.cpp"
    "Jasmine/Jam/StaticJam.cpp"
    "Jasmine/Jam/DynamicJam.cpp"

    # Модуль RINEX
    "Jasmine/RinexWriter/RinexWriter.cpp"

    # логгер json
    "Jasmine/ResultCollector/ResultCollector.cpp"
    
    # Модуль расчета доступности
    "Jasmine/AccessibilityAnalyzer/AccessibilityAnalyzer.cpp"
)


add_executable(Jasmine ${SOURCES})

target_include_directories(Jasmine PRIVATE
    "Jasmine/header"
    "Jasmine/Receiver"
    "Jasmine/Satellite"
    "Jasmine/Jam"
    "Jasmine/RinexWriter"
    "Jasmine/ResultCollector"
    "Jasmine/AccessibilityAnalyzer"
)

target_link_libraries(Jasmine PRIVATE
    eigen
    nlohmann_json
)

if(MSVC)
    target_compile_options(Jasmine PRIVATE /W4)
else()
    target_compile_options(Jasmine PRIVATE -Wall -Wextra -Wpedantic)
endif()

set(INPUT_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/Jasmine")
file(TO_NATIVE_PATH "${INPUT_FOLDER}" NATIVE_INPUT_FOLDER)
if(MSVC)
    set_target_properties(Jasmine PROPERTIES
        VS_DEBUGGER_COMMAND_ARGUMENTS "\"${NATIVE_INPUT_FOLDER}\""
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
endif()

message(STATUS "Input folder path: ${NATIVE_INPUT_FOLDER}")
message(STATUS "Input file path: ${NATIVE_INPUT_FOLDER}/in.json")