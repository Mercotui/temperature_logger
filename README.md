# temperature_logger
Early implementation!
Logs multiple device's temperatures over the network, and compare them to the weather

## Uses:
I mainly just want to graph out my CPU core temperatures against the temperature of my
poorly ventilated room. Should also show how the weather impacts a Computers thermal performance.

## Features:
A simple webviewer that dynamically graphs the latest data.
-   (wip)Select which temperatures to graph.
-   (wip)Select time period to graph.
-   (wip)Select temperature to highlight by adding area under it.

(wip)A data scraper to collect temperatures over the network.
-   http? will http conflict with the web server?
-   tcp? tcp is usefull for tiny digital thermometers

A weather api scraper, uses a user provided http json api.
-   config parameter to specify http uri
-   config parameter to specify json node path to temperature

## Dependencies:
This project uses the Mongoose HTTP server, which is provided via a submodule.
It also uses SQLite3, not included.
Bring your own:
-   sqlite3
-   cmake
-   compiler (gcc or clang are known to work)
-   web browser (tested on firefox and chrome/chromium)
