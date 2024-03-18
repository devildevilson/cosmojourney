cmake_minimum_required(VERSION 3.1...3.25)

set(DEVILS_LIBRARY_NAME devils_demiurg)

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
    include/spdlog/include/
    include/sml/include
    include/qc-hash/
    src/devils_engine/
  )
endif()

set(SRC_LIST
  system.cpp
)

set(HDR_LIST
  resource_base.h
  system.h
)

add_library(${DEVILS_LIBRARY_NAME} ${SRC_LIST} ${HDR_LIST})
target_link_libraries(${DEVILS_LIBRARY_NAME})
