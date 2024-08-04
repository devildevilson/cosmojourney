cmake_minimum_required(VERSION 3.1...3.25)

set(DEVILS_LIBRARY_NAME devils_utils)

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
    src/devils_engine/
  )
endif()

set(SRC_LIST
  core.cpp
  block_allocator.cpp
  stack_allocator.cpp
  bits.cpp
  bitstream.cpp
  prng.cpp
  fileio.cpp
)

set(HDR_LIST
  block_allocator.h
  stack_allocator.h
  bits.h
  bitstream.h
  memory_pool.h
  type_traits.h
  core.h
  string.h
  flat_hash_map.hpp
  safe_handle.h
  prng.h
  dice.h
  
  named_serializer.h
  fileio.h
)

include_directories(
  include/spdlog/include/
  include/stb/
  include/glm/
  include/sml/include
  include/qc-hash/
  include/reflect/
  include/glaze/include/
  include/alpaca/include/
)

add_library(${DEVILS_LIBRARY_NAME} ${SRC_LIST} ${HDR_LIST})
target_link_libraries(${DEVILS_LIBRARY_NAME})
