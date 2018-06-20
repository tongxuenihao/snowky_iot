/*************************************************************************
	> File Name: rlt_net_data_process.h
	> Author: xuetong
	> Mail: 
	> Created Time: Tue 26 Apr 2018 09:51:15 AM CST
 ************************************************************************/
#ifndef RLT_NET_DATA_PROCESS__H
#define RLT_NET_DATA_PROCESS__H

enum{
	WIFI_CONNECT = 0,
	WIFI_DISCONN,
	WIFI_NO_NETWORK,
	CLOUD_CONNECT,
	CLOUD_DISCONNECT,
};

#define WIFI_CONNECT_BIT   		(1<<0)
#define WIFI_DISCONNECT_BIT 	(1<<1)
#define CLOUD_CONNECT_BIT 		(1<<2)
#define CLOUD_DISCONNECT_BIT 	(1<<3)


#define NET_SYNC 0x5A
#define NET_PACKET_VER 0x01
#define NET_ENCRYPE_SIGN 0x01

#define NET_PACKET_HEAD 26  //no msg body


#define WIFI_BROADCAST 0x0081
#define APP_BROADCAST_REQ 0x0082
#define APP_BROADCAST_RES 0x8082
#define AP_LIST_REQ 0x0061
#define AP_LIST_RES 0x8061
#define WIFI_INFO_DOWNLOAD 0x0062
#define WIFI_INFO_ACK 0x8062
#define WIFI_MODE_CHANGE_REQ 0x0063
#define WIFI_MODE_CHANGE_ACK 0x8063

#define M2M_TO_WIFI__REQ_ACK 0x0021
#define WIFI_TO_M2M__RES_ACK 0x8021

#define WIFI_TO_M2M_REQ_ACK 0x0022
#define M2M_TO_WIFI__RES_ACK 0x8022

#define M2M_TO_WIFI_NO_ACK 0x0023
#define WIFI_TO_M2M_NO_ACK 0x0024


#endif
