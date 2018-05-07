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

#define CLOUD_MQTT_SET_ALIVE 120

enum{
	DEVICE_REGISTER_RES = 0,
	DEVICE_LOGOUT_RES,
	DEVICE_PRE_LOGIN_RES,
	DEVICE_CONFIG_FINISH,

	M2M_LOGIN_RES,
	M2M_SUBSCRIPT_TOPIC1_RES,
	M2M_SUBSCRIPT_TOPIC2_RES,
	M2M_RUNNING
};

#define HTTP_SERVER "192.168.1.1"
//#define HTTP_SERVER "api.gizwits.com"
#define HTTP_PORT 8080



#endif