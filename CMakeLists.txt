cmake_minimum_required(VERSION 3.16)

project(demo5 LANGUAGES CXX)

# Стандарт
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Тип сборки по умолчанию
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Собираем исходники
file(GLOB_RECURSE SRC_FILES
    ${CMAKE_SOURCE_DIR}/Core/*.cpp
    ${CMAKE_SOURCE_DIR}/BasicElements/*.cpp
    ${CMAKE_SOURCE_DIR}/src/demo5.cpp
)

# Исполняемый файл
add_executable(demo5
    ${SRC_FILES}
)

# Include paths
target_include_directories(demo5 PRIVATE
    ${CMAKE_SOURCE_DIR}/Core
    ${CMAKE_SOURCE_DIR}/BasicElements
)

# =========================
# Компилятор-зависимые флаги
# =========================
target_compile_options(demo5 PRIVATE
    # MSVC
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4
        /permissive-
    >

    # GCC / Clang
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
        -Wall
        -Wextra
        -Wpedantic
        -Wunused-parameter
        -Wunused-variable
    >
)

# =========================
# Оптимизации (Release)
# =========================
target_compile_options(demo5 PRIVATE
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:
        /O2
        /GL
    >

    $<$<AND:$<CONFIG:Release>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:
        -O3
        -march=native
        -flto
    >
)

# Линковка (LTO)
target_link_options(demo5 PRIVATE
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/LTCG>
    $<$<AND:$<CONFIG:Release>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-flto>
)

# Включить IPO если поддерживается
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)
if(ipo_supported)
    set_property(TARGET demo5 PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()