# Указывает минимальную требуемую версию CMake
cmake_minimum_required(VERSION 3.10)

# Задаёт имя проекта (можешь заменить на своё)
project(main)

# Задаёт параметры сборки
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -flto")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-parameter -Wunused-variable")

# Добавляет исполняемый файл. 
add_executable(${PROJECT_NAME}
    src/demo.cpp
    Core/Control.cpp
)

# Указываем, где искать заголовочные файлы
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/BasicElements
        ${PROJECT_SOURCE_DIR}/Core
)