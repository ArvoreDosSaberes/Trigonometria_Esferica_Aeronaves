cmake_minimum_required(VERSION 3.16)
project(Trigonometria_Esferica_Aeronaves C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Raylib dependency via FetchContent
include(FetchContent)
# Share FetchContent downloads/builds in ~/.cache/cmake by default
if(NOT DEFINED FETCHCONTENT_BASE_DIR)
  set(FETCHCONTENT_BASE_DIR "$ENV{HOME}/.cache/cmake" CACHE PATH "Base dir for FetchContent downloads" FORCE)
endif()

set(RAYLIB_VERSION 5.0)
# Prefer system package if available
find_package(raylib ${RAYLIB_VERSION} QUIET)
if(NOT raylib_FOUND)
  message(STATUS "raylib not found system-wide; fetching ${RAYLIB_VERSION}...")
  FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG ${RAYLIB_VERSION}
  )
  FetchContent_MakeAvailable(raylib)
endif()

# Warnings
if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

# Source
add_executable(spherical_trig
  src/main.c
)

# Link
if (UNIX AND NOT APPLE)
  target_link_libraries(spherical_trig PRIVATE raylib m)
else()
  target_link_libraries(spherical_trig PRIVATE raylib)
endif()

# Include raylib headers if it's a fetched target
if (TARGET raylib)
  target_include_directories(spherical_trig PRIVATE ${raylib_SOURCE_DIR}/src)
endif()

# Install
install(TARGETS spherical_trig RUNTIME DESTINATION bin)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# --- Documentation (Doxygen) ---
find_package(Doxygen QUIET)
if (DOXYGEN_FOUND)
  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
else()
  message(STATUS "Doxygen not found: creating placeholder 'doc' target (install doxygen to enable)")
  add_custom_target(doc
    COMMAND ${CMAKE_COMMAND} -E echo "Doxygen not found. Please install doxygen and reconfigure."
    VERBATIM)
endif()
