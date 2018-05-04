/*************************************************************************
	> File Name: rlt_flash_parameter.h
	> Author: xuetong
	> Mail: 
	> Created Time: Tue 24 Apr 2018 14:03:28 PM CST
 ************************************************************************/
#ifndef _RLT_FLASH_PARAMETER_H
#define _RLT_FLASH_PARAMETER_H

#include "snowky_uart_protocol.h"

#define CONFIG_DATA_ADDRESS		((0x100000) - (0x1000))
#define CONFIG_DEVICE_INFO		((0x100000) - (0x2000))

#define DID_LEN 12
#define TOKEN_LEN 64
#define ACCESS_KEY 32

typedef struct dev_info{
	unsigned char dev_sn[29];
	unsigned char dev_ver[10];
	unsigned char did[13];
	unsigned char access_token[65];
	unsigned char access_key[33];
}t_dev_info;

int rlt_config_write(const char *buffer, int length);
int rlt_config_read(char *buffer, int length);


#endif