cmake_minimum_required(VERSION 2.6)
set(CMAKE_C_FLAGS "-fdiagnostics-color ${CMAKE_C_FLAGS}")
project (web_server)

set(WEB_SERVER_SOURCES web_server.c ../mongoose/mongoose.c)
add_executable(web_server ${WEB_SERVER_SOURCES})
target_link_libraries (web_server sqlite3)

configure_file(web_site/index.html ${EXECUTABLE_OUTPUT_PATH}/web_site/index.html COPYONLY)
configure_file(web_site/styles.css ${EXECUTABLE_OUTPUT_PATH}/web_site/styles.css COPYONLY)
configure_file(web_site/live.js ${EXECUTABLE_OUTPUT_PATH}/web_site/live.js COPYONLY)
configure_file(web_site/favicon.ico ${EXECUTABLE_OUTPUT_PATH}/web_site/favicon.ico COPYONLY)
