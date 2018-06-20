/*************************************************************************
	> File Name: rlt_net_data_process.c
	> Author: xuetong
	> Mail: 
	> Created Time: Tue 26 Apr 2018 09:50:28 AM CST
 ************************************************************************/
#include "common.h"

unsigned char ssid_info[512] = {0};
unsigned int offset = 0;

static unsigned short net2uart_sn = 0;

unsigned short get_new_packet_sn()
{
	static unsigned short temp_sn = 1;
	if(temp_sn < 32767)
	{
		temp_sn++;
	}
	else
	{
		temp_sn = 1;
	}
	return temp_sn;
}

unsigned short CRC_Calculate(unsigned int Length, unsigned char *buf)
{
	unsigned int i;
	unsigned int j;
	unsigned int CRC;

	CRC=0XFFFF;
	for(i=0;i<Length;i++)
		{
			CRC^=*buf++;
			for(j=0;j<8;j++)
				{
					if(CRC&0X01)
						{
							CRC=(CRC>>1)^0X8408;
						}
					else
						{
							CRC>>=0X01;
						}
				}
		}
	return (unsigned short)(~CRC);
}


void net_packet_build(unsigned short msg_type, unsigned short packet_sn, unsigned char *msg_body, unsigned short msgbody_len, unsigned char *packet)
{
	packet[0] = NET_SYNC;
	packet[1] = NET_SYNC;
	packet[2] = NET_PACKET_VER;
	packet[3] = NET_ENCRYPE_SIGN;
	*(unsigned short *)&packet[4] = htons(msgbody_len + NET_PACKET_HEAD);
	*(unsigned short *)&packet[6] = htons(msg_type);
	*(unsigned short *)&packet[8] = htons(packet_sn);
	//print_hex(packet,sizeof(packet));
	//memcpy(&data[16], did, DID_LEN);            //need to add did
	memcpy(&packet[24], msg_body, msgbody_len);
	*(unsigned short *)&packet[msgbody_len + NET_PACKET_HEAD -2] = htons(CRC_Calculate(msgbody_len + 22, &packet[2]));
}


void set_wifi_info_fill_in(unsigned char ssid_lenth, rtw_security_t security, unsigned char *ssid)
{
	ssid_info[offset] = ssid_lenth;
	offset++;
	if(security == RTW_SECURITY_OPEN)
	{
		ssid_info[offset] = 0;
		offset++;
	}
	else if(security == RTW_SECURITY_WEP_PSK)
	{
		ssid_info[offset] = 1;
		offset++;
	}
	else
	{
		ssid_info[offset] = 2;
		offset++;
	}
	memcpy(&ssid_info[offset], ssid, ssid_lenth);
	offset += ssid_lenth;
}


static rtw_result_t app_scan_result_handler( rtw_scan_handler_result_t* malloced_scan_result )
{
	static int ap_num = 0;
	if (malloced_scan_result->scan_complete != RTW_TRUE) 
	{
		rtw_scan_result_t* record = &malloced_scan_result->ap_details;
		record->SSID.val[record->SSID.len] = 0; /* Ensure the SSID is null terminated */
		if(ap_num < 20)
		{
			set_wifi_info_fill_in(strlen(record->SSID.val), record->security, record->SSID.val);
			ap_num++;
		}
	}
	return RTW_SUCCESS;
}


int wifi_info_parse(unsigned char *data)
{
	int ret;
	int ssid_len = 0;
	int passwd_len = 0;
	rtw_wifi_setting_t *wifi_info;
	wifi_info = (rtw_wifi_setting_t *)malloc(sizeof(rtw_wifi_setting_t));
	if(wifi_info == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return -1;
	}
	if(data[0] == 0)
	{
		wifi_info->security_type = RTW_SECURITY_OPEN;
	}
	else if(data[0] == 1)
	{
		wifi_info->security_type = RTW_SECURITY_WEP_PSK;
	}
	else
	{
		wifi_info->security_type = RTW_SECURITY_WPA2_AES_PSK;
	}
	ssid_len = data[1];
	if(ssid_len > 32 || ssid_len < 1)
	{
		return 2;
	}
	passwd_len = data[2];
	if(passwd_len > 64 || passwd_len < 8)
	{
		return 3;
	}
	memcpy(wifi_info->ssid, &data[3], ssid_len);
	wifi_info->ssid[ssid_len] = '\0';
	memcpy(wifi_info->password, &data[3 + ssid_len], passwd_len);
	wifi_info->password[passwd_len] = '\0';
	log_printf(LOG_DEBUG"[%s]ssid: %s           passwd:%s\n",__FUNCTION__,wifi_info->ssid, wifi_info->password);
	ret = rlt_wifi_info_write((unsigned char *)wifi_info, sizeof(rtw_wifi_setting_t));
	if(ret == 0)
	{
		return ret;
	}
	return -1;
}


void cmd_0x0061_handle(unsigned short packet_sn, unsigned int socket_fd)
{
	int ret;
	unsigned char *tdata;
	unsigned int tdata_len;
	if((ret = wifi_scan_networks(app_scan_result_handler, NULL )) != RTW_SUCCESS)
	{
		log_printf(LOG_WARNING"[%s]wifi scan error\n",__func__);
	}
	tdata_len = NET_PACKET_HEAD + offset;
	tdata = (unsigned char *)malloc(tdata_len);
	if(tdata == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return;
	}
	vTaskDelay(2000);
	memset(tdata, 0, tdata_len);
	net_packet_build(AP_LIST_RES, packet_sn, ssid_info, offset, tdata);
	log_printf(LOG_DEBUG"---->APP:\n");
	print_hex(tdata, tdata_len);
	write(socket_fd, tdata, tdata_len);
	free(tdata);
}


void cmd_0x0062_handle(unsigned char *data, unsigned short packet_sn, unsigned int socket_fd)
{
	int ret;
	unsigned char msg_body[2] = {0};
	unsigned char *tdata;
	unsigned int tdata_len;

	tdata_len = NET_PACKET_HEAD + 2;
	tdata = (unsigned char *)malloc(tdata_len);
	if(tdata == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return;
	}
	ret = wifi_info_parse(data);
	if(ret == 0)
	{
		msg_body[0] = 0;
		msg_body[1] = 0;
	}
	else 
	{
		msg_body[0] = 1;
		msg_body[1] = (char)ret;
	}
	memset(tdata, 0, tdata_len);
	net_packet_build(WIFI_INFO_ACK, packet_sn, msg_body, 2, tdata);
	log_printf(LOG_DEBUG"---->APP:\n");
	print_hex(tdata, tdata_len);
	write(socket_fd, tdata, tdata_len);
	free(tdata);
}

void cmd_0x0063_handle(unsigned char *data, unsigned short packet_sn, unsigned int socket_fd)
{
    int ret;
	unsigned char msg_body[1] = {0};
	unsigned char *tdata;
	unsigned int tdata_len;

	tdata_len = NET_PACKET_HEAD + 1;
	tdata = (unsigned char *)malloc(tdata_len);
	if(tdata == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return;
	}

	memset(tdata, 0, tdata_len);
	msg_body[0] = 0x01;
	net_packet_build(WIFI_INFO_ACK, packet_sn, msg_body, 1, tdata);
	log_printf(LOG_DEBUG"---->APP:\n");
	print_hex(tdata, tdata_len);
	write(socket_fd, tdata, tdata_len);
	free(tdata);                                            //todo
}               


void net_data_handle(unsigned short msg_type, unsigned short packet_sn, unsigned char *msg_body, unsigned int socket_fd)
{
	switch(msg_type)
	{
		case AP_LIST_REQ:
		cmd_0x0061_handle(packet_sn, socket_fd);
		break;

		case WIFI_INFO_DOWNLOAD:
		cmd_0x0062_handle(msg_body, packet_sn, socket_fd);
		break;

		case WIFI_MODE_CHANGE_REQ:
		cmd_0x0063_handle(msg_body, packet_sn, socket_fd);
		break;

		default:
		break;
	}
}


void net_data_parse(unsigned char *data, unsigned int data_len, unsigned int socket_fd)
{
	unsigned char msg_type;
	unsigned char *msg_body;
	unsigned int msgbody_len;
	unsigned short packet_sn;
	unsigned short temp_crc;
	if((data[0] != NET_SYNC) && (data[1] != NET_SYNC))
	{
		log_printf(LOG_DEBUG"[%s]net sync error\n",__FUNCTION__);
		return;
	}

	if(data[2] != NET_PACKET_VER)
	{
		log_printf(LOG_DEBUG"[%s]net ver error\n",__FUNCTION__);
		return;
	}
	msg_type = (data[6] << 8) | data[7];
	packet_sn = (data[8] << 8) | data[9];
	msgbody_len = data_len - NET_PACKET_HEAD;
	msg_body = (unsigned char *)malloc(msgbody_len);
	if(msg_body == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);         //func finished,need free
		return;
	}

	memcpy(msg_body, &data[24], msgbody_len);

	temp_crc = (data[data_len - 2] << 8) | data[data_len - 1];

	net_data_handle(msg_type, packet_sn, msg_body, socket_fd);
#if 0
	if(temp_crc = CRC_Calculate(data_len - 2, &data[2]))
	{
		net_data_handle(msg_type, packet_sn, msg_body, socket_fd);
	}

#endif
}

void set_net2uart_packet_sn(unsigned short sn)
{
	net2uart_sn = sn;
}

unsigned short get_uart2net_packet_sn()
{
	return net2uart_sn;
}


int data_from_uart_parse(unsigned char *data, unsigned char *tx_data, unsigned int data_len)
{
	unsigned int rx_data_len = 0;
	unsigned char msg_type;
	msg_type = data[9];
	switch(msg_type)
	{
		case CMD_DEVICE_CONTROL:
			net_packet_build(WIFI_TO_M2M__RES_ACK, get_uart2net_packet_sn(), data, data_len, tx_data);   
			break;

		case CMD_GET_DEVICE_STATUS:
			net_packet_build(WIFI_TO_M2M__RES_ACK, get_new_packet_sn(), data, data_len, tx_data);    
			break;

		case CMD_DEVICE_STATUS_UPLOAD:
			net_packet_build(WIFI_TO_M2M_NO_ACK, get_new_packet_sn(), data, data_len, tx_data);     
			break;

		default:
			break;
	}

	return (data_len + NET_PACKET_HEAD);
}

int data_from_m2m_parse(unsigned char *data, unsigned char *tx_data, unsigned int data_len)
{
	unsigned short *pcmd = NULL;
	unsigned short cmd;

	unsigned short *psn = NULL;
	unsigned short sn;

	unsigned int tx_data_len;
	tx_data_len = data_len - NET_PACKET_HEAD;
	pcmd = (unsigned short*)&data[6];
    cmd = ntohs(*pcmd);
    if(cmd == 0x0021)
    {
		psn = (unsigned short*)&data[8];
	    sn = ntohs(*psn);
	    set_net2uart_packet_sn(sn);
	    memcpy(tx_data, &data[24], tx_data_len);
    }

    else if(cmd == 0x8022)
    {
	    memcpy(tx_data, &data[24], tx_data_len);
    }
    return tx_data_len;
}
