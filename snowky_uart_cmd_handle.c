/*************************************************************************
	> File Name: snowky_uart_cmd_handle.c
	> Author: xuetong
	> Mail: 
	> Created Time: Tue 24 Apr 2018 11:48:57 AM CST
 ************************************************************************/
#include "common.h"

extern unsigned int sn_get_success;
extern unsigned int ver_get_success;
unsigned char wifi_status = 0;
unsigned char cloud_status =0;

extern struct netif xnetif[NET_IF_NUM]; 

int set_ft_connect_wifi()
{
	unsigned char *ip_str[16] = {0};
	unsigned char *ip;
	int ret;
	int dhcp_retry = 1;	
	int connect_retry = 0;

	int ft_start;
	int ft_end;

	ft_start = xTaskGetTickCount();


	wifi_disable_powersave();
	do{
		ret = wifi_connect((char*)FT_SSID,
					  RTW_SECURITY_WPA2_AES_PSK,
					  (char*)FT_PASSWORD,
					  (int)strlen((char const *)FT_SSID),
					  (int)strlen((char const *)FT_PASSWORD),
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
		ft_end = xTaskGetTickCount();
		if(ft_end - ft_start >= FT_TIMEOUT_CONFIG)
		{
			return -1;
		}
	}while(wifi_is_ready_to_transceive(RTW_STA_INTERFACE) != RTW_SUCCESS);
	return 0;
}


unsigned char get_wifi_connect_status()
{
	return wifi_status;
}

void set_wifi_connect_status(unsigned char flag)
{
	wifi_status = flag;
}

unsigned char get_cloud_connect_status()
{
	return cloud_status;
}

void set_cloud_connect_status(unsigned char flag)
{
	cloud_status = flag;
}

void ft_res_confirm(int flag)
{
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
	unsigned char msg_body[4] = {0};
	unsigned char *tdata;
	unsigned int tdata_len = PACKET_LEN_INCLUDE_SYNC + 4;
	tdata = (unsigned char *)malloc(tdata_len);
	if(tdata == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return;
	}

	if(flag == 0)
	{
		msg_body[0] = get_wifi_connect_status();
		msg_body[1] = wifi_get_signal_level();
		msg_body[2] = get_cloud_connect_status();
		msg_body[3] = 0x02;
		memset(tdata, 0, tdata_len);
		tdata_len = uart_packet_build(CMD_REQUEST_NET_STATUS, msg_body, 4, tdata);
		uart_data_send(tdata, tdata_len);
		free(tdata);
		return;
	}
	else if(flag == 1)
	{
		msg_body[0] = get_wifi_connect_status();
		msg_body[1] = wifi_get_signal_level();
		msg_body[2] = get_cloud_connect_status();
		msg_body[3] = 0x01;
		memset(tdata, 0, tdata_len);
		tdata_len = uart_packet_build(CMD_REQUEST_NET_STATUS, msg_body, 4, tdata);
		uart_data_send(tdata, tdata_len);
		free(tdata);
		return;
	}
}

void set_ft_mode_init()
{
	int ret;
	wifi_disconnect();
	ret = set_ft_connect_wifi();
	if(ret == 0)
	{
		ft_res_confirm(1);
	}
	
	else if(ret == -1)
	{
		ft_res_confirm(0);
	}
}



void rlk_ft_task_func()
{
	set_ft_mode_init();
	while(1)
	{
		vTaskDelay(300);
	}
}

int device_enter_ft_mode()
{
	xTaskHandle app_task_handle = NULL;

	if(xTaskCreate((TaskFunction_t)rlk_ft_task_func, (char const *)"rlk_uart_task", 1024*3, NULL, tskIDLE_PRIORITY + 5, &app_task_handle) != pdPASS) {
		printf("xTaskCreate failed\n");	
	}
	printf("start rlk ft task handle\n");
	return 0;
}


void device_exit_ft_mode()
{
	//wifi_off();
	t_dev_info *dev_info;
	dev_info = (t_dev_info *)malloc(sizeof(t_dev_info));
	if(dev_info == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return;
	}
	memset((unsigned char *)dev_info, 0, sizeof(t_dev_info));
	rlt_device_info_read((unsigned char *)dev_info, sizeof(t_dev_info));
	dev_info->ft_flag = 0;
	rlt_device_info_write((unsigned char *)dev_info, sizeof(t_dev_info));
	sys_reset();
}

unsigned char wifi_get_signal_level()
{
	int wifi_signal;
	unsigned char rssi_value;
	wifi_get_rssi(&wifi_signal);
	rssi_value = abs(wifi_signal);
	printf("%d\n", rssi_value);
	if(rssi_value == 0)
	{
		return 0;
	}
	else if((rssi_value > 0) && (rssi_value < 20))
	{
		return 4;
	}
	else if((rssi_value > 20) && (rssi_value < 30))
	{
		return 3;
	}
	else if((rssi_value > 30) && (rssi_value < 40))
	{
		return 2;
	}
	else 
	{
		return 1;
	}
}

void device_version_get()
{
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
	unsigned char msg_body[1] = {0};
	unsigned char *tdata;
	unsigned int tdata_len = PACKET_LEN_INCLUDE_SYNC + 1;
	tdata = (unsigned char *)malloc(tdata_len);
	if(tdata == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return;
	}
	msg_body[0] = 0;
	memset(tdata, 0, tdata_len);
	tdata_len = uart_packet_build(CMD_GET_MCU_VERSION, msg_body, 1, tdata);
	uart_data_send(tdata, tdata_len);
	free(tdata);
}


void uart_cmd_0x06_handle(unsigned char *data)
{
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
	t_dev_info *dev_info;
	unsigned char msg_body[4] = {0};
	unsigned char *tdata;
	unsigned int tdata_len = PACKET_LEN_INCLUDE_SYNC + 4;
	tdata = (unsigned char *)malloc(tdata_len);
	if(tdata == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return;
	}

	 
	dev_info = (t_dev_info *)malloc(sizeof(t_dev_info));
	if(dev_info == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return;
	}
	memset((unsigned char *)dev_info, 0, sizeof(t_dev_info));
	rlt_device_info_read((unsigned char *)dev_info, sizeof(t_dev_info));

	if(data[10] == 0)
	{
		if(dev_info->ft_flag == 0x01)
		{
			device_exit_ft_mode();
		}
		msg_body[0] = get_wifi_connect_status();
		msg_body[1] = wifi_get_signal_level();
		msg_body[2] = get_cloud_connect_status();
		memset(tdata, 0, tdata_len);
		tdata_len = uart_packet_build(CMD_REQUEST_NET_STATUS, msg_body, 4, tdata);
		uart_data_send(tdata, tdata_len);
		free(tdata);
	}
	else if(data[10] == 0x01)
	{
		dev_info->ft_flag = 0x01;
		rlt_device_info_write((unsigned char *)dev_info, sizeof(t_dev_info));
		sys_reset();
	}

}


void uart_cmd_0x07_handle(unsigned char *data, unsigned int data_len)
{
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
	t_dev_info *dev_info;
	sn_get_success = 1;
		 
	dev_info = (t_dev_info *)malloc(sizeof(t_dev_info));
	if(dev_info == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return;
	}
	memset((unsigned char *)dev_info, 0, sizeof(t_dev_info));
	rlt_device_info_read((unsigned char *)dev_info, sizeof(t_dev_info));
	memcpy(dev_info->dev_sn, &data[10], DEV_SN_LEN);
	dev_info->dev_sn[DEV_SN_LEN] = '\0';
	dev_info->dev_type = data[2];
	rlt_device_info_write((unsigned char *)dev_info, sizeof(t_dev_info));
	free(dev_info);

	//device_version_get();
}

void uart_cmd_0x08_handle()
{	
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
	unsigned char msg_body[3] = {0};
	unsigned char *tdata;
	unsigned int tdata_len = PACKET_LEN_INCLUDE_SYNC + 3;
	tdata = (unsigned char *)malloc(tdata_len);
	if(tdata == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return;
	}
	msg_body[0] = 0;
	memset(tdata, 0, tdata_len);
	tdata_len = uart_packet_build(CMD_REQUEST_WIFI_REBOOT, msg_body, 3, tdata);
	uart_data_send(tdata, tdata_len);
	log_printf(LOG_DEBUG"[%s]sys reboot\n",__FUNCTION__);
	sys_reset();
	free(tdata);	
}

void uart_cmd_0x09_handle(unsigned char *data, unsigned int data_len)
{
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
	t_dev_info *dev_info;
	dev_info = (t_dev_info *)malloc(sizeof(t_dev_info));
	if(dev_info == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return;
	}
	memset((unsigned char *)dev_info, 0, sizeof(t_dev_info));
	rlt_device_info_read((unsigned char *)dev_info, sizeof(t_dev_info));
	memcpy(dev_info->dev_ver, &data[10], DEV_VER_LEN);
	dev_info->dev_ver[DEV_VER_LEN] = '\0';
	rlt_device_info_write((unsigned char *)dev_info, sizeof(t_dev_info));
	ver_get_success = 1;

	if(dev_info->ft_flag == 1)
	{
		rlt_device_info_write((unsigned char *)dev_info, sizeof(t_dev_info));
		log_printf(LOG_DEBUG"[%s]----------------->enter ft mode\n",__FUNCTION__);
		device_enter_ft_mode();
		return;
	}

	if(dev_info->alink_reset_flag == 0x01)
	{
		rlt_device_info_clean();
		rlt_wifi_info_clean();
		dev_info->alink_reset_flag = 0;
		rlt_device_info_write((unsigned char *)dev_info, sizeof(t_dev_info));
		log_printf(LOG_DEBUG"[%s]----------------->enter wifi config\n",__FUNCTION__);
		rlt_softap_config_entry();
	}

	free(dev_info);
}


extern unsigned short module_status;
extern xQueueHandle msg_queue;
void uart_cmd_0x0C_handle()
{	
	int ret;
	unsigned char msg_body[3] = {0};
	unsigned char *tdata;
	unsigned int tdata_len = PACKET_LEN_INCLUDE_SYNC + 3;

	rtw_wifi_setting_t *wifi_info;
	t_dev_info *dev_info;
	
	dev_info = (t_dev_info *)malloc(sizeof(t_dev_info));
	if(dev_info == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return;
	}
	memset((unsigned char *)dev_info, 0, sizeof(t_dev_info));
	rlt_device_info_read((unsigned char *)dev_info, sizeof(t_dev_info));

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
		//if(*((uint32_t *) wifi_info) != ~0x0 && strlen(wifi_info->ssid))
		//{
		if(module_status & CLOUD_CONNECT_BIT)
		{
			rlt_msg_queue_send(msg_queue, DEVICE_LOGOUT, NULL, 0);
			return;
		}
		else
		{                            
			rlt_device_info_clean();
			rlt_wifi_info_clean();
			dev_info->alink_reset_flag = 0x01;
			rlt_device_info_write((unsigned char *)dev_info, sizeof(t_dev_info));
			sys_reset();  
		}
		//}

		if(dev_info->alink_reset_flag == 0x01)
		{
			sys_reset(); 
		}
		
		tdata = (unsigned char *)malloc(tdata_len);
		if(tdata == NULL)
		{
			log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
			return;
		}
		msg_body[0] = 1;
		memset(tdata, 0, tdata_len);
		tdata_len = uart_packet_build(CMD_REQUEST_WIFI_CONFIG, msg_body, 3, tdata);
		uart_data_send(tdata, tdata_len);
		rlt_wifi_info_clean();
		log_printf(LOG_DEBUG"[%s]----------------->enter wifi config\n",__FUNCTION__);
		rlt_softap_config_entry();
		free(tdata);
	}	
}

