cmake_minimum_required(VERSION 2.6)
#project (web_server)

set(WEB_SERVER_SOURCES web_server.c ${CMAKE_SOURCE_DIR}/mongoose_http/mongoose.c ${CMAKE_SOURCE_DIR}/parson_json/parson.c)
add_executable(web_server ${WEB_SERVER_SOURCES})
target_link_libraries (web_server sqlite3)

configure_file(web_site/index.html ${CMAKE_BINARY_DIR}/web_site/index.html COPYONLY)
configure_file(web_site/styles.css ${CMAKE_BINARY_DIR}/web_site/styles.css COPYONLY)
configure_file(web_site/live.js ${CMAKE_BINARY_DIR}/web_site/live.js COPYONLY)
configure_file(web_site/favicon.ico ${CMAKE_BINARY_DIR}/web_site/favicon.ico COPYONLY)
