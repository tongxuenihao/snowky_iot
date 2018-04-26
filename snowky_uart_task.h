/*************************************************************************
	> File Name: snowky_uart_task.h
	> Author: xuetong
	> Mail: 
	> Created Time: Tue 24 Apr 2018 11:48:23 AM CST
 ************************************************************************/

#ifndef _SNOWKY_UART_TASK_H
#define _SNOWKY_UART_TASK_H


#define UartNOP  	(0) 
#define UartSOP  	(1)        
#define UartLEN  	(2)        
#define UartDeviceType (3)
#define UartFrameCheck (4)
#define UartRESV_H	(5)			 
#define UartRESV_L	(6)			
#define UartMsgMark (7)
#define UartProVer  (8)
#define UartDeviceVer (9)
#define UartCMD  	(10)         
#define UartDATA 	(11)      
#define UartCRC  	(12)       
int rlk_uart_task_init();
unsigned int uart_packet_build(unsigned char msg_type, unsigned char *msg_body, unsigned int msg_body_len, unsigned char *packet);
int uart_data_send(unsigned char *buff, unsigned int len);

#endif
