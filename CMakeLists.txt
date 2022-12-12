cmake_minimum_required(VERSION 3.0.0)

# set(CMAKE_BUILD_TYPE DEBUG)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message("cmake build type: " ${CMAKE_BUILD_TYPE})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_policy(SET CMP0074 NEW)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(segy VERSION 0.1.0)

if (NOT CMAKE_INSTALL_PREFIX)
  message("default path")
  set(CMAKE_INSTALL_PREFIX ${segy_SOURCE_DIR}/Dist)
endif()
message("Install Prefix: " ${CMAKE_INSTALL_PREFIX})

set(BUILD_PYTHON ON)
set(BUILD_TOOLS ON)
set(ENABLE_OPENMP OFF)

if (BUILD_PYTHON)
  find_package(pybind11 REQUIRED)
endif()

if (APPLE)
  set(CMAKE_BUILD_RPATH "@executable_path")
endif()

# openmp
if (ENABLE_OPENMP)
  if (APPLE)
    set(OpenMP_ROOT /opt/homebrew/opt/libomp/)
  endif()
  find_package(OpenMP)
endif()
if (OpenMP_CXX_FOUND)
  message("Use OpenMP")
  set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)
endif()

# fmt
if (APPLE)
  set(fmt_ROOT /opt/homebrew/Cellar/fmt/9.1.0/)  
endif()
find_package(fmt REQUIRED)

add_subdirectory(src)

if (BUILD_TOOLS)
  add_subdirectory(tools)
endif()

if (BUILD_PYTHON)
  add_subdirectory(python)
endif()





