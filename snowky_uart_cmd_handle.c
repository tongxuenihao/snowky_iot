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


void uart_cmd_0x06_handle()
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
	msg_body[0] = get_wifi_connect_status();
	msg_body[1] = wifi_get_signal_level();
	msg_body[2] = get_cloud_connect_status();
	memset(tdata, 0, tdata_len);
	tdata_len = uart_packet_build(CMD_REQUEST_NET_STATUS, msg_body, 3, tdata);
	uart_data_send(tdata, tdata_len);
	free(tdata);
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
		if(*((uint32_t *) wifi_info) != ~0x0 && strlen(wifi_info->ssid))
		{
#if 1
			if(module_status & CLOUD_CONNECT_BIT)
			{
				rlt_msg_queue_send(msg_queue, DEVICE_LOGOUT, NULL, 0);
				return;
			}
			else
			{
#endif
				rlt_device_info_clean();
				rlt_wifi_info_clean();
				dev_info->alink_reset_flag = 0x01;
				rlt_device_info_write((unsigned char *)dev_info, sizeof(t_dev_info));
				sys_reset();  
			}
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

