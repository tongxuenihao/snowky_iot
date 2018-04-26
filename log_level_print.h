/*************************************************************************
	> File Name: log_level_print.h
	> Author: 
	> Mail: 
	> Created Time: Tue 24 Apr 2018 11:46:23 AM CST
 ************************************************************************/

#ifndef _LOG_LEVEL_PRINT_H
#define _LOG_LEVEL_PRINT_H

#define __func__ __FUNCTION__

#undef LOG_ALERT
#undef LOG_WARNING
#undef LOG_INFO
#undef LOG_DEBUG
#define LOG_ALERT "<0>"
#define LOG_WARNING "<1>"
#define LOG_INFO "<2>"
#define LOG_DEBUG "<3>"

#define MAX_PRINTF_BUF 256

/*If user want to change log-level , use this macro */
#define DEFAULT_LOG_LEVEL 3

/* User should not change this macro */
#define DEFAULT_LOG_PRINTF_LEVEL (DEFAULT_LOG_LEVEL + '0')

#define MAX_LOG_LEVEL 3
#define MAX_LOG_PRINTF_LEVEL (MAX_LOG_LEVEL + '0')

void set_log_printf_level(int log_level);
void log_printf(char *format,...);

#endif
