cmake_minimum_required(VERSION 3.1...3.25)

set(DEVILS_LIBRARY_NAME devils_sound)

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
  set(OPENAL_LIBRARY_NAME OpenAL32)
else()
  set(OPENAL_LIBRARY_NAME openal)
endif()

set(SRC_LIST
  al_helper.cpp
  mp3_decoder.cpp
  flac_decoder.cpp
  wav_decoder.cpp
  ogg_decoder.cpp
  pcm_decoder.cpp
  system.cpp
 "resource.cpp")

set(HDR_LIST
  al_helper.h
  mp3_decoder.h
  flac_decoder.h
  wav_decoder.h
  ogg_decoder.h
  pcm_decoder.h
  decoder.h
  system.h
 "virtual_source.h" "resource.h")

add_library(${DEVILS_LIBRARY_NAME} ${SRC_LIST} ${HDR_LIST})
target_link_libraries(${DEVILS_LIBRARY_NAME} ${OPENAL_LIBRARY_NAME}) # openal
