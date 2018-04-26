/*************************************************************************
	> File Name: snowky_uart_cmd_handle.c
	> Author: xuetong
	> Mail: 
	> Created Time: Tue 24 Apr 2018 11:48:57 AM CST
 ************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "wifi_conf.h"

#include "snowky_uart_protocol.h"
#include "data_type_def.h"
#include "log_level_print.h"
#include "snowky_uart_task.h"
#include "rlt_flash_parameter.h"
#include "rlt_softap_config.h"

extern unsigned int sn_get_success;
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
	memcpy(dev_info->dev_sn, &data[10], DEV_SN_LEN - 1);
	dev_info->dev_sn[DEV_SN_LEN - 1] = '\0';
	rlt_config_write((unsigned char *)dev_info, sizeof(t_dev_info));
	free(dev_info);
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
	memcpy(dev_info->dev_ver, &data[10], DEV_VER_LEN - 1);
	dev_info->dev_sn[DEV_SN_LEN - 1] = '\0';
	rlt_config_write((unsigned char *)dev_info, sizeof(t_dev_info));
	free(dev_info);
}

void uart_cmd_0x0C_handle()
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
	msg_body[0] = 1;
	memset(tdata, 0, tdata_len);
	tdata_len = uart_packet_build(CMD_REQUEST_WIFI_CONFIG, msg_body, 3, tdata);
	uart_data_send(tdata, tdata_len);
	log_printf(LOG_DEBUG"[%s]----------------->enter wifi config\n",__FUNCTION__);
	rlt_softap_config_entry();
	free(tdata);	
}

