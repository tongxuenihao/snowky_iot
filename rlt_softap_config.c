/*************************************************************************
	> File Name: rlt_softap_config.c
	> Author: xuetong
	> Mail: 
	> Created Time: Tue 24 Apr 2018 16:50:08 AM CST
 ************************************************************************/
#include "common.h"


#define BCAST_PORT 28899
int recv_0x0082_flag = 0;
int udp_socket_fd = -1;

extern struct netif xnetif[NET_IF_NUM]; 

int udp_socket_init(unsigned int port)
{
	int socket = -1;
	int broadcast = 1;
	struct sockaddr_in local_addr;
	
	if((socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
		log_printf(LOG_WARNING"[%s]socket create error\n",__FUNCTION__);
		return -1;
	}
	
	if(setsockopt(socket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
	{
		log_printf(LOG_WARNING"[%s]setsockopt failed\n",__FUNCTION__);
		return -1;
	}
	
	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(port);
	local_addr.sin_addr.s_addr = INADDR_ANY;
	if(bind(socket, (struct sockaddr *) &local_addr, sizeof(local_addr)) < 0)
	{
		log_printf(LOG_WARNING"[%s]bind error\n",__FUNCTION__);
		return -1;
	}
	return socket;
}


int tcp_socket_init(unsigned int port)
{
	int local_fd = -1;
	struct sockaddr_in local_addr;
	if((local_fd = socket(AF_INET, SOCK_STREAM, 0)) >= 0) 
	{
		local_addr.sin_family = AF_INET;
		local_addr.sin_port = htons(port);		
		local_addr.sin_addr.s_addr = INADDR_ANY;

		if(bind(local_fd, (struct sockaddr *) &local_addr, sizeof(local_addr)) != 0) 
		{
			log_printf(LOG_WARNING"[%s]bind error\n",__FUNCTION__);
			return -1;
		}
		return local_fd;
	}
	log_printf(LOG_WARNING"[%s]socket create error\n",__FUNCTION__);
	return -1;
}


int socket_client_accept(int server_fd)
{
	struct sockaddr_in client_addr;
	unsigned int client_addr_size = sizeof(client_addr);
	int fd;
    fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_size);
	return fd;
}


int socket_client_data_read(int client_fd)
{
	unsigned char buf[512];
	memset(buf, 0, sizeof(buf));
	int read_size = read(client_fd, buf, sizeof(buf));
	if(read_size > 0) 
	{
		log_printf(LOG_DEBUG"APP---->:\n");
		print_hex(buf, read_size);
		net_data_parse(buf, read_size, client_fd);
	}
    return read_size;
}


void rlt_tcp_server_start(int argc, char *argv[])
{
	int max_socket_fd = -1;
	int ap_start;
	int ap_end;

	struct sockaddr_in server_addr;
	int server_fd = -1;
	int socket_used[MAX_SOCKETS];

	memset(socket_used, 0, sizeof(socket_used));
	if((server_fd = tcp_socket_init(SERVER_PORT)) >= 0) 
	{
		if(listen(server_fd, LISTEN_QLEN) != 0) 
		{
			printf("listen error\n");
			goto exit;
		}
		socket_used[server_fd] = 1;
		if(server_fd > max_socket_fd)
			max_socket_fd = server_fd;
	}
	else
	{
		printf("socket error\n");
		goto exit;
	}
	ap_start = xTaskGetTickCount();
	while(1) 
	{
		int socket_fd;
		fd_set read_fds;
		struct timeval timeout;
		ap_end = xTaskGetTickCount();
		if(ap_end - ap_start > 90*1000)
		{
			sys_reset();
		}

		FD_ZERO(&read_fds);
		timeout.tv_sec = SELECT_TIMEOUT;
		timeout.tv_usec = 0;

		for(socket_fd = 0; socket_fd < MAX_SOCKETS; socket_fd ++)
			if(socket_used[socket_fd])
				FD_SET(socket_fd, &read_fds);

		if(select(max_socket_fd + 1, &read_fds, NULL, NULL, &timeout)) 
		{
			for(socket_fd = 0; socket_fd < MAX_SOCKETS; socket_fd ++) 
			{
				if(socket_used[socket_fd] && FD_ISSET(socket_fd, &read_fds)) 
				{
					if(socket_fd == server_fd) 
					{
						int fd = socket_client_accept(server_fd);
						if(fd >= 0) 
						{
							printf("accept socket fd(%d)\n", fd);
							recv_0x0082_flag = 1;
							socket_used[fd] = 1;
							if(fd > max_socket_fd)
								max_socket_fd = fd;
						}
						else 
						{
							printf("accept error\n");
						}

					}
					else 
					{
						int read_size = socket_client_data_read(socket_fd);
					}
				}
			}
		}
		else
		{
			printf("TCP server: no data in %d seconds\n", SELECT_TIMEOUT);
		}

		vTaskDelay(2000);
	}

exit:
	if(server_fd >= 0)
		close(server_fd);

	vTaskDelete(NULL);
}

int rlt_tcp_server_entry()
{
	xTaskHandle app_task_handle = NULL;

	if(xTaskCreate((TaskFunction_t)rlt_tcp_server_start, (char const *)"rlt tcp server start", 1024*2, NULL, tskIDLE_PRIORITY + 5, &app_task_handle) != pdPASS) {
		printf("xTaskCreate failed\n");	
	}
	return 0;
}

void rlt_generate_ap_name(unsigned char *ap_name);

void rlt_broadcast_recv_func()
{
	unsigned char rxbuf[128];
	struct sockaddr_in from_addr;
	int from_addr_len = sizeof(from_addr);

	while(1) 
	{
		fd_set read_fds;
		struct timeval timeout;

		FD_ZERO(&read_fds);
		FD_SET(udp_socket_fd, &read_fds);
		timeout.tv_sec = SELECT_TIMEOUT;
		timeout.tv_usec = 0;
		memset(rxbuf, 0, 128);

		if(select(udp_socket_fd + 1, &read_fds, NULL, NULL, &timeout)) 
		{
			int read_size = recvfrom(udp_socket_fd, rxbuf, 128, 0, (struct sockaddr *)&from_addr, &from_addr_len);
			if(read_size > 0)
			{
				if((rxbuf[0] == 0x5A) && (rxbuf[7] == 0x83))
				{
					recv_0x0082_flag = 1;
					return;
				}
			}
		}
		else
		{
			printf("udp recv: no data in %d seconds\n", SELECT_TIMEOUT);
		}

		vTaskDelay(300);
	}

exit:
	if(udp_socket_fd >= 0)
		close(udp_socket_fd);
	vTaskDelete(NULL);
}

int rlt_broadcast_recv_entry()
{
	xTaskHandle app_task_handle = NULL;

	if(xTaskCreate((TaskFunction_t)rlt_broadcast_recv_func, (char const *)"rlt softap config", 1024*2, NULL, tskIDLE_PRIORITY + 5, &app_task_handle) != pdPASS) {
		printf("xTaskCreate failed\n");	
	}
	return 0;
}


void rlt_broadcast_func(unsigned char *ap_name, int flag)
{
	struct sockaddr_in to;
	unsigned char *ip;
	unsigned char *mac;
	unsigned char *msg_body;
	unsigned int msgbody_len;
	unsigned char *tdata;
	unsigned int tdata_len;
	//int socket_fd;
	int send_broadcast_pk_cnt = 0;
	udp_socket_fd = udp_socket_init(BCAST_PORT);
	if(udp_socket_fd < 0)
	{
		log_printf(LOG_WARNING"[%s]udp init error\n",__FUNCTION__);
	}
	if(ap_name == NULL)
	{
		msgbody_len = 14;
	}
	else
	{
		msgbody_len = 15 + strlen(ap_name);
	}
	msg_body = (unsigned char *)malloc(msgbody_len);
	if(msg_body == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return;
	}
	ip = LwIP_GetIP(&xnetif[0]);
	memset(msg_body, 0, msgbody_len);
	memcpy(msg_body, ip, 4);                                    //little endian
	*(unsigned int *)&msg_body[4] = htonl(SERVER_PORT);
	mac = LwIP_GetMAC(&xnetif[0]);
	memcpy(&msg_body[8], mac, 6);
	if(ap_name != NULL)
	{
		msg_body[14] = strlen(ap_name);
		strcpy(&msg_body[15], ap_name);
	}

	tdata_len = NET_PACKET_HEAD + msgbody_len;
	tdata = (unsigned char *)malloc(tdata_len);
	if(tdata == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return;
	}

	to.sin_family = AF_INET;
	to.sin_port = htons(BCAST_PORT);
	to.sin_addr.s_addr = htonl(0xffffffff);
	while(1)
	{
		if((recv_0x0082_flag != 1) && (send_broadcast_pk_cnt < 15))
		{
			send_broadcast_pk_cnt++;
			printf("---->APP(%d):\n",send_broadcast_pk_cnt);             //debug,mark
			memset(tdata, 0, tdata_len);
			if(flag)
			{
				net_packet_build(WIFI_BROADCAST_DEVICE_DISCOVER, get_new_packet_sn(), msg_body, msgbody_len, tdata);

			}
			else
			{
				net_packet_build(WIFI_BROADCAST, get_new_packet_sn(), msg_body, msgbody_len, tdata);

			}
			//net_packet_build(WIFI_BROADCAST, get_new_packet_sn(), msg_body, msgbody_len, tdata);
			print_hex(tdata, tdata_len);
			sendto(udp_socket_fd, tdata, tdata_len, 0, &to, sizeof(struct sockaddr));
		}
		vTaskDelay(3000);
	}
} 

void rlt_generate_ap_name(unsigned char *ap_name)
{
	unsigned char *mac;
	mac = LwIP_GetMAC(&xnetif[0]);
	memset(ap_name, 0, AP_SSID_LEN);
	sprintf(ap_name, "SNOWKY_%02x%02x", mac[4], mac[5]);
	log_printf(LOG_DEBUG"[%s]ap name:%s\n",__FUNCTION__,ap_name);
}

void rlt_softap_start(rlt_ap_setting *ap_info)
{
	int timeout = 20;
#if CONFIG_LWIP_LAYER 
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	struct netif * pnetif = &xnetif[0];
#endif
	int ret = 0;
#if CONFIG_LWIP_LAYER
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
	dhcps_deinit();
	IP4_ADDR(&ipaddr, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
	IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
	IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
	netif_set_addr(pnetif, &ipaddr, &netmask,&gw);
#endif
	wifi_off();
	vTaskDelay(20);
	if (wifi_on(RTW_MODE_AP) < 0)
	{
		log_printf(LOG_WARNING"[%s]Wifi on failed\n",__FUNCTION__);
		ret = RTW_ERROR;
		return;
	}
	log_printf(LOG_DEBUG"[%s]Starting AP\n",__FUNCTION__);
	ret = wifi_start_ap((char*)ap_info->ssid, ap_info->security_type, NULL, strlen(ap_info->ssid), 0, ap_info->channel);
	if(ret < 0)
	{
		log_printf(LOG_WARNING"[%s]ERROR: Operation failed!\n",__FUNCTION__);
		return;
	}
	log_printf(LOG_DEBUG"[%s]OK: Operation success!\n",__FUNCTION__);
	dhcps_init(&xnetif[0]);

	rlt_tcp_server_entry();
	rlt_broadcast_func(ap_info->ssid,0);
	//rlt_broadcast_recv_entry();
	while(1)
	{
		vTaskDelay(3000);
	}
}



void rlt_softap_config_mode(int argc, char *argv[])
{
	rlt_ap_setting *ap_info;
	ap_info = (rlt_ap_setting *)malloc(sizeof(rlt_ap_setting));
	if(ap_info == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return;
	}
	rlt_generate_ap_name(ap_info->ssid);
	//strcpy(ap_info->password, PASSWD);
	//ap_info->password = "12345678";
	ap_info->security_type = RTW_SECURITY_OPEN;
	ap_info->channel = 6;

	rlt_softap_start(ap_info);
}

int rlt_softap_config_entry()
{
	xTaskHandle app_task_handle = NULL;

	if(xTaskCreate((TaskFunction_t)rlt_softap_config_mode, (char const *)"rlt softap config", 1024*2, NULL, tskIDLE_PRIORITY + 5, &app_task_handle) != pdPASS) {
		printf("xTaskCreate failed\n");	
	}
	return 0;
}



