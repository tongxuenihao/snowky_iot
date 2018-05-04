/*************************************************************************
	> File Name: rlt_tcp_send.c
	> Author: xuetong
	> Mail: 
	> Created Time: Sat 28 Apr 2018 09:40:15 AM CST
 ************************************************************************/
#include "common.h"

extern xQueueHandle msg_queue;
int socket_fd = -1;

int http_status = 0;
int m2m_status = 0;

static int m2m_port;
static char m2m_server[100] = {0};

void set_http_status(int flag)
{
	http_status = flag;
}

void set_m2m_status(int flag)
{
	m2m_status = flag;
}


int tcp_socket_set(unsigned char *host, unsigned int server_port, unsigned int reconnect_flag)
{
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
	int local_fd = -1;
	struct sockaddr_in local_addr;
	static struct sockaddr_in remote_addr;
	struct hostent *server_host;
	static int reconnect_times = 0;

	static short sev_port;

	if(reconnect_flag == 0)
	{
		server_host = gethostbyname(host);
		memcpy((void *) &remote_addr.sin_addr, (void *) server_host->h_addr, server_host->h_length);
		sev_port = server_port;
	}

	if(reconnect_times > 20)
	{
		log_printf(LOG_DEBUG"[%s]sys reboot\n",__FUNCTION__);
		sys_reset();
	}
  
	log_printf(LOG_DEBUG"[%s]%s(%d)\n",__FUNCTION__,((reconnect_times == 0) ? "CONNECT..." : "RECONNECT..."),reconnect_times);
	if(socket_fd == -1)
	{
		if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) >= 0) 
		{
			local_addr.sin_family = AF_INET;
			local_addr.sin_port = INADDR_ANY;
			local_addr.sin_addr.s_addr = INADDR_ANY;

			if(bind(socket_fd, (struct sockaddr *) &local_addr, sizeof(local_addr)) < 0) 
			{
				log_printf(LOG_WARNING"[%s]bind error\n",__FUNCTION__);
				close(socket_fd);
				reconnect_times++;
				return -1;
			}
			remote_addr.sin_family = AF_INET;
			remote_addr.sin_port = htons(sev_port);
			remote_addr.sin_addr.s_addr = inet_addr("192.168.0.2");

			if(connect(socket_fd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) < 0)
			{
				log_printf(LOG_WARNING"[%s]connect error\n",__FUNCTION__);
				close(socket_fd);
				reconnect_times++;
				return -1;
			}
			log_printf(LOG_DEBUG"[%s]connect OK\n",__FUNCTION__);
		}
		else
		{
			log_printf(LOG_WARNING"[%s]socket error\n",__FUNCTION__);
			return -1;
		}
	}
	return socket_fd;

}

void rlk_tcp_send_func(int argc, char *argv[])
{
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);

	int response_code;
	int server_ip;
	int ret;
	t_queue_msg que_msg;
	while(1)
	{
		ret = xQueueReceive(msg_queue, &que_msg, 0);
		if(ret == pdPASS)
		{
			if(que_msg.msg_flag == SET_TCP_SOCKET)
			{
				log_printf(LOG_DEBUG"[%s]tcp socket create\n",__FUNCTION__);
				ret = tcp_socket_set(HTTP_SERVER, HTTP_PORT,0);
				if(ret >= 0)
				{
					ret = cattsoft_device_register_request(socket_fd);
					if(ret == 1)
					{
						set_http_status(DEVICE_REGISTER_RES);
					}
				}
			}

			else if(que_msg.msg_flag == DATA_FROM_NET)
			{
				log_printf(LOG_DEBUG"[%s]data from net\n",__FUNCTION__);
				if(http_status != DEVICE_CONFIG_FINISH)
				{
					response_code = cattsoft_get_resonse_code(que_msg.data);
					switch(http_status)
					{
						case DEVICE_REGISTER_RES:
							ret = cattsoft_http_registerion(que_msg.data, response_code);
							if(ret == 1)
							{
								ret = cattsoft_device_prelogin_request(socket_fd);
								if(ret == 1)
								{
									set_http_status(DEVICE_PRE_LOGIN_RES);
								}
							}
							rlt_queue_free_msg(que_msg);
							break;

						case DEVICE_PRE_LOGIN_RES:
							ret = cattsoft_http_prelogin(que_msg.data, m2m_server, &m2m_port, response_code);
							if(ret == 1)
							{
								set_http_status(DEVICE_CONFIG_FINISH);
								log_printf(LOG_DEBUG"[%s]m2m server:%s    m2m port:%d\n",__FUNCTION__,m2m_server,m2m_port);

							}
							rlt_queue_free_msg(que_msg);
							break;

						default:
							break;
					}
				}
				else
				{

				}
			}
		}	
		vTaskDelay(300);	
	}
}


int rlk_tcp_send_entry()
{
	xTaskHandle app_task_handle = NULL;

	if(xTaskCreate((TaskFunction_t)rlk_tcp_send_func, (char const *)"rlk tcp send task", 1024*3, NULL, tskIDLE_PRIORITY + 5, &app_task_handle) != pdPASS) {
		printf("xTaskCreate failed\n");	
	}
	return 0;
}