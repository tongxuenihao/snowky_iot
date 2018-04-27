/*************************************************************************
	> File Name: rlt_queue_func.c
	> Author: xuetong
	> Mail: 
	> Created Time: Fri 27 Apr 2018 04:28:15 PM CST
 ************************************************************************/

typedef struct que_msg{
	unsigned char *data;
	unsigned int data_len;
	unsigned int msg_flag;
}t_queue_msg;

enum{
	TCP_CREAT = 0,
	DATA_FROM_UART,
	DATA_FROM_NET,
};

int rlt_msg_queue_create(xQueueHandle *mp_que,int msg_num);
int rlt_msg_queue_send(xQueueHandle que,int msg_flag,unsigned char *data,unsigned int data_len);
void rlt_queue_free_msg(t_queue_msg *ptr);

