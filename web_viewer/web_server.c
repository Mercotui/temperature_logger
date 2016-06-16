//-----includes
#include <string.h>

#include "../mongoose/mongoose.h"

//-----defines
#define DATA_PATH "/data.json"
#define MAX_JSON_DATA_LEN 1000 // arbitrary lenght

//-----static variables
static const char* s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;
char json_buffer[MAX_JSON_DATA_LEN];

//-----static functions
static void ev_handler (struct mg_connection* nc, int ev, void* p);
static void serve_json_data (struct mg_connection* nc, struct http_message* hm);
static void get_data ();
static size_t format_data ();
static const char* json_cat_str (char* dest, const char* src);

//-----function defenitions
static void ev_handler (struct mg_connection* nc, int ev, void* p) {
    struct http_message* hm = (struct http_message*)p;
    if (ev == MG_EV_HTTP_REQUEST) {
        if ((strlen (DATA_PATH) == hm->uri.len) &&
        (strncmp (DATA_PATH, hm->uri.p, hm->uri.len) == 0)) {
            serve_json_data (nc, hm);
        } else {
            mg_serve_http (nc, hm, s_http_server_opts);
        }
        printf ("Served: %.*s\n", (int)hm->uri.len, hm->uri.p);
    }
}

static void serve_json_data (struct mg_connection* nc, struct http_message* hm) {
    size_t data_len;

    get_data ();
    data_len = format_data ();

    mg_printf (nc, "HTTP/1.1 200 OK\r\n"
                   "Cache: no-cache\r\n"
                   "Content-Type: application/json\r\n"
                   "Content-Length: %zu\r\n"
                   "\r\n%s\r\n",
    data_len, json_buffer);
}

static void get_data () {
    ;
}

static size_t format_data () {


    return 5;
}

static const char* json_cat_str (char* dest, const char* src) {
    int i;

    for (i = 0;;) {
        ;
    }

    return &dest[i];
}

int main (void) {
    struct mg_mgr mgr;
    struct mg_connection* nc;

    mg_mgr_init (&mgr, NULL);
    nc = mg_bind (&mgr, s_http_port, ev_handler);

    // Set up HTTP server parameters
    mg_set_protocol_http_websocket (nc);
    s_http_server_opts.document_root =
    "./web_viewer/web_site";                    // Serve current directory
    s_http_server_opts.dav_document_root = "."; // Allow access via WebDav
    s_http_server_opts.enable_directory_listing = "yes";

    printf ("Starting web server on port %s\n", s_http_port);
    while (1) {
        mg_mgr_poll (&mgr, 1000);
    }
    mg_mgr_free (&mgr);

    return 0;
}
