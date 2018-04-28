/*************************************************************************
	> File Name: rlt_tcp_send.c
	> Author: xuetong
	> Mail: 
	> Created Time: Sat 28 Apr 2018 09:40:15 AM CST
 ************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdlib.h>
#include "timers.h"
#include "serial_api.h"
#include "timer_api.h"
#include "wifi_structures.h"
#include "wifi_conf.h"
#include "lwip_netconf.h"
#include <platform/platform_stdlib.h>
#include <lwip/sockets.h>

#include "snowky_uart_protocol.h"
#include "data_type_def.h"
#include "log_level_print.h"
#include "snowky_uart_task.h"
#include "snowky_uart_cmd_handle.h"
#include "rlt_flash_parameter.h"
#include "rlt_queue_func.h"
#include "cattsoft_http.h"

extern xQueueHandle msg_queue;
int socket_fd = -1;

int tcp_socket_set(unsigned int server_ip, unsigned int server_port, unsigned int reconnect_flag)
{
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
	unsigned char temp_data[10] = {0x01,0x02,0x03,0x04,0x05,0x05,0x06,0x07,0x08,0x09};
	int local_fd = -1;
	struct sockaddr_in local_addr;
	struct sockaddr_in remote_addr;
	static int reconnect_times = 0;

	static int sev_ip;
	static short sev_port;

	if(reconnect_flag == 0)
	{
		sev_ip = server_ip;
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
			remote_addr.sin_addr.s_addr = inet_addr("192.168.0.6");

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
	send(socket_fd, temp_data, 10, 0);
	return socket_fd;

}

void rlk_tcp_send_func(int argc, char *argv[])
{
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
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
				tcp_socket_set(server_ip, HTTP_PORT,0);
			}

			else if(que_msg.msg_flag == DATA_FROM_NET)
			{
				log_printf(LOG_DEBUG"[%s]data from net\n",__FUNCTION__);
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