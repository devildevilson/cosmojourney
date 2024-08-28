cmake_minimum_required(VERSION 3.1...3.25)

set(DEVILS_LIBRARY_NAME devils_painter)

project(
  ${DEVILS_LIBRARY_NAME}
  VERSION 0.1
  LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_EXTENSIONS OFF)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  set(CMAKE_CXX_STANDARD 20)

  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	if(MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR /Wall /permissive-")
    else()
	  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")
    endif()
  endif()

  if (CMAKE_BUILD_TYPE STREQUAL "Release")
	if(MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /permissive-")
    else()
	  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    endif()
  endif()

  include_directories(
    include/dr_libs/
    include/spdlog/include/
    include/vorbis/include/
    include/stb/
    include/glm/
    include/openal-soft/include
    src/devils_engine/
  )
endif()

if (WIN32)
  set(SHADERC_LIBRARY_NAME shaderc_shared)
else()
  set(SHADERC_LIBRARY_NAME libshaderc)
endif()

set(SRC_LIST
  auxiliary.cpp
  glsl_source_file.cpp
  makers.cpp
  shader_crafter.cpp
  system.cpp
  system_info.cpp
  vulkan_header.cpp
)

set(HDR_LIST
  auxiliary.h
  glsl_source_file.h
  makers.h
  shader_crafter.h
  system.h
  system_info.h
  vulkan_header.h
  common.h
  shader_guru.h
  stage.h
  stage_buffer.h
  vulkan_minimal.h
 "target.h")

add_library(${DEVILS_LIBRARY_NAME} ${SRC_LIST} ${HDR_LIST})
target_link_libraries(${DEVILS_LIBRARY_NAME} ${SHADERC_LIBRARY_NAME})
