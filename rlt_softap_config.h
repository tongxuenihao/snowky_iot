/*************************************************************************
	> File Name: rlt_softap_config.h
	> Author: xuetong
	> Mail: 
	> Created Time: Tue 24 Apr 2018 16:50:52 AM CST
 ************************************************************************/
#ifndef _RLT_SOFTAP_CONFIG_H
#define _RLT_SOFTAP_CONFIG_H

#define PASSWD "12345678"
#define AP_SSID_LEN 32
#define AP_PASSWD_LEN 64

#define MAX_SOCKETS     10
#define SELECT_TIMEOUT  10
#define SERVER_PORT     5000
#define LISTEN_QLEN     2

typedef struct rlt_ap_setting{
	unsigned char 		ssid[AP_SSID_LEN];
	unsigned char		channel;
	rtw_security_t		security_type;
	unsigned char 		password[AP_PASSWD_LEN];
}rlt_ap_setting;

#endif