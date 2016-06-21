//-----includes
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../mongoose/mongoose.h"

//-----defines
#define S_POLL_TIME 300 // 5 minutes
#define CONF_PATH "./weather_scraper/weather_scraper.conf"
#define CONF_URI_HEADER "weather uri="
#define CONF_JSONPATH_HEADER "termperature json node="

//-----static variables
static char* http_req_uri;
static char* json_temparature_node;

//-----static functions
static void* http_request_thread (void*);
static void http_event_handler (struct mg_connection* nc, int ev, void* ev_data);
static double http_parse_resp (const char* body, const size_t len);
static void db_insert_temperature (const double temperature);
static int config_get (void);
static int config_parse (const char* uri_line, const char* node_line);
static void config_trim_line (char* str);

//-----function defenitions
// http functions
static void* http_request_thread (void* data) {
    struct mg_mgr* mgr = (struct mg_mgr*)data;

    while (1) {
        mg_connect_http (mgr, http_event_handler, http_req_uri, NULL, NULL);
        sleep (S_POLL_TIME);
    }

    return NULL;
}

static void http_event_handler (struct mg_connection* nc, int ev, void* ev_data) {
    struct http_message* hm = (struct http_message*)ev_data;

    switch (ev) {
        case MG_EV_CONNECT:
            if (*(int*)ev_data != 0) {
                fprintf (stderr, "connect() failed: %s\n", strerror (*(int*)ev_data));
            }
            break;
        case MG_EV_HTTP_REPLY:
            fwrite (hm->body.p, 1, hm->body.len, stdout);
            putchar ('\n');
            double temperature = http_parse_resp (hm->body.p, hm->body.len);
            db_insert_temperature (temperature);
            break;
        default: break;
    }
}

static double http_parse_resp (const char* body, const size_t len) {
    struct json_token *arr, *arr2, *tok;

    arr  = parse_json2 (body, len);
    tok  = find_json_token (arr, "current");
    arr2 = parse_json2 (tok->ptr, tok->len);
    tok  = find_json_token (arr2, "temp_c");

    double ret;
    char* endptr;
    ret = strtod (tok->ptr, &endptr);

    if (endptr == tok->ptr) {
        printf ("Could not convert \"%s\" value was \"%.*s\"\n",
        json_temparature_node, tok->len, tok->ptr);
    }

    return ret;
}

// database functions
static void db_insert_temperature (const double temperature) {
    ; // sqmagicl here
    ; // insert temperature in celcius - time in minutes
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
        http_req_uri = strdup (uri_line + strlen (CONF_URI_HEADER));
        config_trim_line (http_req_uri);
    } else {
        status = -1;
        printf ("%s does not follow format %s=http://example.com/api/weather\n",
        CONF_URI_HEADER, CONF_URI_HEADER);
    }

    if (strncmp (node_line, CONF_JSONPATH_HEADER, strlen (CONF_JSONPATH_HEADER)) == 0) {
        json_temparature_node = strdup (node_line + strlen (CONF_JSONPATH_HEADER));
        config_trim_line (json_temparature_node);
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

    // Write new null terminator
    *(end + 1) = '\0';
}

// main function
int main (int argc, char* argv[]) {
    int status = 0;
    struct mg_mgr mgr;
    mg_mgr_init (&mgr, NULL);

    status = config_get ();
    if (status != -1) {
        pthread_t req_thread;
        pthread_create (&req_thread, NULL, http_request_thread, &mgr);

        while (1) {
            mg_mgr_poll (&mgr, 1000);
        }

        pthread_join (req_thread, NULL);
        mg_mgr_free (&mgr);
        free (http_req_uri);
        free (json_temparature_node);
    }
    return status;
}
