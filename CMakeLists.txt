cmake_minimum_required(VERSION 3.16)

# Имя проекта общее, так как таргетов будет несколько
project(DemosCollection LANGUAGES CXX)

# Стандарт
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Тип сборки по умолчанию
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# 1. Собираем общие исходники (без demo.cpp)
file(GLOB_RECURSE COMMON_SRC_FILES
    ${CMAKE_SOURCE_DIR}/Core/*.cpp
    ${CMAKE_SOURCE_DIR}/BasicElements/*.cpp
)

# 2. Ищем все файлы демо-программ в папке src
file(GLOB DEMO_SRC_FILES ${CMAKE_SOURCE_DIR}/src/demo*.cpp)

# Общие заголовочные файлы для всех демок
set(COMMON_INCLUDES
    ${CMAKE_SOURCE_DIR}/Core
    ${CMAKE_SOURCE_DIR}/BasicElements
)

# 3. Цикл по всем найденным демо-файлам
foreach(DEMO_PATH IN LISTS DEMO_SRC_FILES)
    # Получаем имя файла без расширения (например, из "/path/src/demo1.cpp" сделает "demo1")
    get_filename_component(DEMO_NAME ${DEMO_PATH} NAME_WE)

    # Создаем исполняемый файл для конкретного демо
    add_executable(${DEMO_NAME} ${DEMO_PATH} ${COMMON_SRC_FILES})

    # Подключаем инклуды
    target_include_directories(${DEMO_NAME} PRIVATE ${COMMON_INCLUDES})

    # =========================
    # Компилятор-зависимые флаги
    # =========================
    target_compile_options(${DEMO_NAME} PRIVATE
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
    target_compile_options(${DEMO_NAME} PRIVATE
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
    target_link_options(${DEMO_NAME} PRIVATE
        $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/LTCG>
        $<$<AND:$<CONFIG:Release>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-flto>
    )

    # Включить IPO если поддерживается
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported)
    if(ipo_supported)
        set_property(TARGET ${DEMO_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    endif()

    # Установка конкретного EXE прямо в корень папки установки
    install(TARGETS ${DEMO_NAME} RUNTIME DESTINATION .)

    if(NOT MSVC)
        # Для MinGW добавляем статическую линковку, чтобы DLL не были нужны
        target_link_options(${DEMO_NAME} PRIVATE -static-libgcc -static-libstdc++ -static)
    endif()
endforeach()

# 4. Системные рантайм-библиотеки для MSVC (достаточно вызвать один раз вне цикла)
if(MSVC)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE) # Пропускаем лишнее
    include(InstallRequiredSystemLibraries)
    
    if(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
        install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION .)
    endif()
endif()