# CMakeList.txt: проект CMake для MoonNavigation; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

# Добавьте источник в исполняемый файл этого проекта.

set(HEADERS
    LowSatellite.h
    ReceiverObject.h
    SatelliteObject.h
    GeoSatellite.h
    ObjectFactory.h
    MediumSatellite.h
    DynamicReceiver.h
    StaticReceiver.h
    MoonNavigation.h
    Service.h
)

set(SOURCES
    MoonNavigation.cpp
)
include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable (MoonNavigation ${HEADERS} ${SOURCES})

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
