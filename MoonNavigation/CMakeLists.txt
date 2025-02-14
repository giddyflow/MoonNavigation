cmake_minimum_required(VERSION 3.8)
project(MoonNavigation LANGUAGES CXX)

# Задаём стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Пути к вашим папкам
set(HEADER_DIR "${CMAKE_SOURCE_DIR}/MoonNavigation/header")
set(SRC_DIR    "${CMAKE_SOURCE_DIR}/MoonNavigation/src")

# Если хотите добавить в пути и папку include:
# include_directories("${CMAKE_SOURCE_DIR}/include")

# Добавляем директорию с заголовками (где лежат .h/.hpp)
include_directories(${HEADER_DIR})

# Собираем список исходников (*.cpp) из папки src
file(GLOB_RECURSE SOURCES
    "${SRC_DIR}/*.cpp"
)

# (Опционально) соберём список заголовочных файлов, чтобы IDE их видела
file(GLOB_RECURSE HEADERS
    "${HEADER_DIR}/*.h"
    "${HEADER_DIR}/*.hpp"
)

# Создаём исполняемый файл. Заголовки добавляем для удобства IDE,
# хотя для сборки достаточно только SOURCES.
add_executable(MoonNavigation
    ${SOURCES}
    ${HEADERS}
)

# Если нужны дополнительные библиотеки - добавьте их здесь:
# target_link_libraries(MoonNavigation PRIVATE MyLib)

# Или дополнительные пути для #include:
target_include_directories(MoonNavigation PRIVATE "${CMAKE_SOURCE_DIR}/include")
