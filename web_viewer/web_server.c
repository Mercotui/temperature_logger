//-----includes
#include <sqlite3.h>
#include <string.h>

#include "../mongoose/mongoose.h"

//-----defines
#define DEFAULT_PORT "8000"
#define DATA_PATH "/data.json"
#define MAX_JSON_DATA_LEN 1000 // arbitrary lenght
struct db_response {
    int argc;
    char** argv;
    char** azColName;
};

//-----static variables
static int alive;
static struct mg_serve_http_opts s_http_server_opts;

//-----static functions
static void ev_handler (struct mg_connection* nc, int ev, void* p);
static void serve_json_data (struct mg_connection* nc, struct http_message* hm);
static struct db_response* get_data (void);
static size_t format_data ();
static const char* json_cat_str (char* dest, const char* src);

//-----function defenitions
// http functions
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
    char json_buff;

    struct db_response* data = get_data ();
    data_len                 = format_data (&json_buff, data);

    mg_printf (nc, "HTTP/1.1 200 OK\r\n"
                   "Cache: no-cache\r\n"
                   "Content-Type: application/json\r\n"
                   "Content-Length: %zu\r\n"
                   "\r\n%s\r\n",
    data_len, &json_buff);
}

// database functions
static struct db_response* get_data (void) {
    struct db_response* ret = malloc (sizeof (struct db_response));
    char* err_msg           = NULL;
    int status;
    sqlite3* db;

    status = sqlite3_open ("temperatures.db", &db);
    if (status != SQLITE_OK) {
        fprintf (stderr, "Cannot open database: %s\n", sqlite3_errmsg (db));
    } else {
        status = sqlite3_exec (db,
        "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;",
        format_data, buffer, &err_msg);
        if (status != SQLITE_OK) {
            printf ("SQL error: %s\n", err_msg);
            sqlite3_free (err_msg);
        }
    }

    sqlite3_close (db);
    return;
}

// json functions
static size_t format_data (char** json_buff2) {

    return 5;
}

static const char* json_cat_str (char* dest, const char* src) {
    int i;

    for (i = 0;;) {
        ;
    }

    return &dest[i];
}

int main (int argc, char* argv[]) {
    static const char* s_http_port = DEFAULT_PORT;
    struct mg_mgr mgr;
    struct mg_connection* nc;

    // Set up HTTP server parameters
    s_http_server_opts.document_root            = "./web_viewer/web_site";
    s_http_server_opts.enable_directory_listing = "yes";

    //  init mongoose
    mg_mgr_init (&mgr, NULL);
    nc = mg_bind (&mgr, s_http_port, ev_handler);
    mg_set_protocol_http_websocket (nc);

    printf ("Listening on port: %s\n", s_http_port);
    while (alive) {
        mg_mgr_poll (&mgr, 1000);
    }
    mg_mgr_free (&mgr);

    return 0;
}
