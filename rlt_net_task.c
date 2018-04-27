/*************************************************************************
	> File Name: rlt_net_task.c
	> Author: xuetong
	> Mail: 
	> Created Time: Fri 27 Apr 2018 02:09:24 PM CST
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


xQueueHandle msg_queue;
extern int sn_get_success;
extern struct netif xnetif[NET_IF_NUM]; 

int connect_wifi_config(rtw_wifi_setting_t *wifi_info)
{
	unsigned char *ip_str[16] = {0};
	unsigned char *ip;
	int ret;
	int dhcp_retry = 1;	
	int connect_retry = 0;
	
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
	rlt_msg_queue_create(msg_queue, 20);
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