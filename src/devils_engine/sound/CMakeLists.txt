cmake_minimum_required(VERSION 3.1...3.25)

set(DEVILS_LIBRARY_NAME devils_sound)

project(
  ${DEVILS_LIBRARY_NAME}
  VERSION 0.1
  LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  # Let's ensure -std=c++xx instead of -std=g++xx
  set(CMAKE_CXX_EXTENSIONS OFF)

  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  #set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++23")

  include_directories(
    include/dr_libs/
    include/spdlog/include/
    include/vorbis/include/
    include/stb/
    src/devils_engine/
  )
endif()

set(SRC_LIST
  al_helper.cpp
  mp3_decoder.cpp
  flac_decoder.cpp
  wav_decoder.cpp
  ogg_decoder.cpp
  pcm_decoder.cpp
  system.cpp
)

set(HDR_LIST
  al_helper.h
  mp3_decoder.h
  flac_decoder.h
  wav_decoder.h
  ogg_decoder.h
  pcm_decoder.h
  decoder.h
  system.h
)

add_library(${DEVILS_LIBRARY_NAME} ${SRC_LIST} ${HDR_LIST})
target_link_libraries(${DEVILS_LIBRARY_NAME} openal ) # OpenAL32
