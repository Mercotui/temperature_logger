cmake_minimum_required(VERSION 2.6)
set(CMAKE_C_FLAGS "-fdiagnostics-color ${CMAKE_C_FLAGS}")

project (weather_scraper)
set(WEATHER_SCRAPER_SOURCES  weather_scraper.c ../mongoose/mongoose.c)
add_executable(weather_scraper_out ${WEATHER_SCRAPER_SOURCES})
target_link_libraries(weather_scraper_out pthread sqlite3)

configure_file(weather_scraper.conf.in ${EXECUTABLE_OUTPUT_PATH}/weather_scraper.conf)
