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

#define DID_LEN 18
#define TOKEN_LEN 32
#define ACCESS_KEY 6

typedef struct dev_info{
	unsigned char dev_sn[29];
	unsigned char dev_ver[10];
	unsigned char did[19];
	unsigned char access_token[33];
	unsigned char access_key[7];
	unsigned char dev_type;
}t_dev_info;


#endif