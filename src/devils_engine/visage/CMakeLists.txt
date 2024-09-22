cmake_minimum_required(VERSION 3.1...3.25)

set(DEVILS_LIBRARY_NAME devils_visage)

project(
  ${DEVILS_LIBRARY_NAME}
  VERSION 0.1
  LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_EXTENSIONS OFF)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

  include_directories(
    src/devils_engine/
  )
endif()

set(SRC_LIST
  draw_resource.cpp
  draw_stage.cpp
  font.cpp
  header.cpp
  system.cpp
)

set(HDR_LIST
  draw_resource.h
  draw_stage.h
  font.h
  header.h
  system.h
)

add_library(${DEVILS_LIBRARY_NAME} ${SRC_LIST} ${HDR_LIST})
target_link_libraries(${DEVILS_LIBRARY_NAME} devils_painter)
