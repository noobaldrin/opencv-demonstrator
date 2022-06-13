cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 17)

project(opencv-demonstrator)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_definitions(-DLINUX)
endif()

#if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
#  target_compile_definitions(example PUBLIC "IS_MACOS")
#endif()
#if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
#  target_compile_definitions(example PUBLIC "IS_WINDOWS")
#endif()

add_subdirectory(libcutil)
add_subdirectory(libocvext)
add_subdirectory(ocvdemo)
