# Указывает минимальную требуемую версию CMake
cmake_minimum_required(VERSION 3.10)

# Задаёт имя проекта (можешь заменить на своё)
project(demo4)

# Задаёт параметры сборки
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -flto")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-parameter -Wunused-variable")

# Добавляет все файлы из папки Core и BasicElements
file(GLOB_RECURSE SRC_FILES
    ${CMAKE_SOURCE_DIR}/Core/*.cpp
    ${CMAKE_SOURCE_DIR}/BasicElements/*.cpp
    ${CMAKE_SOURCE_DIR}/src/demo4.cpp
)

# Добавление исполняемого файла
add_executable(demo4 ${SRC_FILES})

# Указываем, где искать заголовки
target_include_directories(demo4 PUBLIC
    ${CMAKE_SOURCE_DIR}/Core
    ${CMAKE_SOURCE_DIR}/BasicElements
)



# # Добавляет исполняемый файл. 
# add_library(winui3 SHARED ${SRC_FILES})

# # Указываем, где искать заголовочные файлы
# target_include_directories(winui3 PUBLIC
#     ${CMAKE_SOURCE_DIR}/Core
#     ${CMAKE_SOURCE_DIR}/BasicElements
# )