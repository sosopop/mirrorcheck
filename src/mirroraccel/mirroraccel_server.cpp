#include "mirroraccel_server.h"

mirroraccel::Server::Server(
	const std::string & addr, 
	const std::string & jsonOption)
{
	int port = 0;
	char port_buf[10] = { 0 };
	struct mg_connection *nc;
	struct ma_srv_s *srv = 0;

	ma_mutex_acquire(&mtx_srv);

	//初始化server
	srv = (struct ma_srv_s *)malloc(sizeof(struct ma_srv_s));
	memset(srv, 0, sizeof(struct ma_srv_s));
	QUEUE_INIT(&srv->mirror_items);

	//解析json选项
	if (!json_opt || !json_opt[0])
	{
		goto cleanup;
	}
	if (json_scanf(json_opt, strlen(json_opt), "{targets:%M}", parse_mirrors_array, srv) < 0)
	{
		goto cleanup;
	}

	//初始化mongoose服务器
	mg_mgr_init(&srv->mgr, srv);
	nc = mg_bind(&srv->mgr, addr, ev_handler, 0);
	if (nc == NULL)
	{
		printf("failed to create listener\n");
		goto cleanup;
	}
	mg_set_protocol_http_websocket(nc);
	//获取监听端口
	mg_conn_addr_to_str(nc, port_buf, sizeof(port_buf), MG_SOCK_STRINGIFY_PORT);
	port = atoi(port_buf);
	srv->port = port;
	printf("create port = %d \n", port);

	//加入到服务树
	RB_INSERT(ma_srv_tree_s, &srv_tree, srv);

	//创建mongoose事件循环线程
	srv->thd_server = ma_thread_create(srv_poll_thread, srv);
cleanup:
	if (port <= 0)
	{
		if (srv)
		{
			free_srv_data(srv);
		}
	}
	ma_mutex_release(&mtx_srv);
	return port;
}

mirroraccel::Server::~Server()
{

}

int mirroraccel::Server::getPort()
{
	return port;
}
