/*************************************************************************
	> File Name: rlt_queue_func.c
	> Author: xuetong
	> Mail: 
	> Created Time: Fri 27 Apr 2018 04:28:09 PM CST
 ************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "device_lock.h"
#include "flash_api.h"
#include "platform_opts.h"
#include "semphr.h"

#include "log_level_print.h"
#include "rlt_queue_func.h"


xSemaphoreHandle mutex_msg_queue;

#define MSG_QUEUE_SEM_LOCK	        xSemaphoreTake(mutex_msg_queue,portMAX_DELAY)
#define MSG_QUEUE_SEM_UNLOCK	        xSemaphoreGive(mutex_msg_queue)

int rlt_msg_queue_sem_init()
{
   	mutex_msg_queue = xSemaphoreCreateMutex();
    return 1;
}


int rlt_msg_queue_create(xQueueHandle *mp_que,int msg_num)
{
    printf("create msg queue\n");
    *mp_que = xQueueCreate(msg_num,sizeof(long int));

    if (*mp_que == 0) {
            return 0;
    }
    rlt_msg_queue_sem_init();
    return 1;
}

int rlt_msg_queue_send(xQueueHandle que,int msg_flag,unsigned char *data,unsigned int data_len)
{
    unsigned int ret;
    t_queue_msg que_msg;
    MSG_QUEUE_SEM_LOCK;

    if(data == NULL)
    {
    	que_msg.msg_flag = msg_flag;
    }
    else
    {
    	que_msg.msg_flag = msg_flag;
    	que_msg.data = (unsigned char *)malloc(data_len);
    	if(que_msg.data == NULL)
    	{
			log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
			return -1;
    	}
    	memcpy(que_msg.data, data, data_len);
    	que_msg.data_len = data_len;
    }

    ret = xQueueSendToBack(que,(const void *)&que_msg,0);
    if(ret == errQUEUE_FULL)
    {
    	log_printf(LOG_WARNING"[%s]queue full\n",__FUNCTION__);
        if (data != NULL) 
        {
            free(que_msg.data);
            que_msg.data = NULL;
        }
    }

    MSG_QUEUE_SEM_UNLOCK;
    return ret;
}

void rlt_queue_free_msg(t_queue_msg *ptr)
{
    if(ptr != NULL)
    {
        if(ptr->data != NULL) 
        {
        	free((void *)(ptr->data));
        	ptr->data = NULL;
    	}
    }
}
