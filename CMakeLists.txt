cmake_minimum_required(VERSION 3.1...3.25)

project(
  cosmojourney
  VERSION 0.1
  LANGUAGES CXX C
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_EXTENSIONS OFF)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  set(CMAKE_CXX_STANDARD 23)

  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	if(MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR /Wall /permissive- /Zc:preprocessor")
    else()
	  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")
    endif()
  endif()

  if (CMAKE_BUILD_TYPE STREQUAL "Release")
	if(MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /permissive- /Zc:preprocessor")
    else()
	  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    endif()
  endif()

endif()

include_directories(
  include/dr_libs/
  include/spdlog/include/
  include/vorbis/include/
  include/stb/
  include/glm/
  include/openal-soft/include
  include/sml/include
  include/qc-hash/
  include/reflect/
  include/glaze/include/
  include/alpaca/include/
  src/devils_engine/
  src/cosmojourney/
)

link_directories(libs)
link_directories(libs/lua54)
link_directories(libs/glfw34)
link_directories(libs/ktx)
link_directories(libs/openal)

if(MSVC)
  set(DYN_LIBRARIES_LIST
    libs/lua54/lua54.dll
    libs/openal/OpenAL32.dll
    libs/openal/soft_oal.dll
    libs/glfw34/glfw3.dll
    libs/ktx/ktx.dll
  )
else()
  set(DYN_LIBRARIES_LIST

  )
endif()

# Find packages go here.

# Link each target with other targets or add options, etc.
add_subdirectory(src/devils_engine/utils/)
add_subdirectory(src/devils_engine/sound/)
add_subdirectory(src/devils_engine/demiurg/)
add_subdirectory(src/devils_engine/thread/)
#add_subdirectory(src/devils_engine/interface/)

# надо бы переназвать exe файл, и сверху еще папку проекта сделать
add_executable(${PROJECT_NAME} main.cpp "src/cosmojourney/dsm.h" "src/cosmojourney/dsm.cpp" "src/cosmojourney/sound_resource.h" "src/cosmojourney/sound_resource.cpp")
target_link_libraries(${PROJECT_NAME} devils_utils devils_sound devils_demiurg devils_thread) #sui 

foreach (library_path IN LISTS DYN_LIBRARIES_LIST)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
    COMMAND "${CMAKE_COMMAND}" -E copy 
        "${CMAKE_SOURCE_DIR}/${library_path}"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/" 
    COMMENT "Copy libs")
endforeach()