cmake_minimum_required(VERSION 3.12...3.26)

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT is_multi_config AND NOT (CMAKE_BUILD_TYPE OR DEFINED ENV{CMAKE_BUILD_TYPE}))
  set(CMAKE_BUILD_TYPE "Debug;Release;Profile" CACHE STRING "Release default")
endif()

project(tinyrenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}) # Set the output directory for the executable file

option(iwyu "Run include-what-you-use")
if(iwyu)
  find_program(IWYU_EXE NAMES include-what-you-use REQUIRED)
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_EXE})
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Cla`ng|GNU|Intel")
  add_compile_options(-Wall)
endif()

find_package(OpenMP COMPONENTS CXX)

set(SOURCES main.cpp object.cpp mesh.cpp tgaimage.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE $<$<BOOL:${OpenMP_CXX_FOUND}>:OpenMP::OpenMP_CXX>)

# --------------------------------------------- Profile Option -------------------------------------------
if(CMAKE_BUILD_TYPE STREQUAL "Profile")
  # add gprof compile and link option
  add_compile_options(-pg)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
  
  # disable inline for more accurate call maps
  add_compile_options(-fno-inline -fno-inline-small-functions -fno-default-inline)
  
  # add optimization level
  add_compile_options(-O3)
endif()

# target_compile_options(${PROJECT_NAME} PRIVATE -DDEBUG) # add compile define for debugging
# target_compile_options(${PROJECT_NAME} PRIVATE -DWIREFRAME) # add compile define for WIREFRAME RENDERING

file(GENERATE OUTPUT .gitignore CONTENT "*")
