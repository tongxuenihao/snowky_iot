/*************************************************************************
	> File Name: snowky_uart_task.c
	> Author:xuetong 
	> Mail: 
	> Created Time: Tue 24 Apr 2018 11:48:02 AM CST
 ************************************************************************/
/* FreeRTOS includes. */
#include "common.h"

extern xQueueHandle msg_queue;

gtimer_t cmd_07_timer;
serial_t sobj;
unsigned char recv_char_temp = 0;
unsigned char uart_rx_Ok_Flag = 0;
unsigned char uart_status = UartNOP;
unsigned int  uart_recv_len = 0;
static unsigned char recv_buffer[RECV_MAX_LEN] = {0};
unsigned int sn_get_success = 0;

unsigned char get_checksum(unsigned char *data, unsigned int len)
{
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
	int i;
	unsigned char temp_checksum = 0;
	for(i = 0; i < len; i++)
	{	
		temp_checksum += data[i];
	}
	temp_checksum = temp_checksum & 0x00ff;
	return temp_checksum;
}

unsigned int uart_packet_build(unsigned char msg_type, unsigned char *msg_body, unsigned int msg_body_len, unsigned char *packet)
{
	int i;
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
	packet[0] = SYNC;
	packet[1]= PACKET_LEN_ExCLUDE_SYNC + msg_body_len;
	packet[2] = 0;//get_device_type();
	packet[3] = packet[1]^packet[2];
	packet[MSG_TYPE_POS] = msg_type;
	memcpy(&packet[PACKET_LEN_ExCLUDE_SYNC], msg_body, msg_body_len);
	packet[PACKET_LEN_ExCLUDE_SYNC + msg_body_len] = get_checksum(&packet[1], PACKET_LEN_ExCLUDE_SYNC + msg_body_len - 1);
	return PACKET_LEN_ExCLUDE_SYNC + msg_body_len + 1;
}


int uart_data_send(unsigned char *buff, unsigned int len)
{
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
	int i;
	print_hex(buff,len);

	for(i = 0; i < len; i++)
	{
		serial_putc(&sobj, *(buff + i));
	}
	return 1;
}

void cmd_07_timer_handler()
{
	int i;
	unsigned char msgbody[1] = {0};
	unsigned char tdata[12] = {0};
	unsigned int tdata_len;
	memset(tdata, 0, 12);
	tdata_len = uart_packet_build(CMD_GET_SN, msgbody, 1, tdata);
	uart_data_send(tdata, tdata_len);
}

void cmd_07_timeout_handler(unsigned int TimeOut)
{
	if(sn_get_success)
	{
		gtimer_stop(&cmd_07_timer);
	}
	else
	{
		log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
		cmd_07_timer_handler();
	}
}

void uart_msg_handle(unsigned char *data, unsigned int data_len)
{
	int post_rawdata_flag = 0;
	unsigned char msg_type;
	msg_type = data[MSG_TYPE_POS];
	switch(msg_type)
	{
		case CMD_DEVICE_CONTROL:
			post_rawdata_flag = 1;
			break;

		case CMD_GET_DEVICE_STATUS:
			post_rawdata_flag = 1;
			break;

		case CMD_REQUEST_NET_STATUS:
			uart_cmd_0x06_handle();
			break;

		case CMD_GET_SN:
			log_printf(LOG_DEBUG"[%s]get cmd:0x07\n",__FUNCTION__);
			uart_cmd_0x07_handle(data, data_len);
			break;

		case CMD_REQUEST_WIFI_REBOOT:
			uart_cmd_0x08_handle();
			break;

		case CMD_GET_MCU_VERSION:
			uart_cmd_0x09_handle(data, data_len);
			break;

		case CMD_DEVICE_STATUS_UPLOAD:
			post_rawdata_flag = 1;
			break;

		case CMD_REQUEST_WIFI_CONFIG:
			log_printf(LOG_DEBUG"[%s]get cmd:0x11\n",__FUNCTION__);
			uart_cmd_0x0C_handle();
			break;

		default:
			break;
	}
	if(post_rawdata_flag)
	{
		rlt_msg_queue_send(msg_queue, DATA_FROM_UART, data, data_len);
	}
}

void uart_irq(uint32_t id, SerialIrq event)
{
    serial_t    *sobj = (void*)id;
	int i = 0;
	int j = 0;
    static int msg_len = 0;
	static int temp_msg_len = 0;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(event == RxIrq)
	{
        recv_char_temp = serial_getc(sobj);
        switch(uart_status)
        {
			case UartNOP:
			{
				if(uart_rx_Ok_Flag == 1)
				{
					break;
				}else{
					uart_status = UartSOP;
				}
			}
			
			case UartSOP:
			{
				if(recv_char_temp == SYNC)
				{
					recv_buffer[uart_recv_len++] = recv_char_temp;
					uart_status = UartLEN;
				}
				break;
			}

			case UartLEN:
			{
				recv_buffer[uart_recv_len++] = recv_char_temp;
				msg_len = recv_char_temp;
				uart_status = UartDeviceType;
				break;
			}

            case UartDeviceType:
            {
                recv_buffer[uart_recv_len++] = recv_char_temp;
                //set_device_type(recv_char_temp);
                uart_status = UartFrameCheck;
                break;
            }

            
            case UartFrameCheck:
            {
                recv_buffer[uart_recv_len++] = recv_char_temp;
                uart_status = UartRESV_H;
                break;
            }

			case UartRESV_H:
			{
				recv_buffer[uart_recv_len++] = recv_char_temp;
				uart_status = UartRESV_L;
				break;
			}

			case UartRESV_L:
			{
				recv_buffer[uart_recv_len++] = recv_char_temp;
				uart_status = UartMsgMark;
				break;
			}

            case UartMsgMark:
            {
                recv_buffer[uart_recv_len++] = recv_char_temp;
                uart_status = UartProVer;
                break;
            }
            
            case UartProVer:
            {
                recv_buffer[uart_recv_len++] = recv_char_temp;
                uart_status = UartDeviceVer;
                break;
            }

            case UartDeviceVer:
            {
                recv_buffer[uart_recv_len++] = recv_char_temp;
                uart_status = UartCMD;
                break;
            }
            
			case UartCMD:
			{
				recv_buffer[uart_recv_len++] = recv_char_temp;
				uart_status = UartDATA;
				break;
			}

			case UartDATA:
			{
                if(msg_len > 10)
                {
   	                recv_buffer[uart_recv_len++] = recv_char_temp;
                    msg_len--;
					if(msg_len == 10)
					{
						uart_status = UartCRC;
					}
                    break;
                }
			}

			case UartCRC:
			{
				recv_buffer[uart_recv_len++] = recv_char_temp;
				log_printf(LOG_DEBUG"UART---->:\n");
				print_hex(recv_buffer, uart_recv_len);
				uart_msg_handle(recv_buffer, uart_recv_len);
				uart_rx_Ok_Flag = 0;
                uart_recv_len = 0;
                msg_len = 0;
                uart_status = UartNOP;
                break;
#if 0
                if(recv_char_temp == get_checksum(&recv_buffer[1],msg_len - 1))
                {
                	uart_rx_Ok_Flag = 0;
                    uart_recv_len = 0;
                    msg_len = 0;
                    uart_status = UartNOP;
				}
				else 
				{
					uart_rx_Ok_Flag = 0;					
					uart_recv_len = 0;
                    msg_len = 0;
					uart_status = UartNOP;
					printf("crc error\n");
				}
#endif
			}
		}
    }
}

void rlk_uart_task(int argc, char *argv[])
{
	printf("[%s]\n",__FUNCTION__);
    //init uart
	serial_init(&sobj,UART_TX,UART_RX);
	serial_baud(&sobj,9600);
	serial_format(&sobj, 8, ParityNone, 1);
	
	serial_irq_handler(&sobj, uart_irq, (uint32_t)&sobj);
	serial_irq_set(&sobj, RxIrq, 1);
	serial_irq_set(&sobj, TxIrq, 1);

	//gtimer_init(&cmd_07_timer, TIMER2);
	//gtimer_start_periodical(&cmd_07_timer, 1000000, (void*)cmd_07_timeout_handler, NULL);
	while(1)
	{
		vTaskDelay(3000);
	}

}

int rlk_uart_task_init()
{
	xTaskHandle app_task_handle = NULL;

	if(xTaskCreate((TaskFunction_t)rlk_uart_task, (char const *)"rlk_uart_task", 1024*3, NULL, tskIDLE_PRIORITY + 5, &app_task_handle) != pdPASS) {
		printf("xTaskCreate failed\n");	
	}
	printf("start rlk uart task handle\n");
	return 0;
}

