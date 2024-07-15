cmake_minimum_required(VERSION 3.22)

#===========================================================================================
# GLAD CONFIGURATION
#
# Configure GLAD loader generation variables (lines below pulled from the GLAD top-level
# CMakeLists.txt file, with slight modifications.
# Refer to https://github.com/Dav1dde/glad for more information)

set(GLAD_OUT_DIR        "${CMAKE_CURRENT_BINARY_DIR}" CACHE STRING "Output directory" FORCE)
set(GLAD_PROFILE        "core" CACHE STRING "OpenGL profile" FORCE)
set(GLAD_API            "gl=4.5" CACHE STRING "API type/version pairs, like \"gl=3.2,gles=\", no version means latest" FORCE)
set(GLAD_GENERATOR      "c" CACHE STRING "Language to generate the binding for" FORCE)
set(GLAD_EXTENSIONS     "" CACHE STRING "Path to extensions file or comma separated list of extensions, if missing all extensions are included" FORCE)
set(GLAD_SPEC           "gl" CACHE STRING "Name of the spec" FORCE)
set(GLAD_ALL_EXTENSIONS OFF CACHE BOOL "Include all extensions instead of those specified by GLAD_EXTENSIONS" FORCE)
set(GLAD_NO_LOADER      OFF CACHE BOOL "No loader" FORCE)
set(GLAD_REPRODUCIBLE   OFF CACHE BOOL "Reproducible build" FORCE)

#===========================================================================================

add_library(glad STATIC src/glad.c)
target_include_directories(glad PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/)