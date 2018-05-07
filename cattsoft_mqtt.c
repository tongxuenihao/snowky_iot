/*************************************************************************
	> File Name: cattsoft_mqtt.c
	> Author: xuetong
	> Mail: 
	> Created Time: Mon 07 May 2018 02:23:53 PM CST
 ************************************************************************/
#include "common.h"

t_mqtt_broke mqtt_broker;

int send_packet(int socket_fd, unsigned char *buff, unsigned int buff_len)
{
	int ret;
	ret = send(socket_fd, buff, buff_len, 0);
	if(ret > 0)
	{
		    log_printf(LOG_DEBUG"---->NET(%d):\n", ret);
    		print_hex(buff, buff_len);
	}
	return ret;
}


static void mqtt_init(t_mqtt_broke *broke, char *clientid)
{
	broke->alive = CLOUD_MQTT_SET_ALIVE;
	broke->seq = 1;
	memset(broke->clientid, 0, sizeof(broke->clientid));
	memset(broke->user_name, 0, sizeof(broke->user_name));
	memset(broke->password, 0, sizeof(broke->password));
	if(clientid)
	{
		strncpy(broke->clientid, clientid, sizeof(broke->clientid));
	}
	else
	{
		strcpy(broke->clientid, "emqtt");
	}
	broke->clean_session = 1;
}

static void mqtt_auth_init(t_mqtt_broke *broke, char *user_name, char *user_password)
{
	if(user_name && user_name[0] != '\0')
	{		
		strncpy(broke->user_name, user_name, sizeof(broke->user_name) -1);
	}

	if(user_password && user_password[0] != '\0')
	{
		strncpy(broke->password, user_name, sizeof(broke->password) - 1);
	}
}

static int mqtt_connect(t_mqtt_broke *broker) 
{
	unsigned char flags = 0x00;
    int ret;
    unsigned char *fixed_header = NULL;
    int fixed_header_len;
    unsigned char *packet =NULL;
    int packet_len;
    unsigned short offset = 0;
    unsigned short clientidlen,usernamelen,passwordlen,payload_len;
    unsigned char fixed_header_size;
    unsigned char remain_len;
    unsigned char var_header[12] = {
        0x00,0x06,0x4d,0x51,0x49,0x73,0x64,0x70, // Protocol name: MQIsdp
        0x03, // Protocol version
        0, // Connect flags
        0, 0, // Keep alive
    };
		
    var_header[10]=  broker->alive>>8;
    var_header[11]= broker->alive&0xFF;
	
    clientidlen = strlen(broker->clientid);
    usernamelen = strlen(broker->user_name);
    passwordlen = strlen(broker->password);
    payload_len = clientidlen + 2;

    payload_len += usernamelen + 2;
    flags |= MQTT_USERNAME_FLAG;

    payload_len += passwordlen + 2;
    flags |= MQTT_PASSWORD_FLAG;

    if(broker->clean_session)
    {
        flags |= MQTT_CLEAN_SESSION;
    }
    var_header[9]= flags;
    fixed_header_size = 2; 
    remain_len = sizeof(var_header)+payload_len;
    if (remain_len > 127) 
    {
        fixed_header_size++;         
    }
    fixed_header = (unsigned char *)malloc(fixed_header_size);
    if(fixed_header == NULL)
    {
		log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
        return -1;
    }
    fixed_header_len = fixed_header_size;
    fixed_header[0] = MQTT_MSG_CONNECT;
    if (remain_len <= 127) 
    {
        fixed_header[1] = remain_len;
    } 
    else 
    {
        fixed_header[1] = remain_len % 128;
        fixed_header[1] = fixed_header[1] | 0x80;
        fixed_header[2] = remain_len / 128;
    }
	
    packet = (unsigned char *)malloc(fixed_header_len + sizeof(var_header)+payload_len );
    if(packet == NULL)
    {
    	log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
        free(fixed_header);
        return -1;
    }
    packet_len = fixed_header_len + sizeof(var_header)+payload_len;
    memset(packet, 0, packet_len);
    memcpy(packet, fixed_header, fixed_header_len);
    offset += fixed_header_len;
    memcpy(packet + offset, var_header, sizeof(var_header));
    offset += sizeof(var_header);

    packet[offset++] = clientidlen >> 8;
    packet[offset++] = clientidlen & 0xFF;
    memcpy(packet+offset, broker->clientid, clientidlen);
    offset += clientidlen;

    packet[offset++] = usernamelen>>8;
    packet[offset++] = usernamelen&0xFF;
    memcpy(packet+offset, broker->user_name, usernamelen);
    offset += usernamelen;

    packet[offset++] = passwordlen>>8;
    packet[offset++] = passwordlen&0xFF;
    memcpy(packet+offset, broker->password, passwordlen);
    offset += passwordlen;	

    ret = broker->mqtt_send(broker->socket_fd, packet, packet_len);
    if(ret < packet_len) 
    {      
        free(fixed_header);
        free(packet);
        return -1;
    } 
    free(fixed_header);
    free(packet);
    return 1;
}


int mqtt_connect_packet_send(t_mqtt_broke *pst_mqtt_broker, unsigned int socket_fd, char *user_name, char *user_password)
{
	int ret;
	if((user_name == NULL) || (user_password == NULL))
	{
		return -1;
	}
	else
	{
		mqtt_init(pst_mqtt_broker, user_name);
		mqtt_auth_init(pst_mqtt_broker, user_name, user_password);
	}
	pst_mqtt_broker->socket_fd = socket_fd;
	pst_mqtt_broker->mqtt_send = send_packet;

	ret = mqtt_connect(pst_mqtt_broker);
	if(ret == 1)
	{
		log_printf(LOG_DEBUG"[%s]connect packet send ok\n",__FUNCTION__);
		return 1;
	}

	return -1;
}

int cattsoft_m2m_login_request(int socket_fd, char *user_name, char *user_password)
{
	int ret;
	ret = mqtt_connect_packet_send(&mqtt_broker, socket_fd, user_name, user_password);
	if(ret == 1)
	{
		return 1;
	}
	return -1;
}

