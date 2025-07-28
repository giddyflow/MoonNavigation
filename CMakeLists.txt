cmake_minimum_required(VERSION 3.16)
project(MoonNavigation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Шаг 1: Описываем наши сторонние библиотеки ---
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
    "MoonNavigation/src/main.cpp"
    "MoonNavigation/src/Service.cpp"
    "MoonNavigation/src/ModelEnvironment.cpp"

    # Модуль Приемников
    "MoonNavigation/Receiver/ReceiverObject.cpp"
    "MoonNavigation/Receiver/StaticReceiver.cpp"
    "MoonNavigation/Receiver/DynamicReceiver.cpp"

    # Модуль Спутников
    "MoonNavigation/Satellite/SatelliteObject.cpp"
    "MoonNavigation/Satellite/MediumSatellite.cpp"
    "MoonNavigation/Satellite/LowSatellite.cpp"
    "MoonNavigation/Satellite/GeoSatellite.cpp"

    # Модуль Помех
    "MoonNavigation/Jam/JamObject.cpp"
    "MoonNavigation/Jam/StaticJam.cpp"
    "MoonNavigation/Jam/DynamicJam.cpp"

    # Модуль RINEX
    "MoonNavigation/RinexWriter/RinexWriter.cpp"

    "MoonNavigation/ResultCollector/ResultCollector.cpp"
)


# Создаем исполняемый файл
add_executable(MoonNavigation ${SOURCES})

# Указываем компилятору ВСЕ папки, где нужно искать #include
target_include_directories(MoonNavigation PRIVATE
    "MoonNavigation/header"
    "MoonNavigation/Receiver"
    "MoonNavigation/Satellite"
    "MoonNavigation/Jam"
    "MoonNavigation/RinexWriter"
    "MoonNavigation/ResultCollector"
)

# --- Шаг 3: "Связываем" наш проект с библиотеками ---
target_link_libraries(MoonNavigation PRIVATE
    eigen
    nlohmann_json
)



# --- Остальная часть файла без изменений ---
if(MSVC)
    target_compile_options(MoonNavigation PRIVATE /W4)
else()
    target_compile_options(MoonNavigation PRIVATE -Wall -Wextra -Wpedantic)
endif()

set(INPUT_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/MoonNavigation")
file(TO_NATIVE_PATH "${INPUT_FOLDER}" NATIVE_INPUT_FOLDER)
if(MSVC)
    set_target_properties(MoonNavigation PROPERTIES
        VS_DEBUGGER_COMMAND_ARGUMENTS "\"${NATIVE_INPUT_FOLDER}\""
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
endif()

message(STATUS "Input folder path: ${NATIVE_INPUT_FOLDER}")
message(STATUS "Input file path: ${NATIVE_INPUT_FOLDER}/in.json")