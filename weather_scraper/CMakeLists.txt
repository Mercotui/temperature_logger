cmake_minimum_required(VERSION 2.6)

set(WEATHER_SCRAPER_SOURCES weather_scraper.c ${CMAKE_SOURCE_DIR}/mongoose_http/mongoose.c ${CMAKE_SOURCE_DIR}/parson_json/parson.c)
add_executable(scraper_weather ${WEATHER_SCRAPER_SOURCES})
target_link_libraries(scraper_weather sqlite3)

configure_file(weather_scraper.conf.in ${CMAKE_BINARY_DIR}/weather_scraper.conf)
