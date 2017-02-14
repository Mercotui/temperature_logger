# temperature_logger
Logs multiple device's temperatures over a network, and compare them to the weather.

## Uses:
I mainly just want to graph out my CPU core temperatures against the temperature of my
poorly ventilated room. Should also show how the weather impacts a Computers thermal performance.

## Features:
A simple website viewer that dynamically graphs the latest data.
-   Select which temperatures to graph.
-   (wip)Select time period to graph (Day, Week, Month, Year).

(wip)A data scraper to collect temperatures over the network.
-   http? will http conflict with the web server?
-   tcp? tcp is usefull for tiny digital thermometers

A weather API scraper
-   config parameter to specify the URI for an HTTP weather API
-   config parameter to specify the JSON nodes for temperature

## Dependencies:
This project uses Mongoose (HTTP), and Parson (JSON), which are included in the repo.
It also uses SQLite3, not included.
Bring your own:
-   sqlite3
-   cmake
-   C compiler (gcc or clang are known to work)
-   web browser (tested on firefox and chrome/chromium)
