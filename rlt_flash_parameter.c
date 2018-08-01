/*************************************************************************
	> File Name: rlt_flash_parameter.c
	> Author: xuetong
	> Mail: 
	> Created Time: Tue 24 Apr 2018 14:03:23 PM CST
 ************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "device_lock.h"
#include "flash_api.h"
#include "platform_opts.h"

#include "log_level_print.h"
#include "rlt_flash_parameter.h"


int rlt_device_info_write(const char *buffer, int length)
{
	int ret=0;
	flash_t flash_config;
	if (buffer == NULL || length < 1) 
	{
		return -1;
	}

	/*Erase flash before write*/
	device_mutex_lock(RT_DEV_LOCK_FLASH);
	flash_erase_sector(&flash_config, CONFIG_DATA_ADDRESS);
	ret = flash_stream_write(&flash_config, CONFIG_DATA_ADDRESS, length, (u8 *)buffer);
	device_mutex_unlock(RT_DEV_LOCK_FLASH);

	if(-1 == ret) 
	{
		log_printf(LOG_WARNING"[%s]write open file fail\n",__FUNCTION__);
		return -1;
	}
	log_printf(LOG_WARNING"[%s]write open file successful\n",__FUNCTION__);
	return 0;
}


int rlt_device_info_read(char *buffer, int length)
{
	int ret=0;
	flash_t flash_config;

	if (buffer== NULL || length < 1) 
	{
		return -1;
	}
	//Read flash direct
	device_mutex_lock(RT_DEV_LOCK_FLASH);
	ret = flash_stream_read(&flash_config, CONFIG_DATA_ADDRESS, length, (u8 *)buffer);
	device_mutex_unlock(RT_DEV_LOCK_FLASH);
	if(-1 == ret)
	{
		log_printf(LOG_WARNING"[%s]read open file fail\n",__FUNCTION__);
		return -1;
	}
	log_printf(LOG_WARNING"[%s]read open file successful\n",__FUNCTION__);
	return 0;
}

void rlt_device_info_clean()
{
	flash_t flash_config;
	flash_erase_sector(&flash_config, CONFIG_DATA_ADDRESS);
	return;
}

int rlt_wifi_info_write(const char *buffer, int length)
{
	int ret=0;
	flash_t flash_config;
	if (buffer == NULL || length < 1) 
	{
		return -1;
	}

	/*Erase flash before write*/
	device_mutex_lock(RT_DEV_LOCK_FLASH);
	flash_erase_sector(&flash_config, FAST_RECONNECT_DATA);
	ret = flash_stream_write(&flash_config, FAST_RECONNECT_DATA, length, (u8 *)buffer);
	device_mutex_unlock(RT_DEV_LOCK_FLASH);

	if(-1 == ret) 
	{
		log_printf(LOG_WARNING"[%s]write open file fail\n",__FUNCTION__);
		return -1;
	}
	log_printf(LOG_WARNING"[%s]write open file successful\n",__FUNCTION__);
	return 0;
}


int rlt_wifi_info_read(char *buffer, int length)
{
	int ret=0;
	flash_t flash_config;

	if (buffer== NULL || length < 1) 
	{
		return -1;
	}
	//Read flash direct
	device_mutex_lock(RT_DEV_LOCK_FLASH);
	ret = flash_stream_read(&flash_config, FAST_RECONNECT_DATA, length, (u8 *)buffer);
	device_mutex_unlock(RT_DEV_LOCK_FLASH);
	if(-1 == ret)
	{
		log_printf(LOG_WARNING"[%s]read open file fail\n",__FUNCTION__);
		return -1;
	}
		log_printf(LOG_WARNING"[%s]read open file successful\n",__FUNCTION__);
	return 0;
}

void rlt_wifi_info_clean()
{
	flash_t flash_config;
	flash_erase_sector(&flash_config, FAST_RECONNECT_DATA);
	return;
}


