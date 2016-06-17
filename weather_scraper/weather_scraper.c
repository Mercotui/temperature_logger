//-----includes
#include <pthread.h>
#include <stdio.h>
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
static void ev_handler (struct mg_connection* nc, int ev, void* ev_data);
static int parse_resp (const char* body, const size_t len);
static void insert_temp_db (const int temperature);
static int get_conf (void);
static int parse_conf (const char* uri_line, const char* node_line);
static void* http_req_thread (void*);

//-----function defenitions
static void ev_handler (struct mg_connection* nc, int ev, void* ev_data) {
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
            int temperature = parse_resp (hm->body.p, hm->body.len);
            insert_temp_db (temperature);
            break;
        default: break;
    }
}

static int parse_resp (const char* body, const size_t len) {
    struct json_token *arr, *arr2, *tok;
    arr = parse_json2 (body, len);

    tok  = find_json_token (arr, "\"current\"");
    arr2 = parse_json2 (tok->ptr, tok->len);
    tok  = find_json_token (arr2, "\"temp_c\"");

    fwrite (tok->ptr, 1, tok->len, stdout);

    return 4;
}

static void insert_temp_db (const int temperature) {
    ; // sqmagicl here
    ; // insert temperature in celcius - time in minutes
}

static int get_conf (void) {
    int status = -1;
    FILE* conf_file;
    char *uri_line = NULL, *node_line = NULL;
    size_t read_buff_size = 0;
    ssize_t read_size;

    conf_file = fopen (CONF_PATH, "r");
    if (conf_file != NULL) {
        read_size = getline (&uri_line, &read_buff_size, conf_file);
        if (read_size > 0) {
            read_buff_size = 0;
            read_size      = getline (&node_line, &read_buff_size, conf_file);
            if (read_size > 0) {
                status = parse_conf (uri_line, node_line);
            } else {
                printf ("No second line in %s\n", CONF_PATH);
            }
        } else {
            printf ("No contents in %s\n", CONF_PATH);
        }
        fclose (conf_file);
    } else {
        printf ("Can not open %s\n", CONF_PATH);
    }

    free (uri_line);
    free (node_line);

    return status;
}

static int parse_conf (const char* uri_line, const char* node_line) {
    int status = 0;

    if (strncmp (uri_line, CONF_URI_HEADER, strlen (CONF_URI_HEADER)) == 0) {
        http_req_uri = strdup (uri_line + strlen (CONF_URI_HEADER));
    } else {
        status = -1;
        printf ("%s does not follow format %s=http://example.com/api/weather\n",
        CONF_URI_HEADER, CONF_URI_HEADER);
    }
    if (strncmp (node_line, CONF_JSONPATH_HEADER, strlen (CONF_JSONPATH_HEADER)) == 0) {
        json_temparature_node = strdup (node_line + strlen (CONF_JSONPATH_HEADER));
    } else {
        status = -1;
        printf ("%s does not follow format %s=example.weather.current.temp\n",
        CONF_JSONPATH_HEADER, CONF_JSONPATH_HEADER);
    }

    return status;
}

static void* http_req_thread (void* data) {
    struct mg_mgr* mgr = (struct mg_mgr*)data;

    while (1) {
        mg_connect_http (mgr, ev_handler, http_req_uri, NULL, NULL);
        sleep (S_POLL_TIME);
    }

    return NULL;
}

int main (int argc, char* argv[]) {
    int status = 0;
    struct mg_mgr mgr;
    mg_mgr_init (&mgr, NULL);

    status = get_conf ();
    if (status != -1) {
        pthread_t req_thread;
        pthread_create (&req_thread, NULL, http_req_thread, &mgr);

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
