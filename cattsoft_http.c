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
	char *url = "/dev/reg";
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


int cattsoft_device_prelogin_request(int socket_fd)
{
	int ret;
	cJSON *json_Object;
	char *url = "/dev/reg";
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
		tdata = (unsigned char *)malloc(200);
		if(tdata == NULL)
		{
			log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
			return -1;
		}
		memset(tdata, 0, 200);
    	snprintf( (char *)tdata,200,"%s %s%s %s%s%s %s%s%s%s%s",
            "GET",url,content,"HTTP/1.1",kCRLFNewLine,
            "Host:",HTTP_SERVER,kCRLFNewLine
            "Cache-Control: no-cache",kCRLFNewLine,
            "Content-Type: application/x-www-form-urlencoded",kCRLFLineEnding);
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

int cattsoft_http_registerion_parse(unsigned char *data)
{
	t_dev_info *dev_info;
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
			log_printf(LOG_WARNING"[%s]get statusCode error,ret:%d\n",__FUNCTION__,p_sub->valueint);
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
		memcpy((unsigned char *)dev_info->did, cJSON_GetObjectItem(json_Object, "devid")->valuestring, DID_LEN);
		dev_info->did[DID_LEN] = '\0';
		memcpy((unsigned char *)dev_info->access_token, cJSON_GetObjectItem(json_Object, "accessToken")->valuestring, TOKEN_LEN);
		dev_info->access_token[TOKEN_LEN] = '\0';
		rlt_device_info_write((unsigned char *)dev_info, sizeof(t_dev_info));
		free(dev_info);
	}
	return 1;
}

int cattsoft_http_prelogin_parse(unsigned char *data, char *host, int *port)
{
	t_dev_info *dev_info;
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
	return 1;
}


int cattsoft_http_registerion(unsigned char *data, int response_code)
{
	int ret;
	if(response_code != 201)
	{
		printf("[%s]did get response code error:%d\n",__FUNCTION__,response_code);
		return -1;
	}
	ret = cattsoft_http_registerion_parse(data);
	if(ret == 1)
	{
		return 1;
	}
	return -1;
}

int cattsoft_http_prelogin(unsigned char *data, char *host, int *port, int response_code)
{
	int ret;
	if(response_code != 200)
	{
		log_printf(LOG_DEBUG"[%s]did get response code error\n",__FUNCTION__);
		return -1;
	}
	ret = cattsoft_http_prelogin_parse(data, host, port);
	if(ret == 1)
	{
		return 1;
	}
	return -1;
}


