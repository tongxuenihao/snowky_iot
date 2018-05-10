/*************************************************************************
	> File Name: rlt_net_task.c
	> Author: xuetong
	> Mail: 
	> Created Time: Fri 27 Apr 2018 02:09:24 PM CST
 ************************************************************************/
#include "common.h"


xQueueHandle msg_queue;
extern int sn_get_success;
extern struct netif xnetif[NET_IF_NUM]; 

unsigned short module_status = 0;

void set_wifi_status_bit(unsigned short wifi_status_mode, int flag);
static void wifi_no_network_cb(char* buf, int buf_len, int flags, void* userdata)
{
	log_printf(LOG_WARNING"[%s]no network\n",__FUNCTION__);
	set_wifi_connect_status(WIFI_NO_NETWORK);
}

static void wifi_connected_cb( char* buf, int buf_len, int flags, void* userdata)
{
	log_printf(LOG_DEBUG"[%s]wifi connect\n",__FUNCTION__);
	set_wifi_connect_status(WIFI_CONNECT);
	set_wifi_status_bit(WIFI_CONNECT_BIT, 1);
}

static void wifi_disconn_cb( char* buf, int buf_len, int flags, void* userdata)
{
	log_printf(LOG_WARNING"[%s]wifi disconnect\n",__FUNCTION__);
	set_wifi_connect_status(WIFI_DISCONN);
	set_wifi_status_bit(WIFI_CONNECT_BIT, 0);
}

void set_wifi_status_bit(unsigned short wifi_status_mode, int flag)
{
	if(flag == 1)
	{
		module_status |= wifi_status_mode;
	}
	else
	{
		module_status &=~ wifi_status_mode;
	}
}

int wifi_status_compare()
{
	static unsigned short temp_wifi_status;
	if(temp_wifi_status != module_status)
	{
		temp_wifi_status = module_status;
		return 1;
	}
	return -1;
}

void wifi_staus_print()
{
	printf("\r\n");
	if(module_status & WIFI_CONNECT_BIT)
	{
		printf(LOG_DEBUG"wifi status:  connect\n");
	}
	else
	{
		printf(LOG_WARNING"wifi status:  disconnect\n");
	}
	if(module_status & CLOUD_CONNECT_BIT)
	{
		printf(LOG_DEBUG"cloud status: connect\n");
	}
	else
	{
		printf(LOG_WARNING"cloud status: disconnect\n");
	}
}


int connect_wifi_config(rtw_wifi_setting_t *wifi_info)
{
	unsigned char *ip_str[16] = {0};
	unsigned char *ip;
	int ret;
	int dhcp_retry = 1;	
	int connect_retry = 0;
	
	wifi_reg_event_handler(WIFI_EVENT_NO_NETWORK,wifi_no_network_cb,NULL);
	wifi_reg_event_handler(WIFI_EVENT_CONNECT, wifi_connected_cb, NULL);
	wifi_reg_event_handler(WIFI_EVENT_DISCONNECT, wifi_disconn_cb, NULL);

	wifi_disable_powersave();
	do{
		ret = wifi_connect((char*)wifi_info->ssid,
					  (rtw_security_t)wifi_info->security_type,
					  (char*)wifi_info->password,
					  (int)strlen((char const *)wifi_info->ssid),
					  (int)strlen((char const *)wifi_info->password),
					  0,
					  NULL);
		if (ret == 0) 
		{
Try_again:		
			ret = LwIP_DHCP(0, DHCP_START);	
			if(ret != DHCP_ADDRESS_ASSIGNED) 
			{
				if(dhcp_retry)
				{
					log_printf(LOG_WARNING"[%s]dhcp error,retry\n",__FUNCTION__);
					dhcp_retry--;
					goto Try_again;
				} 
				else 
				{
					log_printf(LOG_WARNING"[%s]dhcp error\n",__FUNCTION__);
					return -1;
				}
			} 
			else 
			{
				ip = LwIP_GetIP(&xnetif[0]);	
				sprintf((char *)ip_str,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
				log_printf(LOG_DEBUG"[%s]IP:%s\n",__FUNCTION__,ip_str);
				return 0;
			}
		} 
		else 
		{
			log_printf(LOG_WARNING"[%s]connect fail\n",__FUNCTION__);
		}	
	}while(wifi_is_ready_to_transceive(RTW_STA_INTERFACE) != RTW_SUCCESS);
	return 0;
}

void net_event_init()
{
	rlt_msg_queue_create(&msg_queue, 20);
	rlt_msg_queue_send(msg_queue, SET_TCP_SOCKET, NULL, 0);
	rlk_tcp_send_entry();
	rlk_tcp_recv_entry();
}

void rlk_net_task(int argc, char *argv[])
{
	int ret;
	rtw_wifi_setting_t *wifi_info;
#if 0
	while(sn_get_success != 1)
	{
		vTaskDelay(1000);
	}
#endif
	wifi_info = (rtw_wifi_setting_t *)malloc(sizeof(rtw_wifi_setting_t));
	if(wifi_info == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return;
	}
	memset((unsigned char *)wifi_info, 0, sizeof(rtw_wifi_setting_t));
	ret = rlt_wifi_info_read((unsigned char *)wifi_info, sizeof(rtw_wifi_setting_t));
	if(ret == 0)
	{
		if(*((uint32_t *) wifi_info) != ~0x0 && strlen(wifi_info->ssid))
		{
			goto connect_ap;
		}
		else 
		{
			goto null_model;
		}
	}

connect_ap:
	ret = connect_wifi_config(wifi_info);
	if(ret == 0)
	{
		net_event_init();
	}
	goto null_model;

null_model:
	while(1)
	{
		//if(wifi_status_compare())
		//{
		wifi_staus_print();
		//}
		vTaskDelay(3000);
	}
}

int rlk_net_task_init()
{
	xTaskHandle app_task_handle = NULL;

	if(xTaskCreate((TaskFunction_t)rlk_net_task, (char const *)"rlk net task", 1024*3, NULL, tskIDLE_PRIORITY + 5, &app_task_handle) != pdPASS) {
		printf("xTaskCreate failed\n");	
	}
	return 0;
}