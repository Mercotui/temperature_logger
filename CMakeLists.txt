cmake_minimum_required(VERSION 2.6)
project (rootproject)

#set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/build)

add_subdirectory(weather_scraper)
add_subdirectory(web_viewer)
