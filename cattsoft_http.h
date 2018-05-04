/*************************************************************************
	> File Name: cattsoft_http.h
	> Author: xuetong
	> Mail: 
	> Created Time: Sat 28 Apr 2018 10:47:29 PM CST
 ************************************************************************/
#ifndef _CATTSOFT_HTTP_H
#define _CATTSOFT_HTTP_H

#define kCRLFNewLine "\r\n" 
#define kCRLFLineEnding "\r\n\r\n"

enum{
	DEVICE_REGISTER_REQ = 0,
	DEVICE_REGISTER_RES,
	DEVICE_LOGOUT_REQ,
	DEVICE_LOGOUT_RES,
	DEVICE_PRE_LOGIN_REQ,
	DEVICE_PRE_LOGIN_RES,
	DEVICE_CONFIG_FINISH,
};

#define HTTP_SERVER "192.168.1.1"
//#define HTTP_SERVER "api.gizwits.com"
#define HTTP_PORT 8080



#endif