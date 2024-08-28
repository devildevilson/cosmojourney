cmake_minimum_required(VERSION 3.1...3.25)

project(
  cosmojourney
  VERSION 0.1
  LANGUAGES CXX C
)

set(DEVILS_ENGINE_NAME "devils_engine")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

if (CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_EXTENSIONS OFF)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  set(CMAKE_CXX_STANDARD 23)

  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	if(MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /permissive- /Zc:preprocessor")
    else()
	  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -fno-rtti")
    endif()
  endif()

  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-D_NDEBUG)
	if(MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /O2 /permissive- /Zc:preprocessor")
    else()
	  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fno-rtti")
    endif()
  endif()

endif()

include_directories(
  libs/vulkan/Include/
  libs/lua54/include/
  libs/glfw34/include/
  libs/ktx/include/
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
  include/sol2/include/
  include/fmt/include/
  include/parallel-hashmap/
  include/VulkanMemoryAllocator-Hpp/include/
  src/devils_engine/
  src/cosmojourney/
)

link_directories(libs)
link_directories(libs/lua54)
link_directories(libs/glfw34)
link_directories(libs/ktx)
link_directories(libs/openal)
link_directories(libs/vulkan)

if(MSVC)
  set(DYN_LIBRARIES_LIST
    libs/lua54/lua54.dll
    libs/openal/OpenAL32.dll
    libs/openal/soft_oal.dll
    libs/glfw34/glfw3.dll
    libs/ktx/ktx.dll
    libs/vulkan/shaderc_shared.dll
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
add_subdirectory(src/devils_engine/input/)
add_subdirectory(src/devils_engine/painter/)
#add_subdirectory(src/devils_engine/interface/)

# надо бы переназвать exe файл, и сверху еще папку проекта сделать
add_executable(${PROJECT_NAME} main.cpp "src/cosmojourney/dsm.h" "src/cosmojourney/dsm.cpp" "src/cosmojourney/sound_resource.h" "src/cosmojourney/sound_resource.cpp")
target_link_libraries(${PROJECT_NAME} devils_utils devils_sound devils_demiurg devils_thread devils_input devils_painter lua54)
target_compile_definitions(${PROJECT_NAME} PUBLIC "DEVILS_ENGINE_PROJECT_NAME=\"${PROJECT_NAME}\"")
target_compile_definitions(${PROJECT_NAME} PUBLIC "DEVILS_ENGINE_ENGINE_NAME=\"${DEVILS_ENGINE_NAME}\"")

foreach (library_path IN LISTS DYN_LIBRARIES_LIST)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
    COMMAND "${CMAKE_COMMAND}" -E copy 
        "${CMAKE_SOURCE_DIR}/${library_path}"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/" 
    COMMENT "Copy libs")
endforeach()