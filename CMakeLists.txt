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

if(MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} /wd5045 /wd4820 /GR- /permissive- /Zc:preprocessor")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} /GR- /O2 /permissive- /Zc:preprocessor")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} /wd5045 /wd4820 /GR- /permissive- /Zc:preprocessor")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} /GR- /O2 /permissive- /Zc:preprocessor")
else()
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -fno-rtti")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -fno-rtti")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -g -Wall -Wextra -fno-rtti")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3 -fno-rtti")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  add_definitions(-D_NDEBUG)
  add_definitions(-DNDEBUG)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_EXTENSIONS OFF)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  set(CMAKE_CXX_STANDARD 23)

  #if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	#if(MSVC)
  #    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /permissive- /Zc:preprocessor")
  #  else()
	#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -fno-rtti")
  #  endif()
  #endif()

  #if (CMAKE_BUILD_TYPE STREQUAL "Release")
	#if(MSVC)
  #    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /O2 /permissive- /Zc:preprocessor")
  #  else()
	#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fno-rtti")
  #  endif()
  #endif()

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
  include/dynamic_bitset/include
  include/Nuklear/
  include/msdf-atlas-gen/
  include/msdfgen/
  include/
  src/devils_engine/
  src/cosmojourney/
)

link_directories(libs)
link_directories(libs/lua54)
link_directories(libs/glfw34)
link_directories(libs/ktx)
link_directories(libs/openal)
link_directories(libs/vulkan)
link_directories(libs/libpng16)
link_directories(libs/brotli)
link_directories(libs/freetype)

if(MSVC)
  set(DYN_LIBRARIES_LIST
    libs/lua54/lua54.dll
    libs/openal/OpenAL32.dll
    libs/openal/soft_oal.dll
    libs/glfw34/glfw3.dll
    libs/ktx/ktx.dll
    libs/vulkan/shaderc_shared.dll
    libs/freetype/freetype.dll
    libs/libpng16/libpng16.dll
    libs/brotli/brotlicommon.dll
    libs/brotli/brotlidec.dll
    libs/brotli/brotlienc.dll
  )
else()
  set(DYN_LIBRARIES_LIST

  )
endif()

add_compile_definitions("DEVILS_ENGINE_PROJECT_NAME=\"${PROJECT_NAME}\"")
add_compile_definitions("DEVILS_ENGINE_ENGINE_NAME=\"${DEVILS_ENGINE_NAME}\"")

# Link each target with other targets or add options, etc.
add_subdirectory(src/devils_engine/utils/)
add_subdirectory(src/devils_engine/sound/)
add_subdirectory(src/devils_engine/demiurg/)
add_subdirectory(src/devils_engine/thread/)
add_subdirectory(src/devils_engine/input/)
add_subdirectory(src/devils_engine/painter/)
add_subdirectory(src/devils_engine/visage/)

add_subdirectory(include/dynamic_bitset)

# надо бы переназвать exe файл, и сверху еще папку проекта сделать
add_executable(${PROJECT_NAME} main.cpp "src/cosmojourney/dsm.h" "src/cosmojourney/dsm.cpp" "src/cosmojourney/sound_resource.h" "src/cosmojourney/sound_resource.cpp" "src/devils_engine/simul/interface.h" "src/devils_engine/simul/interface.cpp")
target_link_libraries(${PROJECT_NAME} devils_utils devils_sound devils_demiurg devils_thread devils_input devils_painter devils_visage lua54 sul::dynamic_bitset msdf-atlas-gen msdfgen-core msdfgen-ext freetype libpng16)
set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

foreach (library_path IN LISTS DYN_LIBRARIES_LIST)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
    COMMAND "${CMAKE_COMMAND}" -E copy 
        "${CMAKE_SOURCE_DIR}/${library_path}"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/" 
    COMMENT "Copy libs")
endforeach()