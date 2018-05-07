/*************************************************************************
	> File Name: common.h
	> Author: xuetong
	> Mail: 
	> Created Time: Thu 03 May 2018 03:46:51 PM CST
 ************************************************************************/

#ifndef _COMMON_H
#define _COMMON_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdlib.h>
#include "timers.h"
#include "serial_api.h"
#include "timer_api.h"
#include "wifi_structures.h"
#include "wifi_conf.h"
#include "lwip_netconf.h"
#include "platform/platform_stdlib.h"
#include "lwip/sockets.h"
#include "netdb.h"
#include <platform/platform_stdlib.h>
#include "main.h"


#include "snowky_uart_protocol.h"
#include "rlt_net_data_process.h"
#include "data_type_def.h"
#include "log_level_print.h"
#include "snowky_uart_task.h"
#include "snowky_uart_cmd_handle.h"
#include "rlt_flash_parameter.h"
#include "rlt_queue_func.h"
#include "cattsoft.h"
#include "cJSON.h"
#include "rlt_softap_config.h"
#include "rlt_net_data_process.h"
#include "cattsoft_mqtt.h"

#endif