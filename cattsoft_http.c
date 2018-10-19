/*************************************************************************
	> File Name: cattsoft_http.c
	> Author: xuetong
	> Mail: 
	> Created Time: Wed 02 May 2018 03:00:32 PM CST
 ************************************************************************/
#include "common.h"

extern struct netif xnetif[NET_IF_NUM]; 

int cattsoft_get_resonse_code(unsigned char *data)
{
    int response_code=0;
    char *p_start = NULL;
    char *p_end =NULL; 
    char re_code[10] ={0};
    memset(re_code,0,sizeof(re_code));

    p_start = strstr((char *)data," ");
    if(NULL == p_start)
    {
        return -1;
    }
    p_end = strstr(++p_start," ");
    if(p_end)
    {
        if(p_end - p_start > sizeof(re_code))
        {
            return -1;
        }
        memcpy(re_code,p_start, (p_end-p_start));
    }

    response_code = atoi(re_code); 
    return response_code;
}


int cattsoft_device_register_request(int socket_fd)
{
	int ret;
	unsigned char *mac;
	unsigned char macstr[13] ={0};
	cJSON *json_Object;
	char *url = "/snowkyMobile/dev/reg";

	char *content;
	unsigned int content_len;
	unsigned char *tdata;
	unsigned int tdata_len;
	t_dev_info *dev_info;
	dev_info = (t_dev_info *)malloc(sizeof(t_dev_info));
	if(dev_info == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return -1;
	}
	memset((unsigned char *)dev_info, 0, sizeof(t_dev_info));
	rlt_device_info_read((unsigned char *)dev_info, sizeof(t_dev_info));
	mac = LwIP_GetMAC(&xnetif[0]);
	sprintf(macstr, "%02x%02x%02x%02x%02x%02x",mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	json_Object = cJSON_CreateObject();
	if(json_Object != NULL)
	{
		cJSON_AddStringToObject(json_Object, "sn", dev_info->dev_sn);
		cJSON_AddStringToObject(json_Object, "mac", macstr);
		cJSON_AddStringToObject(json_Object, "mcuVersion", dev_info->dev_ver);
		cJSON_AddNumberToObject(json_Object, "devType", dev_info->dev_type);
	}
	content = cJSON_Print(json_Object);
	if(content != NULL)
	{
		tdata = (unsigned char *)malloc(1024);
		if(tdata == NULL)
		{
			log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
			return -1;
		}
		memset(tdata, 0, 1024);
		content_len=strlen(content);
    	snprintf( (char *)tdata,1024,"%s %s %s%s%s %s%s%s %d%s%s%s%s%s",
            "POST" ,url,"HTTP/1.1",kCRLFNewLine,
            "Host:",HTTP_SERVER,kCRLFNewLine,
            "Content-Length:",content_len,kCRLFNewLine,
            "Content-Type: text/plain",kCRLFNewLine,
            kCRLFNewLine,
            content
            );
    	tdata_len = strlen(tdata);
    	ret = send(socket_fd, tdata, tdata_len, 0);
    	if(ret >= 0)
    	{
		    log_printf(LOG_DEBUG"---->NET:\n");
    		printf("%s\n", tdata);
    		free(dev_info);
    		free(tdata);
    		return 1;
    	}
	}
	return -1;
}

int cattsoft_device_logout_request(int socket_fd)
{
	int ret;
	cJSON *json_Object;
	char *url = "/snowkyMobile/dev/cancel";
	char *content;
	unsigned int content_len;
	unsigned char *tdata;
	unsigned int tdata_len;
	t_dev_info *dev_info;
	dev_info = (t_dev_info *)malloc(sizeof(t_dev_info));
	if(dev_info == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return -1;
	}
	memset((unsigned char *)dev_info, 0, sizeof(t_dev_info));
	rlt_device_info_read((unsigned char *)dev_info, sizeof(t_dev_info));
	json_Object = cJSON_CreateObject();
	if(json_Object != NULL)
	{
		cJSON_AddStringToObject(json_Object, "accessToken", dev_info->access_token);
	}
	content = cJSON_Print(json_Object);
	if(content != NULL)
	{
		tdata = (unsigned char *)malloc(512);
		if(tdata == NULL)
		{
			log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
			return -1;
		}
		memset(tdata, 0, 512);
		content_len=strlen(content);
    	snprintf( (char *)tdata,1024,"%s %s %s%s%s %s%s%s %d%s%s%s%s%s",
            "POST" ,url,"HTTP/1.1",kCRLFNewLine,
            "Host:",HTTP_SERVER,kCRLFNewLine,
            "Content-Length:",content_len,kCRLFNewLine,
            "Content-Type: text/plain",kCRLFNewLine,
            kCRLFNewLine,
            content
            );
    	tdata_len = strlen(tdata);
    	ret = send(socket_fd, tdata, tdata_len, 0);
    	if(ret >= 0)
    	{
		    log_printf(LOG_DEBUG"---->NET:\n");
    		printf("%s\n", tdata);
    		free(dev_info);
    		free(tdata);
    		return 1;
    	}
	}
	return -1;
}


int cattsoft_device_prelogin_request(int socket_fd)
{
	int ret;
	unsigned int content_len;
	cJSON *json_Object;
	char *url = "/snowkyMobile/dev/prelogin";
	char *content;
	unsigned char *tdata;
	unsigned int tdata_len;
	t_dev_info *dev_info;
	dev_info = (t_dev_info *)malloc(sizeof(t_dev_info));
	if(dev_info == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return -1;
	}
	memset((unsigned char *)dev_info, 0, sizeof(t_dev_info));
	rlt_device_info_read((unsigned char *)dev_info, sizeof(t_dev_info));
	json_Object = cJSON_CreateObject();
	if(json_Object != NULL)
	{
		cJSON_AddStringToObject(json_Object, "devId", dev_info->did);
		cJSON_AddStringToObject(json_Object, "accessToken", dev_info->access_token);
	}
	content = cJSON_Print(json_Object);
	if(content != NULL)
	{
		tdata = (unsigned char *)malloc(512);
		if(tdata == NULL)
		{
			log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
			return -1;
		}
		memset(tdata, 0, 512);
		content_len=strlen(content);
    	snprintf( (char *)tdata,1024,"%s %s %s%s%s %s%s%s %d%s%s%s%s%s",
            "POST" ,url,"HTTP/1.1",kCRLFNewLine,
            "Host:",HTTP_SERVER,kCRLFNewLine,
            "Content-Length:",content_len,kCRLFNewLine,
            "Content-Type: text/plain",kCRLFNewLine,
            kCRLFNewLine,
            content
            );

    	tdata_len = strlen(tdata);
    	ret = send(socket_fd, tdata, tdata_len, 0);
    	if(ret >= 0)
    	{
		    log_printf(LOG_DEBUG"---->NET:\n");
    		printf("%s\n", tdata);
    		free(dev_info);
    		free(tdata);
    		return 1;
    	}
	}
	return -1;
}

void get_info_from_cjson(char *src, char *name_str, char *info_str)
{
	char *str_pos;
	char *end;
	int len;
	str_pos = strstr(src, name_str);
	if(!str_pos)
	{
		printf("[%s]str not found\n",__FUNCTION__);
		return;
	}
	str_pos += (strlen(name_str) + NET_INFO_POS);
	end = strchr(str_pos, '"');
	len = end - str_pos;
	strncpy(info_str, str_pos, len);
	info_str[len] = '\0';
	printf("[%s]info_str:%s\n",__FUNCTION__,info_str);
}

int cattsoft_http_registerion_parse(unsigned char *data)
{
	log_printf(LOG_DEBUG"[%s]\n",__FUNCTION__);
	t_dev_info *dev_info;
	char *header = NULL;
	char *body = NULL;
	char *dev_id;
	char *access_token;
	char *temp_str;
	char *end;
	int len;
	
	header = strstr(data, kCRLFLineEnding);
	if(header) 
	{
		body = header + strlen("\r\n\r\n");
		printf("[%s]get kCRLFLineEnding ok\n",__FUNCTION__);
	}
	else
	{
		printf("[%s]get kCRLFLineEnding error\n",__FUNCTION__);
		return -1;
	}
	printf("msg body:%s\n", body);

	dev_info = (t_dev_info *)malloc(sizeof(t_dev_info));
	if(dev_info == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return -1;
	}
	memset((unsigned char *)dev_info, 0, sizeof(t_dev_info));
	rlt_device_info_read((unsigned char *)dev_info, sizeof(t_dev_info));

	get_info_from_cjson(body, "devid", dev_info->did);
	get_info_from_cjson(body, "accessToken", dev_info->access_token);
	dev_info->alink_reset_flag = 0x02;
	rlt_device_info_write((unsigned char *)dev_info, sizeof(t_dev_info));
	free(dev_info);
	return 1;
}

int cattsoft_http_prelogin_parse(unsigned char *data, char *host, int *port)
{
	t_dev_info *dev_info;
	char *header = NULL;
	char *body = NULL;
	char *temp_str;
	char *mqttservPwd;
	char *mqttservIp;
	char *mqttservPort;
	char port_arry[5];
	header = strstr(data, kCRLFLineEnding);
	if(header) 
	{
		body = header + strlen("\r\n\r\n");
	}
	else
	{
		log_printf(LOG_WARNING"[%s]get kCRLFLineEnding error\n",__FUNCTION__);
		return -1;
	}

	dev_info = (t_dev_info *)malloc(sizeof(t_dev_info));
	if(dev_info == NULL)
	{
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
		return -1;
	}
	memset((unsigned char *)dev_info, 0, sizeof(t_dev_info));
	rlt_device_info_read((unsigned char *)dev_info, sizeof(t_dev_info));

	get_info_from_cjson(body, "mqttservIp", host);
	get_info_from_cjson(body, "mqttservPort", port_arry);
	*port = atoi(port_arry);
	get_info_from_cjson(body, "mqttservPwd", dev_info->access_key);
#if 0
//mqttservIp
	temp_str = "mqttservIp";
	mqttservIp = strstr(body, temp_str);
	if(!mqttservIp)
	{
		printf("[%s]accessToken not found\n",__FUNCTION__);
		return -1;
	}
	mqttservIp += strlen(temp_str);
	strncpy(host, mqttservIp + 5, 13);
	host[13] = '\0';

//mqttservPort
	temp_str = "mqttservPort";
	mqttservPort = strstr(body, temp_str);
	if(!mqttservPort)
	{
		printf("[%s]accessToken not found\n",__FUNCTION__);
		return -1;
	}
	mqttservPort += strlen(temp_str);
	strncpy(port_arry, mqttservPort + 5, 4);
	port_arry[4] = '\0';
	*port = atoi(port_arry);

//mqttservPwd
	temp_str = "mqttservPwd";
	mqttservPwd = strstr(body, temp_str);
	if(!mqttservPwd)
	{
		printf("[%s]accessToken not found\n",__FUNCTION__);
		return -1;
	}
	mqttservPwd += strlen(temp_str);
	strncpy(dev_info->access_key, mqttservPwd + 5, ACCESS_KEY);
	dev_info->access_key[ACCESS_KEY] = '\0';
	printf("[%s]mqttservPwd:%s\n",__FUNCTION__,dev_info->access_key);
#endif
	rlt_device_info_write((unsigned char *)dev_info, sizeof(t_dev_info));
	free(dev_info);
	return 1;
#if 0
	cJSON * json_Object = cJSON_Parse(body);
	if(json_Object != NULL)
	{
		cJSON * p_sub = cJSON_GetObjectItem(json_Object, "statusCode");
		if(p_sub->valueint != 0)
		{
			log_printf(LOG_WARNING"[%s]get statusCode error,ret:%d\n",__FUNCTION__,p_sub->valueint);
			return -1;
		}

		dev_info = (t_dev_info *)malloc(sizeof(t_dev_info));
		if(dev_info == NULL)
		{
			log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
			return -1;
		}
		memcpy(host, cJSON_GetObjectItem(json_Object, "mqttservIp")->valuestring, strlen(cJSON_GetObjectItem(json_Object, "mqttservIp")->valuestring));
		*port = atoi(cJSON_GetObjectItem(json_Object, "mqttservPort")->valuestring);
		memset((unsigned char *)dev_info, 0, sizeof(t_dev_info));
		rlt_device_info_read((unsigned char *)dev_info, sizeof(t_dev_info));
		memcpy((unsigned char *)dev_info->access_key, cJSON_GetObjectItem(json_Object, "mqttservPwd")->valuestring, ACCESS_KEY);
		dev_info->access_key[ACCESS_KEY] = '\0';
		rlt_device_info_write((unsigned char *)dev_info, sizeof(t_dev_info));
		free(dev_info);
	}
	#endif

}

int cattsoft_http_logout_parse(unsigned char *data)
{
	char *header = NULL;
	char *body = NULL;
	header = strstr(data, kCRLFLineEnding);
	if(header) 
	{
		body = header + strlen("\r\n\r\n");
	}
	else
	{
		log_printf(LOG_WARNING"[%s]get kCRLFLineEnding error\n",__FUNCTION__);
		return -1;
	}
	cJSON * json_Object = cJSON_Parse(body);
	if(json_Object != NULL)
	{
		cJSON * p_sub = cJSON_GetObjectItem(json_Object, "statusCode");
		if(p_sub->valueint != 0)
		{
			log_printf(LOG_WARNING"[%s]get statusCode error\n",__FUNCTION__);
			return -1;
		}
		log_printf(LOG_DEBUG"[%s]logout success!\n",__FUNCTION__);
	}
	return 1;
}


int cattsoft_http_registerion(unsigned char *data, int response_code)
{
	int ret;
	if(response_code != 200)
	{
		printf("[%s]did get response code error:%d\n",__FUNCTION__,response_code);
		return -1;
	}
	printf("[%s]did get response code ok:%d\n",__FUNCTION__,response_code);
	ret = cattsoft_http_registerion_parse(data);
	if(ret == 1)
	{
		return 1;
	}
	printf("[%s]error :-1\n",__FUNCTION__);
	return -1;
}

int cattsoft_http_prelogin(unsigned char *data, char *host, int *port, int response_code)
{
	int ret;
	if(response_code != 200)
	{
		log_printf(LOG_DEBUG"[%s]prelogin get response code error\n",__FUNCTION__);
		return -1;
	}
	ret = cattsoft_http_prelogin_parse(data, host, port);
	if(ret == 1)
	{
		return 1;
	}
	return -1;
}

int cattsoft_http_logout(unsigned char *data, int response_code)
{
	int ret;
	if(response_code != 200)
	{
		log_printf(LOG_DEBUG"[%s]did get response code error\n",__FUNCTION__);
		return -1;
	}
	ret = cattsoft_http_logout_parse(data);
	if(ret == 1)
	{
		return 1;
	}
	return -1;
}


