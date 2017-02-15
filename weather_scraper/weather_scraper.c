//-----includes
#define _GNU_SOURCE
#include <pthread.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../mongoose_http/mongoose.h"
#include "../parson_json/parson.h"

//-----defines
#define S_POLL_TIME 300 // 5 minutes
#define SQLCMD_FORMAT "INSERT INTO Weather VALUES(CURRENT_TIMESTAMP, %f);"
#define CONF_PATH "./weather_scraper.conf"
#define CONF_URI_HEADER "weather uri="
#define CONF_JSONPATH_HEADER "termperature json node="

//-----global variables
static char* _req_uri;
static char* _json_nodes_path;
static int _alive;
static int _connection_open;

//-----static functions
static void http_event_handler (struct mg_connection* nc, int ev, void* ev_data);
static double http_parse_resp (const char* body, const size_t len);
static void db_insert_temperature (double temperature);
static int config_get (void);
static int config_parse (const char* uri_line, const char* node_line);
static void config_trim_line (char* str);

//-----function defenitions
// http functions
static void http_event_handler (struct mg_connection* nc, int ev, void* ev_data) {
    struct http_message* hm = (struct http_message*)ev_data;

    switch (ev) {
        case MG_EV_CONNECT: {
            if (*(int*)ev_data != 0) {
                printf ("failed to connect: %s\n", strerror (*(int*)ev_data));
            } else {
                printf ("sent request to: %s\n", _req_uri);
            }
            break;
        }
        case MG_EV_HTTP_REPLY: {
            printf ("recieved reply\n");

            double temperature = http_parse_resp (hm->body.p, hm->body.len);
            db_insert_temperature (temperature);

            nc->flags |= MG_F_CLOSE_IMMEDIATELY;
            break;
        }
        case MG_EV_CLOSE: {
            _connection_open = 0;
            printf ("connection closed\n");
            break;
        }
        default: { break; }
    }
}

static double http_parse_resp (const char* body, const size_t len) {
    double ret = 0;

    JSON_Value* json_root;
    JSON_Object* json_root_object;
    json_root        = json_parse_string (body);
    json_root_object = json_value_get_object (json_root);

    JSON_Value* json_temp_node =
    json_object_dotget_value (json_root_object, _json_nodes_path);

    if (json_temp_node != NULL) {
        switch (json_value_get_type (json_temp_node)) {
            case JSONString:
                ret = strtod (json_value_get_string (json_temp_node), NULL);
                break;
            case JSONNumber:
                ret = json_value_get_number (json_temp_node);
                break;
            default:
                printf ("'%s' is not a number or string in "
                        "response:\n>>>>>\n%s\n<<<<<\n",
                _json_nodes_path, body);
        }
    } else {
        printf ("did not find '%s' in response:\n>>>>>\n%s\n<<<<<\n", _json_nodes_path, body);
    }

    json_value_free (json_temp_node);
    json_value_free (json_root);
    return ret;
}

// database functions
// CREATE TABLE Weather (Time INT?, Temperature DOUBLE);
static void db_insert_temperature (double temperature) {
    char* err_msg = NULL;
    int status;
    sqlite3* db;

    status = sqlite3_open ("temperatures.db", &db);
    if (status != SQLITE_OK) {
        fprintf (stderr, "Cannot open database: %s\n", sqlite3_errmsg (db));
    } else {
        status = sqlite3_exec (db, "CREATE TABLE IF NOT EXISTS Weather "
                                   "(Timestamp DATETIME, Temperature DOUBLE);",
        NULL, NULL, &err_msg);
        if (status != SQLITE_OK) {
            printf ("SQL error: %s\n", err_msg);
            sqlite3_free (err_msg);
        } else {
            /*size_t sqlcmd_len = strlen (SQLCMD_FORMAT) + strlen (temperature);
            char* sqlcmd      = malloc (sqlcmd_len);
*/
            char* sqlcmd;
            asprintf (&sqlcmd, SQLCMD_FORMAT, temperature);
            status = sqlite3_exec (db, sqlcmd, NULL, NULL, &err_msg);
            if (status == SQLITE_OK) {
                printf ("Inserted %f\n\n", temperature);
            } else {
                printf ("SQL error: %s\n\n", err_msg);
                sqlite3_free (err_msg);
            }
        }
    }

    sqlite3_close (db);
    return;
}

// config functions
static int config_get (void) {
    int status = -1;
    FILE* conf_file;
    char *uri_line = NULL, *node_line = NULL;
    size_t read_buff_size = 0;
    ssize_t read_size1, read_size2;

    conf_file = fopen (CONF_PATH, "r");
    if (conf_file != NULL) {
        read_size1 = getline (&uri_line, &read_buff_size, conf_file);
        read_size2 = getline (&node_line, &read_buff_size, conf_file);
        if (read_size1 <= 0) {
            printf ("No contents in %s\n", CONF_PATH);
        } else if (read_size2 <= 0) {
            printf ("No second line in %s\n", CONF_PATH);
        } else {
            status = config_parse (uri_line, node_line);
        }
        fclose (conf_file);
    } else {
        printf ("Can not open %s\n", CONF_PATH);
    }

    free (uri_line);
    free (node_line);

    return status;
}

static int config_parse (const char* uri_line, const char* node_line) {
    int status = 0;

    if (strncmp (uri_line, CONF_URI_HEADER, strlen (CONF_URI_HEADER)) == 0) {
        _req_uri = strdup (uri_line + strlen (CONF_URI_HEADER));
        config_trim_line (_req_uri);
    } else {
        status = -1;
        printf ("%s does not follow format %s=http://example.com/api/weather\n",
        CONF_URI_HEADER, CONF_URI_HEADER);
    }

    if (strncmp (node_line, CONF_JSONPATH_HEADER, strlen (CONF_JSONPATH_HEADER)) == 0) {
        _json_nodes_path = strdup (node_line + strlen (CONF_JSONPATH_HEADER));
        config_trim_line (_json_nodes_path);
    } else {
        status = -1;
        printf ("%s does not follow format %s=example.weather.current.temp\n",
        CONF_JSONPATH_HEADER, CONF_JSONPATH_HEADER);
    }

    return status;
}

static void config_trim_line (char* str) {
    char* end;
    end = str + strlen (str) - 1;
    while (end > str && isspace (*end)) end--;

    *(end + 1) = '\0';
}

// main function
int main (int argc, char* argv[]) {
    int status = 0;
    struct mg_mgr mgr;
    mg_mgr_init (&mgr, NULL);

    status = config_get ();
    if (status != -1) {
        _alive = 1;
        while (_alive) {
            if (mg_connect_http (&mgr, http_event_handler, _req_uri, NULL, NULL) == NULL) {
                printf ("failed to connect to %s\n", _req_uri);
            } else {
                _connection_open = 1;
                while (_connection_open) {
                    mg_mgr_poll (&mgr, 1000);
                }
            }
            sleep (S_POLL_TIME);
        }
    }
    mg_mgr_free (&mgr);
    free (_req_uri);
    free (_json_nodes_path);
    return status;
}
