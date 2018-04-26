/*************************************************************************
	> File Name: rlt_main_task.c
	> Author: xuetong
	> Mail: 
	> Created Time: Tue 24 Apr 2018 11:43:41 AM CST
 ************************************************************************/
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "freertos_pmu.h"
#include "queue.h"
#include <stdlib.h>
#include "timers.h"
#include "serial_api.h"
#include "timer_api.h"

#include "snowky_uart_task.h"
#include "log_level_print.h"

void ilife_watchdog_irq_handler(uint32_t id)
{
    printf("!!!!!!watchdog barks!!!!!!\r\n");
	vTaskDelay(800);
	sys_reset();
}


void snowky_example_thread(int argc, char *argv[])
{
	int ret;
	set_log_printf_level(3);
	log_printf(LOG_DEBUG"[%s]system init\n",__func__);

	//watchdog_init(10000);	
	//watchdog_irq_init(ilife_watchdog_irq_handler, 0);
	//watchdog_start();
	//watchdog_refresh();
	
	rlk_uart_task_init();

	while(1)
	{
		printf("Available heap 0x%x\n",xPortGetFreeHeapSize());	
		vTaskDelay(3000);
	}
}

typedef int (*init_done_ptr)(void);
extern init_done_ptr p_wlan_init_done_callback;

int start_snowky()
{
	xTaskHandle app_task_handle = NULL;

	p_wlan_init_done_callback = NULL;
	if(xTaskCreate((TaskFunction_t)snowky_example_thread, (char const *)"snowky_entry", 1024*3, NULL, tskIDLE_PRIORITY + 5, &app_task_handle) != pdPASS) {
		printf("xTaskCreate failed\n");	
	}
	printf("start snowky task handle\n");
	return 0;
}

void example_snowky(void)
{
	//have to be called after wlan init done.
	p_wlan_init_done_callback = start_snowky;
}

