//-----includes
#include <pthread.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../frozen/frozen.h"
#include "../mongoose/mongoose.h"

//-----defines
#define S_POLL_TIME 300 // 5 minutes
#define SQLCMD_FORMAT "INSERT INTO Weather VALUES(CURRENT_TIMESTAMP, %s);"
#define CONF_PATH "./weather_scraper.conf"
#define CONF_URI_HEADER "weather uri="
#define CONF_JSONPATH_HEADER "termperature json node="

//-----global variables
static char* _req_uri;
static char** _json_nodes;
int _alive;
int _connection_open;

//-----static functions
static void* http_request_thread (void*);
static void http_event_handler (struct mg_connection* nc, int ev, void* ev_data);
static char* http_parse_resp (const char* body, const size_t len);
static void db_insert_temperature (char* temperature);
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
            char* temperature = http_parse_resp (hm->body.p, hm->body.len);
            if (temperature != NULL) {
                db_insert_temperature (temperature);
                free (temperature);
            }
            _connection_open = 0;
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

static char* http_parse_resp (const char* body, const size_t len) {
    char* ret = NULL;
    struct json_token *arr, *arr2, *tok;

    arr  = parse_json2 (body, len);
    tok  = find_json_token (arr, "main");
    arr2 = parse_json2 (tok->ptr, tok->len);
    tok  = find_json_token (arr2, "temp");

    char* endptr;
    strtod (tok->ptr, &endptr);
    if (endptr == tok->ptr) {
        printf ("Given temperature node \"%s\" not valid, value was
       \"%.*s\"\n", json_temparature_node, tok->len, tok->ptr);
    } else {
        ret = strndup (tok->ptr, tok->len);
    }

    return ret;
}

// database functions
// CREATE TABLE Weather (Time INT?, Temperature DOUBLE);
static void db_insert_temperature (char* temperature) {
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
            size_t sqlcmd_len = strlen (SQLCMD_FORMAT) + strlen (temperature);
            char* sqlcmd      = malloc (sqlcmd_len);

            sprintf (sqlcmd, SQLCMD_FORMAT, temperature);
            status = sqlite3_exec (db, sqlcmd, NULL, NULL, &err_msg);
            if (status == SQLITE_OK) {
                printf ("Inserted %s\n", temperature);
            } else {
                printf ("SQL error: %s\n", err_msg);
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
        _json_nodes  = malloc (sizeof (char*));
        *_json_nodes = strdup (node_line + strlen (CONF_JSONPATH_HEADER));
        config_trim_line (*_json_nodes);
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
    free (_json_nodes);
    return status;
}
