/*************************************************************************
	> File Name: rlt_tcp_send.c
	> Author: xuetong
	> Mail: 
	> Created Time: Sat 28 Apr 2018 02:05:56 PM CST
 ************************************************************************/
#include "common.h"


extern int socket_fd;
extern xQueueHandle msg_queue;

void rlk_tcp_recv_func(int argc, char *argv[])
{
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
	fd_set read_fds;
	struct timeval timeout;
	unsigned char rx_buff[512] = {0};
	while(1) 
	{
		FD_ZERO(&read_fds);
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;

		FD_SET(socket_fd, &read_fds);

		if(select(socket_fd + 1, &read_fds, NULL, NULL, &timeout)) 
		{
			if(FD_ISSET(socket_fd, &read_fds)) 
			{
				int read_size = recv(socket_fd, rx_buff, 512, 0);
				log_printf(LOG_DEBUG"NET---->:\n");
				//printf("recv length:%d\n",read_size);
				//printf("recv:(str)\n");
				//printf("%s\n",rx_buff);
				print_hex(rx_buff,read_size);
				rlt_msg_queue_send(msg_queue, DATA_FROM_NET, rx_buff, read_size);
			}
		}
		{
			//printf("TCP recv: no data in 10 seconds\n");
		}

		vTaskDelay(300);
	}
}


int rlk_tcp_recv_entry()
{
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
	xTaskHandle app_task_handle = NULL;

	if(xTaskCreate((TaskFunction_t)rlk_tcp_recv_func, (char const *)"rlk tcp recv task", 1024*2, NULL, tskIDLE_PRIORITY + 5, &app_task_handle) != pdPASS) {
		printf("xTaskCreate failed\n");	
	}
	return 0;
}