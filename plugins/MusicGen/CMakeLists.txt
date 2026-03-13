set(CMAKE_CXX_STANDARD 17)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

dpf_add_plugin(
  MusicGen
  TARGETS 
      vst2
      vst3
      jack
  FILES_DSP
      MusicGen.cpp
  FILES_UI
        MusicGenUI.cpp
      ${CMAKE_SOURCE_DIR}/common/src/SimpleButton.cpp
)

# Externals:
if(APPLE)
  find_library(FOUNDATION Foundation)
  find_library(COREFOUNDATION CoreFoundation)
elseif(WIN32)
elseif(UNIX AND NOT APPLE)
  target_compile_definitions(WAIVE_Sampler PUBLIC LINUX)
endif()

set(BUILD_SHARED_LIBS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")

message(STATUS "BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")

link_directories(
  /usr/lib
)

target_include_directories(
  MusicGen
  PUBLIC 
    .
    ./components
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/common/fonts
    ${CMAKE_SOURCE_DIR}/common/icons
)

target_link_libraries(
    MusicGen
    PUBLIC
    ${CoreFoundation}
    ${Foundation}
)

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")