/*************************************************************************
	> File Name: cattsoft.h
	> Author: xuetong
	> Mail: 
	> Created Time: Sat 28 Apr 2018 10:47:29 PM CST



 ************************************************************************/
#ifndef _CATTSOFT_H
#define _CATTSOFT_H

#define kCRLFNewLine "\r\n" 
#define kCRLFLineEnding "\r\n\r\n"

#define NET_INFO_POS 5

#define CLOUD_MQTT_SET_ALIVE 60

enum{
	DEVICE_REGISTER_RES = 0,
	DEVICE_LOGOUT_RES,
	DEVICE_PRE_LOGIN_RES,
	DEVICE_CONFIG_FINISH,

	M2M_LOGIN_RES,
	M2M_SUBSCRIPT_TOPIC1_RES,
	M2M_SUBSCRIPT_TOPIC2_RES,
	M2M_SUBSCRIPT_TOPIC3_RES,
	M2M_RUNNING
};

#define HTTP_SERVER "117.71.58.103"
#define SN_HTTP_SERVER "172.16.1.8"
#define HTTP_PORT 8087
//#define HTTP_SERVER_CONNECT "118.178.132.84"
//#define HTTP_PORT 80

#define M2M_SERVER "172.16.1.9"
#define M2M_PORT 1883

//#define M2M_SERVER "111.148.9.98"
//#define M2M_PORT 1883



#endif