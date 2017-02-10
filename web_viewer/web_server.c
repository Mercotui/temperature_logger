//-----includes
#include <signal.h>
#include <sqlite3.h>
#include <string.h>

#include "../mongoose/mongoose.h"

//-----defines
#define WEBSITE_DIR "./web_site"
#define DEFAULT_PORT "8000"
#define DATA_PATH "/data.json"
#define MAX_JSON_DATA_LEN 1000 // arbitrary lenght
struct db_response {
    int argc;
    char** argv;
    char** col_names;
};

//-----static variables
static int alive;
static struct mg_serve_http_opts s_http_server_opts;

//-----static functions
static void ev_handler (struct mg_connection* connection, int ev, void* p);
static void serve_json_data (struct mg_connection* connection, struct http_message* message);
static int get_data (struct db_response* data);
int db_callback (void* ret, int argc, char** argv, char** col_names);
static size_t format_data ();
static void sigint_handler (int sig);

//-----function defenitions
// http functions
static void ev_handler (struct mg_connection* connection, int ev, void* p) {
    struct http_message* message = (struct http_message*)p;
    if (ev == MG_EV_HTTP_REQUEST) {
        if ((strlen (DATA_PATH) == message->uri.len) &&
        (strncmp (DATA_PATH, message->uri.p, message->uri.len) == 0)) {
            serve_json_data (connection, message);
        } else {
            mg_serve_http (connection, message, s_http_server_opts);
        }
        printf ("Served: %.*s\n", (int)message->uri.len, message->uri.p);
    }
}

static void serve_json_data (struct mg_connection* connection, struct http_message* message) {
    size_t data_len;
    char* json_buff;

    struct db_response data;
    get_data (&data);
    data_len = format_data (data, &json_buff);

    mg_printf (connection, "HTTP/1.1 200 OK\r\n"
                           "Cache: no-cache\r\n"
                           "Content-Type: application/json\r\n"
                           "Content-Length: %zu\r\n"
                           "\r\n%s\r\n",
    data_len, json_buff);

    free (json_buff);
}

// database functions
static int get_data (struct db_response* data) {
    struct db_response* ret = calloc (1, sizeof (struct db_response));
    char* err_msg           = NULL;
    int status;
    sqlite3* db;

    status = sqlite3_open ("temperatures.db", &db);
    if (status != SQLITE_OK) {
        fprintf (stderr, "Cannot open database: %s\n", sqlite3_errmsg (db));
    } else {
        status = sqlite3_exec (db,
        "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;",
        db_callback, &data, &err_msg);
        if (status != SQLITE_OK) {
            printf ("SQL error: %s\n", err_msg);
            sqlite3_free (err_msg);
        }
    }

    sqlite3_close (db);
    return status;
}

int db_callback (void* ret, int argc, char** argv, char** col_names) {
    ((struct db_response*)ret)->argc      = argc;
    ((struct db_response*)ret)->argv      = argv;
    ((struct db_response*)ret)->col_names = col_names;
    return 0;
}

// json functions
static size_t format_data (struct db_response* data, char** resp_buff) {
    char* json_buff         = (char*)malloc (sizeof (char) * 1000);
    unsigned int char_count = 0;

    json_buff[char_count++] = '{';
    for (int idx = 0; idx < data->argc; idx++) {
        printf ("%s\n", data->argv[0]);
        /*    for (table = tables[idx];;) {
                ll;
            }*/
    }

    json_buff[char_count++] = '}';

    *resp_buff = json_buff;
    return char_count;
}

// MAIN functions
void sigint_handler (int sig) {
    alive = 0;
}

int main (int argc, char* argv[]) {
    static const char* s_http_port = DEFAULT_PORT;
    struct mg_mgr mgr;
    struct mg_connection* connection;
    struct sigaction handler;

    alive              = 1;
    handler.sa_handler = sigint_handler;
    sigaction (SIGINT, &handler, NULL);

    // Set up HTTP server parameters
    s_http_server_opts.document_root            = WEBSITE_DIR;
    s_http_server_opts.enable_directory_listing = "yes";

    //  init mongoose
    mg_mgr_init (&mgr, NULL);
    connection = mg_bind (&mgr, s_http_port, ev_handler);
    mg_set_protocol_http_websocket (connection);

    printf ("Listening on port: %s\n", s_http_port);
    while (alive) {
        mg_mgr_poll (&mgr, 1000);
    }
    mg_mgr_free (&mgr);

    return 0;
}
