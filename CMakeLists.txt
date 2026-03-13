cmake_minimum_required(VERSION 3.22)

set(MUSICGENVST_VERSION "0.0.1" CACHE STRING "Plugin version (set from CI tag)")

project(MusicGenVST VERSION ${MUSICGENVST_VERSION})

add_subdirectory(JUCE)
add_subdirectory(plugins/MusicGenVST)
