/*************************************************************************
	> File Name: snowky_uart_protocol.h
	> Author: xuetong
	> Mail: 
	> Created Time: Tue 24 Apr 2018 11:47:08 AM CST
 ************************************************************************/

#ifndef _SNOWKY_UART_PROTOCOL_H
#define _SNOWKY_UART_PROTOCOL_H

#define UART_TX    PA_7
#define UART_RX    PA_6
#define RECV_MAX_LEN 80
#define DEV_SN_LEN 28
#define DEV_VER_LEN 9

#define SYNC 0xF5
#define PACKET_LEN_ExCLUDE_SYNC 10
#define PACKET_LEN_INCLUDE_SYNC 11
#define MSG_TYPE_POS 9

#define CMD_DEVICE_CONTROL 0x02
#define CMD_GET_DEVICE_STATUS 0x03
#define CMD_UPGRADE_ONLINE 0x04
#define CMD_REQUEST_NET_STATUS 0x06
#define CMD_GET_SN 0x07
#define CMD_REQUEST_WIFI_REBOOT 0x08
#define CMD_GET_MCU_VERSION 0x09
#define CMD_NET_STATUS_DOWNLOAD 0x0A
#define CMD_DEVICE_STATUS_UPLOAD 0x0B
#define CMD_REQUEST_WIFI_CONFIG 0x0C
#define CMD_REQUEST_SN_WRITE 0x11

#endif
