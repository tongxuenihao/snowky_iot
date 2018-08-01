/*************************************************************************
	> File Name: rlt_queue_func.h
	> Author: xuetong
	> Mail: 
	> Created Time: Fri 27 Apr 2018 04:28:15 PM CST
 ************************************************************************/
#ifndef _RLT_QUEUE_FUNC_H
#define _RLT_QUEUE_FUNC_H

typedef struct que_msg{
	unsigned char *data;
	unsigned int data_len;
	unsigned int msg_flag;
}t_queue_msg;

enum{
	SET_TCP_SOCKET = 0,
	DATA_FROM_UART,
	DATA_FROM_NET,
	DEVICE_LOGOUT,
};

int rlt_msg_queue_create(xQueueHandle *mp_que,int msg_num);
int rlt_msg_queue_send(xQueueHandle que,int msg_flag,unsigned char *data,unsigned int data_len);
void rlt_queue_free_msg(t_queue_msg ptr);

#endif
