cmake_minimum_required(VERSION 2.6)
set(CMAKE_C_FLAGS "-fdiagnostics-color ${CMAKE_C_FLAGS}")
#project (weather_scraper)

set(WEATHER_SCRAPER_SOURCES weather_scraper.c ${CMAKE_SOURCE_DIR}/mongoose/mongoose.c)
add_executable(scraper_weather ${WEATHER_SCRAPER_SOURCES})
target_link_libraries(scraper_weather pthread sqlite3)

configure_file(weather_scraper.conf.in ${CMAKE_BINARY_DIR}/weather_scraper.conf)
