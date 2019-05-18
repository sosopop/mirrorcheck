#include "mirroraccel_inter.h"
#include "mirroraccel_thread.h"

#define MA_ERROR_GEN(n, s) {"CODE_" #n, s},
static struct
{
    const char *name;
    const char *description;
} ma_strerror_tab[] = {MA_ERROR_MAP(MA_ERROR_GEN)};
#undef MA_ERROR_GEN

static struct ma_srv_tree_s srv_tree;
static ma_mutex_t mtx_srv;

static int ma_srv_compare(struct ma_srv_s *a, struct ma_srv_s *b)
{
    return a->port - b->port;
}

RB_GENERATE_STATIC(ma_srv_tree_s, ma_srv_s, tree_entry, ma_srv_compare);

static void free_srv_data(struct ma_srv_s *srv)
{
    if (srv->thd)
    {
        srv->stop_signal = 1;
        ma_thread_join(&srv->thd);
        ma_thread_destroy(srv->thd);
    }
    printf("remove port = %d \n", srv->port);
    RB_REMOVE(ma_srv_tree_s, &srv_tree, srv);
    mg_mgr_free(&srv->mgr);
    free(srv);
}

void mirror_accel_init()
{
    ma_mutex_init(&mtx_srv);
    RB_INIT(&srv_tree);
}

void mirror_accel_uninit()
{
    struct ma_srv_s *srv = 0;
    struct ma_srv_s *x = 0;
    RB_FOREACH_REVERSE_SAFE(srv, ma_srv_tree_s, &srv_tree, x)
    {
        free_srv_data(srv);
    }
    ma_mutex_destroy(&mtx_srv);
}

static int start_with(const struct mg_str *str1, const char *str2)
{
    size_t i = 0;
    while (str2[i] && i < str1->len && str2[i] == str1->p[i])
        i++;
    return str2[i];
}

static const char *json_response(int code)
{
    return 0;
}

static void parse_mirrors_array(const char *str, int len, void *user_data)
{
    struct ma_accel_vir_conn_s * vc = (struct ma_accel_vir_conn_s *)user_data;
    struct json_token t;
    int i;
    for (i = 0; json_scanf_array_elem(str, len, "", i, &t) > 0; i++)
    {
        char *url = NULL;
        json_scanf(t.ptr, t.len, "{url:%Q}", &url);
        if (url) {
            struct ma_mirror_item_s* item = (struct ma_mirror_item_s*)malloc(sizeof(struct ma_mirror_item_s));
            item->url = url;
            QUEUE_INSERT_TAIL(&vc->mirror_items, &item->link);
        }
    }
}

static int api_conn_new(struct mg_connection *nc, int ev, void *p, void *user_data)
{
    int ret = 0;
    struct http_message *hm = (struct http_message *)p;
    struct ma_accel_vir_conn_s *vc = 0;
    if (user_data)
    {
        ret = -1;
        goto cleanup;
    }
    vc = (struct ma_accel_vir_conn_s *)malloc(sizeof(struct ma_accel_vir_conn_s));
    memset(vc, 0, sizeof(struct ma_accel_vir_conn_s));
    QUEUE_INIT(&vc->mirror_items);
    vc->type = MA_CONN_TYPE_VC;

    nc->user_data = vc;

    json_scanf(hm->body.p, hm->body.len, "{targets:%M}", parse_mirrors_array, vc);

cleanup:
    if (ret)
    {
        free(vc);
        nc->user_data = 0;
    }

    return ret;
}

static int api_conn_delete(struct mg_connection *nc, int ev, void *p, void *user_data)
{
    int ret = 0;
    struct http_message *hm = (struct http_message *)p;
    return ret;
}

static int api_conn_get(struct mg_connection *nc, int ev, void *p, void *user_data)
{
    int ret = 0;
    struct http_message *hm = (struct http_message *)p;
    return ret;
}

static void ev_handler(struct mg_connection *nc, int ev, void *p, void *user_data)
{
    if (ev == MG_EV_HTTP_REQUEST)
    {
        struct http_message *hm = (struct http_message *)p;
        if (mg_vcmp(&hm->uri, "/api/conn") == 0)
        {
            if (mg_vcmp(&hm->method, "POST") == 0)
            {
                api_conn_new(nc, ev, p, user_data);
            }
            else if (mg_vcmp(&hm->method, "DELETE") == 0)
            {
                api_conn_delete(nc, ev, p, user_data);
            }
            else if (mg_vcmp(&hm->method, "GET") == 0)
            {
                api_conn_get(nc, ev, p, user_data);
            }
        }
        else if (start_with(&hm->uri, "/stream/") == 0)
        {
            mg_send_head(nc, 200, sizeof("world") - 1, 0);
            mg_send(nc, "world", sizeof("world") - 1);
        }
        else
        {
            mg_send_head(nc, 200, sizeof("!!!") - 1, 0);
            mg_send(nc, "!!!", sizeof("!!!") - 1);
        }
    }
}

static unsigned int MA_STDCALL srv_poll_thread(void *arg)
{
    struct ma_srv_s *srv = (struct ma_srv_s *)arg;
    while (!srv->stop_signal)
    {
        mg_mgr_poll(&srv->mgr, 100);
    }
    return 0;
}

int mirror_accel_create(const char *addr)
{
    int port = 0;
    char port_buf[10] = {0};
    struct mg_connection *nc;
    struct ma_srv_s *srv = 0;

    ma_mutex_acquire(&mtx_srv);
    srv = (struct ma_srv_s *)malloc(sizeof(struct ma_srv_s));
    memset(srv, 0, sizeof(struct ma_srv_s));

    mg_mgr_init(&srv->mgr, srv);
    nc = mg_bind(&srv->mgr, addr, ev_handler, 0);
    if (nc == NULL)
    {
        printf("failed to create listener\n");
        goto cleanup;
    }
    mg_set_protocol_http_websocket(nc);
    mg_conn_addr_to_str(nc, port_buf, sizeof(port_buf), MG_SOCK_STRINGIFY_PORT);
    port = atoi(port_buf);
    srv->port = port;
    printf("create port = %d \n", port);
    RB_INSERT(ma_srv_tree_s, &srv_tree, srv);
    srv->thd = ma_thread_create(srv_poll_thread, srv);
cleanup:
    if (port <= 0)
    {
        if (srv)
        {
            mg_mgr_free(&srv->mgr);
            free(srv);
        }
    }
    ma_mutex_release(&mtx_srv);
    return port;
}

void mirror_accel_destroy(int port)
{
    ma_mutex_acquire(&mtx_srv);
    struct ma_srv_s srv_port;
    srv_port.port = port;
    struct ma_srv_s *srv = RB_FIND(ma_srv_tree_s, &srv_tree, &srv_port);
    if (srv)
    {
        free_srv_data(srv);
    }
    ma_mutex_release(&mtx_srv);
}