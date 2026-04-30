cmake_minimum_required(VERSION 3.16)

project(demo_widget LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# =========================
# Demo selector
# =========================
set(DEMO_NAME "demo_widget" CACHE STRING "Select demo entrypoint (without .cpp)")

message(STATUS "Building demo: ${DEMO_NAME}")

# =========================
# Core sources (engine / framework)
# =========================
file(GLOB_RECURSE CORE_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/Core/*.cpp
    ${CMAKE_SOURCE_DIR}/BasicElements/*.cpp
)

# =========================
# Demo entrypoints (only ONE selected)
# =========================
set(DEMO_SOURCES
    ${CMAKE_SOURCE_DIR}/src/${DEMO_NAME}.cpp
)

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/src/${DEMO_NAME}.cpp)
    message(FATAL_ERROR "Demo entrypoint not found: ${DEMO_NAME}.cpp")
endif()

# =========================
# Target
# =========================
add_executable(${PROJECT_NAME}
    ${CORE_SOURCES}
    ${DEMO_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Core
    ${CMAKE_SOURCE_DIR}/BasicElements
)

# =========================
# Warnings
# =========================
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Wunused-parameter -Wunused-variable>
)

# =========================
# Release optimizations
# =========================
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/O2 /GL>
    $<$<AND:$<CONFIG:Release>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-O3 -march=native>
)

target_link_options(${PROJECT_NAME} PRIVATE
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/LTCG>
)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)
if(ipo_supported)
    set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()