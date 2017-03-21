//-----includes
#define _GNU_SOURCE
#include <signal.h>
#include <sqlite3.h>
#include <string.h>

#include "../mongoose_http/mongoose.h"
#include "../parson_json/parson.h"

//-----defines
#define WEBSITE_DIR "./web_site"
#define DEFAULT_PORT "8000"
#define DATA_PATH "/data.json"
#define MAX_JSON_DATA_LEN 1000 // arbitrary lenght
#define MAX_TABLE_COUNT 10     // arbitrary
#define SQLCMD_FORMAT_NAMES \
    "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;"
#define SQLCMD_FORMAT_CONTENT \
    "SELECT * FROM %s WHERE Timestamp >= datetime('now','%s');"

struct db_data_node {
    char* time_stamp;
    char* temp;
    struct db_data_node* next;
};

struct db_table {
    char* name;
    struct db_data_node* first_data_node;
};

//-----static variables
static int alive;
static struct mg_serve_http_opts s_http_server_opts;
static struct db_table db_tables[MAX_TABLE_COUNT];

//-----static functions
static void ev_handler (struct mg_connection* connection, int ev, void* p);
static void serve_json_data (struct mg_connection* connection, struct http_message* message);
static int get_data (void);
int db_callback_names (void* ret, int argc, char** argv, char** col_names);
int db_callback_content (void* data, int argc, char** argv, char** col_names);
void cleanup_data (void);
static char* format_data (void);
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
    (void)message;
    int status;
    size_t data_len;
    char* json_buff;

    status = get_data ();
    if (status == SQLITE_OK) {
        json_buff = format_data ();
        data_len  = strlen (json_buff);

        mg_printf (connection, "HTTP/1.1 200 OK\r\n"
                               "Cache: no-cache\r\n"
                               "Content-Type: application/json\r\n"
                               "Content-Length: %zu\r\n"
                               "\r\n%s\r\n",
        data_len, json_buff);

        json_free_serialized_string (json_buff);
        cleanup_data ();
    } else {
        mg_http_send_error (connection, 500, NULL);
    }
}

// database functions
static int get_data (void) {
    char* err_msg = NULL;
    int status;
    sqlite3* db;

    memset (&db_tables, 0, (sizeof (struct db_table) * MAX_TABLE_COUNT));

    status = sqlite3_open ("temperatures.db", &db);
    if (status != SQLITE_OK) {
        fprintf (stderr, "Cannot open database: %s\n", sqlite3_errmsg (db));
    } else {
        int tables_idx = 0;
        status         = sqlite3_exec (
        db, SQLCMD_FORMAT_NAMES, db_callback_names, &tables_idx, &err_msg);
        if (status == SQLITE_OK) {
            char* sqlcmd;
            char* time_span = "-1 month";

            for (int i = 0; (i < MAX_TABLE_COUNT) && (db_tables[i].name != NULL); ++i) {
                struct db_data_node** end_node_pointer = &(db_tables[i].first_data_node);

                asprintf (&sqlcmd, SQLCMD_FORMAT_CONTENT, db_tables[i].name, time_span);
                status = sqlite3_exec (
                db, sqlcmd, db_callback_content, &end_node_pointer, &err_msg);
                if (status != SQLITE_OK) {
                    printf ("SQL error: %s\n", err_msg);
                    sqlite3_free (err_msg);
                }
                free (sqlcmd);
            }
        } else {
            printf ("SQL error: %s\n", err_msg);
            sqlite3_free (err_msg);
        }
    }

    sqlite3_close (db);
    return status;
}

int db_callback_names (void* data, int argc, char** argv, char** col_names) {
    int status       = 1;
    int* idx_pointer = (int*)data;
    int idx          = *idx_pointer;

    if (argc == 1) {
        if ((strlen (col_names[0]) == strlen ("name")) &&
        (strcmp (col_names[0], "name") == 0)) {
            status = 0;
        }
    }

    if (status == 0) {
        size_t name_lenght  = strlen (argv[0]);
        db_tables[idx].name = malloc (name_lenght + 1);
        strncpy (db_tables[idx].name, argv[idx], name_lenght + 1);

        (*idx_pointer)++;
    }

    return status;
}

int db_callback_content (void* data, int argc, char** argv, char** col_names) {
    int status                              = 1;
    struct db_data_node*** end_node_pointer = (struct db_data_node***)data;

    // check if the data is formatted correctly
    if (argc == 2) {
        if ((strlen (col_names[0]) == strlen ("Timestamp")) &&
        (strcmp (col_names[0], "Timestamp") == 0)) {
            if ((strlen (col_names[1]) == strlen ("Temperature")) &&
            (strcmp (col_names[1], "Temperature") == 0)) {
                status = 0;
            }
        }
    }

    // if passed format check, start reading
    if (status == 0) {
        struct db_data_node* data_node = malloc (sizeof (struct db_data_node));
        memset (data_node, 0, sizeof (struct db_data_node));

        size_t time_lenght    = strlen (argv[0]);
        data_node->time_stamp = malloc (time_lenght + 1);
        strncpy (data_node->time_stamp, argv[0], time_lenght + 1);

        size_t temp_lenght = strlen (argv[1]);
        data_node->temp    = malloc (temp_lenght + 1);
        strncpy (data_node->temp, argv[1], temp_lenght + 1);

        **end_node_pointer = data_node;
        *end_node_pointer  = &(data_node->next);
    }

    return status;
}

void cleanup_data (void) {
    for (int table_idx = 0;
         (table_idx < MAX_TABLE_COUNT) && (db_tables[table_idx].name != NULL); ++table_idx) {
        struct db_data_node* data_node = db_tables[table_idx].first_data_node;
        while (data_node != NULL) {
            struct db_data_node* next_data_node = data_node->next;
            free (data_node->time_stamp);
            free (data_node->temp);
            free (data_node);
            data_node = next_data_node;
        }
        free (db_tables[table_idx].name);
    }
}

// json functions
static char* format_data (void) {
    JSON_Value* root_value = json_value_init_object ();
    // JSON_Object* root_object = json_value_get_object (root_value);
    char* serialized_string = NULL;
    for (int table_idx = 0;
         (table_idx < MAX_TABLE_COUNT) && (db_tables[table_idx].name != NULL); ++table_idx) {

        struct db_data_node* data_node = db_tables[table_idx].first_data_node;
        while (data_node != NULL) {
            // json_array_append_value (array, json_value_init_number (double
            // number));
            // json_object_set_string (root_object, "name", "John Smith");
            // json_object_set_number (root_object, "age", 25);

            printf ("%s: %s\n", data_node->time_stamp, data_node->temp);

            data_node = data_node->next;
        }
    }

    serialized_string = json_serialize_to_string (root_value);
    printf ("%s\n", serialized_string);

    // free internal json structures
    json_value_free (root_value);

    // return json string
    return serialized_string;
}

// MAIN functions
static void sigint_handler (int sig) {
    (void)sig;
    alive = 0;
}

int main (int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    static const char* s_http_port = DEFAULT_PORT;
    struct mg_mgr mgr;
    struct mg_connection* connection;
    struct sigaction handler;

    alive = 1;
    memset (&handler, 0, sizeof (struct sigaction));
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
