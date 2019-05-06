#include "mirroraccel_inter.h"

struct ma_srv_tree_s srv_tree;

void mirror_accel_init()
{
    RB_INIT(&srv_tree);
}

void mirror_accel_uninit()
{
    struct ma_srv_s *h = 0;
    RB_FOREACH_REVERSE(h, ma_srv_tree_s, &srv_tree) {
        printf("sorted port = %d \n", h->port);
    }
}

static void ev_handler(struct mg_connection *nc, int ev, void *p, void* user_data) {
  if (ev == MG_EV_HTTP_REQUEST) {
    //mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
  }
}

int mirror_accel_create( const char* addr)
{
    int port = 0;
    char port_buf[10] = {0};
    struct mg_connection *nc;
    struct ma_srv_s* srv = malloc(sizeof(struct ma_srv_s));
    memset(srv, 0, sizeof(struct ma_srv_s));

    mg_mgr_init(&srv->mgr, srv);
    nc = mg_bind(&srv->mgr, addr, ev_handler, 0);
    if (nc == NULL) {
        printf("Failed to create listener\n");
        goto cleanup;
    }
    mg_set_protocol_http_websocket(nc);
    mg_conn_addr_to_str(nc, port_buf, sizeof(port_buf), MG_SOCK_STRINGIFY_PORT);
    port = atoi(port_buf);
    srv->port = port;
    RB_INSERT(ma_srv_tree_s, &srv_tree, srv);

cleanup:
    if( port <= 0) {
        if(srv) {
            mg_mgr_free(&srv->mgr);
            free(srv);
        }
    }
    return port;
}

void mirror_accel_destroy( int port )
{
}

int ma_srv_compare(struct ma_srv_s *a, struct ma_srv_s *b)
{
    return a->port - b->port;
}

RB_GENERATE(ma_srv_tree_s, ma_srv_s, tree_entry, ma_srv_compare);