cmake_minimum_required(VERSION 2.8)

MESSAGE(${CMAKE_CURRENT_SOURCE_DIR})
MESSAGE(${CMAKE_CURRENT_BINARY_DIR})

project (multimonop)

# Add path for custom modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include(FindPackageHandleStandardArgs)

add_subdirectory(zaama-v2)
