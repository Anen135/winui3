# Указывает минимальную требуемую версию CMake
cmake_minimum_required(VERSION 3.10)

# Задаёт имя проекта (можешь заменить на своё)
project(winui3)

# Указывает стандарт C++ (например, C++17)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Добавляет исполняемый файл. 
add_executable(${PROJECT_NAME}
    src/test2.cpp
    Core/Control.cpp
)

# Указываем, где искать заголовочные файлы
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/BasicElements
        ${PROJECT_SOURCE_DIR}/Core
)