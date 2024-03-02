cmake_minimum_required(VERSION 3.1...3.25)

project(
  cosmojourney
  VERSION 0.1
  LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  # Let's ensure -std=c++xx instead of -std=g++xx
  set(CMAKE_CXX_EXTENSIONS OFF)

  # Let's nicely support folders in IDEs
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

endif()

# If you set any CMAKE_ variables, that can go here.
# (But usually don't do this, except maybe for C++ standard)
include_directories(
  include/dr_libs/
  include/spdlog/include/
  include/vorbis/include/
  include/stb/
  include/glm/
  include/openal-soft/include
  src/devils_engine/
  src/cosmojorney/
)

link_directories(libs)

# Find packages go here.

# You should usually split this into folders, but this is a simple example

# This is a "default" library, and will match the *** variable setting.
# Other common choices are STATIC, SHARED, and MODULE
# Including header files here helps IDEs but is not required.
# Output libname matches target name, with the usual extensions on your system
#add_library(MyLibExample simple_lib.cpp simple_lib.hpp)

# Link each target with other targets or add options, etc.
add_subdirectory(src/devils_engine/utils/)
add_subdirectory(src/devils_engine/sound/)
#add_subdirectory(src/devils_engine/interface/)

# Adding something we can run - Output name matches target name
add_executable(${PROJECT_NAME} main.cpp)

# Make sure you link your targets with this command. It can also link libraries and
# even flags, so linking a target that does not exist will not give a configure-time error. sui
target_link_libraries(${PROJECT_NAME} devils_utils devils_sound) #sui 
