cmake_minimum_required(VERSION 3.1...3.25)

set(DEVILS_LIBRARY_NAME devils_thread)

project(
  ${DEVILS_LIBRARY_NAME}
  VERSION 0.1
  LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  # Let's ensure -std=c++xx instead of -std=g++xx
  set(CMAKE_CXX_EXTENSIONS OFF)

  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

  include_directories(
    src/devils_engine/
  )
endif()

set(SRC_LIST
  pool.cpp
  stack_pool.cpp
)

set(HDR_LIST
  pool.h
  atomic.h
  lock.h
  stack_pool.h
)

add_library(${DEVILS_LIBRARY_NAME} ${SRC_LIST} ${HDR_LIST})
target_link_libraries(${DEVILS_LIBRARY_NAME})
