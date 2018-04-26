/*************************************************************************
	> File Name: snowky_uart_cmd_handle.h
	> Author: xuetong
	> Mail: 
	> Created Time: Tue 24 Apr 2018 11:49:07 AM CST
 ************************************************************************/

#ifndef _SNOWKY_UART_CMD_HANDLE_H
#define _SNOWKY_UART_CMD_HANDLE_H

void uart_cmd_0x06_handle();
void uart_cmd_0x07_handle(unsigned char *data, unsigned int data_len);
void uart_cmd_0x08_handle();

#endif
