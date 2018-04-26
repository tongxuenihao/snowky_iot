/*************************************************************************
	> File Name: data_type_def.h
	> Author: xuetong
	> Mail: 
	> Created Time: Tue 24 Apr 2018 11:47:23 AM CST
 ************************************************************************/

#ifndef _DATA_TYPE_DEF_H
#define _DATA_TYPE_DEF_H

#define print_hex(a,b)  { \
                int count_tmp; \
                for(count_tmp = 0;count_tmp < b;){ \
                        printf("%02x ",a[count_tmp++]); \
                        if(count_tmp % 16 == 0){ \
                                printf("\n"); \
                        } \
                } \
                printf("\n"); \
        }  
#define print_char(a,b)  { \
										int count_tmp; \
										for(count_tmp = 0;count_tmp < b;){ \
												printf("%c,",a[count_tmp++]); \
												} \
												printf("\n"); \
												}


#endif
