/*************************************************************************
	> File Name: cattsoft_mqtt.c
	> Author: xuetong
	> Mail: 
	> Created Time: Mon 07 May 2018 02:23:53 PM CST
 ************************************************************************/
#include "common.h"

t_mqtt_broke mqtt_broker;
unsigned short mqtt_message_id;

varc Tran2varc(unsigned int remainLen)
{
    int i;
    varc Tmp;

    memset(&Tmp, 0x0, sizeof(Tmp));

    Tmp.varcbty = 1;
    for(i = 0; i < 4; i++)
    {
        Tmp.var[i] = remainLen % 128;
        remainLen >>= 7;
        if(remainLen)
        {
            Tmp.var[i] |= 0x80;
            Tmp.varcbty++;
        }
        else
        {
            break;
        }
    }
    return Tmp;
}

unsigned char mqtt_num_rem_len_bytes(unsigned char *buf) 
{
    unsigned char num_bytes = 1;
    if ((buf[1] & 0x80) == 0x80) 
    {
        num_bytes++;
        if ((buf[2] & 0x80) == 0x80) 
        {
            num_bytes ++;
            if ((buf[3] & 0x80) == 0x80) 
            {
                num_bytes ++;
            }
        }
    }
    return num_bytes;
}

unsigned short mqtt_parse_rem_len(unsigned char *buf) 
{
    unsigned short multiplier = 1;
    unsigned short value = 0;
    unsigned char digit;

    buf++;
    do {
        digit = *buf;
        value += (digit & 127) * multiplier;
        multiplier *= 128;
        buf++;
    } while ((digit & 128) != 0);
    return value;
}

unsigned short mqtt_parse_pub_topic_ptr(unsigned char* buf, unsigned char **topic_ptr) 
{
    unsigned short len = 0;
    if(MQTTParseMessageType(buf) == MQTT_MSG_PUBLISH) {
        uint8_t rlb = mqtt_num_rem_len_bytes(buf);
        len = *(buf+1+rlb)<<8;  // MSB of topic UTF
        len |= *(buf+1+rlb+1);  // LSB of topic UTF
        *topic_ptr = (buf + (1+rlb+2));
    } 
    else 
    {
        *topic_ptr = NULL;
    }
    return len;
}


unsigned short mqtt_parse_pub_topic(unsigned char* buf, unsigned char* topic) 
{
    unsigned char* ptr;
    unsigned short topic_len = mqtt_parse_pub_topic_ptr(buf, &ptr);
    if(topic_len != 0 && ptr != NULL) 
    {
        memcpy(topic, ptr, topic_len);
    }

    return topic_len;
}


unsigned short mqtt_parse_pub_msg_ptr(const unsigned char* buf, const unsigned char **msg_ptr) 
{
    unsigned short len = 0;
    if(MQTTParseMessageType(buf) == MQTT_MSG_PUBLISH) 
    {
        // message starts at
        // fixed header length + Topic (UTF encoded) + msg id (if QoS>0)
        uint8_t rlb = mqtt_num_rem_len_bytes(buf);
        uint8_t offset = (*(buf+1+rlb))<<8; // topic UTF MSB
        offset |= *(buf+1+rlb+1);           // topic UTF LSB
        offset += (1+rlb+2);                // fixed header + topic size

        if(MQTTParseMessageQos(buf)) {
            offset += 2;                    // add two bytes of msg id
        }

        *msg_ptr = (buf + offset);
        len = mqtt_parse_rem_len(buf) - (offset-(rlb+1));
    } 
    else 
    {
        *msg_ptr = NULL;
    }
    return len;
}

unsigned short mqtt_parse_publish_msg(const unsigned char* buf, unsigned char** msg) 
{
    unsigned char* ptr;
    unsigned short msg_len = mqtt_parse_pub_msg_ptr(buf, (const uint8_t **)&ptr);

    if(msg_len != 0 && ptr != NULL) 
    {
        (*msg)=ptr;
    }

    return msg_len;
}

int send_packet(int socket_fd, unsigned char *buff, unsigned int buff_len)
{
	int ret;
	log_printf(LOG_DEBUG"---->NET:\n");
    print_hex(buff, buff_len);
	ret = send(socket_fd, buff, buff_len, 0);
	if(ret > 0)
	{
		    //log_printf(LOG_DEBUG"---->NET:\n");
    		//print_hex(buff, buff_len);
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
		strcpy(broke->clientid, "xuetong");
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
		strncpy(broke->password, user_password, sizeof(broke->password) - 1);
	}
}

static int mqtt_connect(t_mqtt_broke *broker,unsigned char *dev_did, int did_len) 
{
    char sub_topic_buff[128];            //fix
    int topiclen;
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
    unsigned char var_header[10] = {
        0x00,0x04,0x4D,0x51,0x54,0x54, // Protocol name: MQTT
        0x04, // Protocol version
        0, // Connect flags
        0, 0, // Keep alive
    };
		
    var_header[8]=  broker->alive>>8;
    var_header[9]= broker->alive&0xFF;
	
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

	flags |= MQTT_WILL_FLAG;                //fix
    payload_len += (did_len + strlen("dev2ser/lastWill/") + 2);

	flags |= MQTT_WILL_RETAIN;  
	
	flags |= MQTT_WILL_QoS2;  

	payload_len += 2;//(strlen("close") + 2);

    var_header[7]= flags;
    fixed_header_size = 2; 
    remain_len = sizeof(var_header) + payload_len;
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
    packet_len = fixed_header_len + sizeof(var_header) + payload_len;
    memset(packet, 0, packet_len);
    memcpy(packet, fixed_header, fixed_header_len);
    offset += fixed_header_len;
    memcpy(packet + offset, var_header, sizeof(var_header));
    offset += sizeof(var_header);

    packet[offset++] = clientidlen >> 8;
    packet[offset++] = clientidlen & 0xFF;
    memcpy(packet+offset, broker->clientid, clientidlen);
    offset += clientidlen;
#if 1
    topiclen = did_len + strlen("dev2ser/lastwill/");
    memset(sub_topic_buff, 0, 128);           //fix
    sub_topic_buff[0] = topiclen >> 8;
    sub_topic_buff[1] = topiclen & 0xFF;
    memcpy(sub_topic_buff + 2, "dev2ser/lastwill/", strlen("dev2ser/lastwill/"));
    memcpy(sub_topic_buff + strlen("dev2ser/lastwill/") + 2, dev_did, did_len);

	sub_topic_buff[topiclen + 2] = 0;//strlen("close") >> 8;
	sub_topic_buff[topiclen + 3] = 0;//strlen("close") & 0xFF;
	//memcpy(sub_topic_buff + topiclen + 4, "close", strlen("close"));
	
    memcpy(packet+offset, sub_topic_buff, topiclen + 4);
    offset += (topiclen + 4);       //fix	
#endif

    packet[offset++] = usernamelen>>8;
    packet[offset++] = usernamelen&0xFF;
    memcpy(packet+offset, broker->user_name, usernamelen);
    offset += usernamelen;

    packet[offset++] = passwordlen>>8;
    packet[offset++] = passwordlen&0xFF;
    memcpy(packet+offset, broker->password, passwordlen);
    offset += passwordlen;
	
	print_hex(packet, offset);
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
	ret = mqtt_connect(pst_mqtt_broker, user_name, DID_LEN);
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


int cattsoft_m2m_login_response(unsigned char *rdata)
{
    if(rdata == NULL)
    {
        return -1;
    }
    if(rdata[3] == 0)
    {
        if(rdata[0] !=0 && rdata[1] != 0)
        {
            log_printf(LOG_DEBUG"[%s]m2m connect ok\n",__FUNCTION__);
            return 1;
        }
    }
    log_printf(LOG_WARNING"[%s]m2m connect fail\n",__FUNCTION__);
    return -1;
}


int mqtt_subscribe(t_mqtt_broke *broker, char *topic, unsigned short *message_id)
{
    int ret;
    unsigned short topiclen = strlen(topic);
    unsigned char *utf_topic = NULL;
    unsigned char *packet = NULL;   
    int utf_topic_len;
    int packet_len;  
    unsigned char fixed_header[2];
    unsigned char var_header[2];

    var_header[0] = broker->seq >> 8;
    var_header[1] = broker->seq & 0xFF;
    if(message_id) {
        *message_id = broker->seq;
    }
    broker->seq++;

    utf_topic = (unsigned char *)malloc(topiclen + 3);
    if(utf_topic == NULL)
    {
        return -1;
    }
    utf_topic_len = topiclen + 3;
    memset(utf_topic, 0, utf_topic_len);
    utf_topic[0] = topiclen >> 8;
    utf_topic[1] = topiclen & 0xFF;
    memcpy(utf_topic + 2, topic, topiclen);
	utf_topic[utf_topic_len - 1] = 0x02;

    fixed_header[0] = MQTT_MSG_SUBSCRIBE | MQTT_QOS1_FLAG;
    fixed_header[1] = sizeof(var_header) + utf_topic_len;
     
    packet_len = sizeof(var_header) + sizeof(fixed_header) + utf_topic_len;
    packet = (unsigned char *)malloc(packet_len);
    if(packet==NULL)
    {
        log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
        free(utf_topic);
        return -1;
    }
    memset(packet, 0, packet_len);
    memcpy(packet, fixed_header, sizeof(fixed_header));
    memcpy(packet+sizeof(fixed_header), var_header, sizeof(var_header));
    memcpy(packet+sizeof(fixed_header)+sizeof(var_header), utf_topic, utf_topic_len);

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


void subtopic_init(char *topic_buff, char *dev_did, int flag)
{
    int did_len;
    did_len = strlen(dev_did);
    if(did_len != DID_LEN)
    {
        printf("did:%s\n",dev_did);
        printf("[%s]did length:%d\n",__FUNCTION__,did_len);
        log_printf(LOG_WARNING"[%s]did length error\n",__FUNCTION__);
        return;  
    }
    switch(flag)
    {
        case 1:		
            memcpy(topic_buff, "ser2dev/req/", strlen("ser2dev/req/"));
            memcpy(topic_buff + strlen("ser2dev/req/"), dev_did, did_len);
            topic_buff[did_len + strlen("ser2dev/req/")] = '\0';
            break;

        case 2:
            memcpy(topic_buff, "ser2dev/noack/", strlen("ser2dev/noack/"));
            memcpy(topic_buff + strlen("ser2dev/noack/"), dev_did, did_len);
            topic_buff[did_len + strlen("ser2dev/noack/")] = '\0';
            break;

        case 3:
            memcpy(topic_buff, "ser2dev/res/", strlen("ser2dev/res/"));
            memcpy(topic_buff + strlen("ser2dev/res/"), dev_did, did_len);
            topic_buff[did_len + strlen("ser2dev/res/")] = '\0';
            break;

        default:
            break;
    }
}

int mqtt_subscribe_login_topic(t_mqtt_broke *pst_mqtt_broker, char *dev_did, int flag)
{
    int ret;
    char sub_topic_buff[128];
    memset(sub_topic_buff, 0, 128);
    switch(flag)
    {
        case 1:
            subtopic_init(sub_topic_buff, dev_did, 1);
            ret = mqtt_subscribe(pst_mqtt_broker, sub_topic_buff, &mqtt_message_id);
            if(ret == 1)
            {
                printf("login topic 1: %s\n", sub_topic_buff);
            }
            break;

        case 2:
            subtopic_init(sub_topic_buff, dev_did, 2);
            ret = mqtt_subscribe(pst_mqtt_broker, sub_topic_buff, &mqtt_message_id);
            if(ret == 1)
            {
                printf("login topic 2: %s\n", sub_topic_buff);
            }
            break;

        case 3:
            subtopic_init(sub_topic_buff, dev_did, 3);
            ret = mqtt_subscribe(pst_mqtt_broker, sub_topic_buff, &mqtt_message_id);
            if(ret == 1)
            {
                printf("login topic 3: %s\n", sub_topic_buff);
            }
            break;

        default:
            break;
    }
    return 1;
}

int cattsoft_m2m_subscribe_topic_request(char *dev_did, int flag)
{
    int ret;
    ret = mqtt_subscribe_login_topic(&mqtt_broker, dev_did, flag);
    if(ret == 1)
    {
        return 1;
    }
    return -1;
}


unsigned char mqtt_parse_msg_id(unsigned char *buf) 
{
    unsigned char type = MQTTParseMessageType(buf);
    unsigned char qos = MQTTParseMessageQos(buf);
    unsigned char id = 0;
    
    if(type >= MQTT_MSG_PUBLISH && type <= MQTT_MSG_UNSUBACK) 
    {
        if(type == MQTT_MSG_PUBLISH) 
        {
            if(qos != 0) 
            {
                unsigned char rlb = mqtt_num_rem_len_bytes(buf);
                unsigned char offset = *(buf + 1 + rlb) << 8; 
                offset |= *(buf + 1 + rlb + 1);          
                offset += (1 + rlb + 2);                  
                id = *(buf + offset) << 8;             
                id |= *(buf + offset + 1);           
            }
        } 
        else 
        {
            unsigned char rlb = mqtt_num_rem_len_bytes(buf);
            id = *(buf + 1 + rlb) << 8;
            id |= *(buf + 1 + rlb + 1);  
        }
    }
    return id;
}

int cattsoft_m2m_subscribe_response(unsigned char *rdata)
{
    unsigned short recv_msg_id = 0;
    if(rdata == NULL)
    {
        return -1;
    }
    recv_msg_id = mqtt_parse_msg_id(rdata);
    if( recv_msg_id != mqtt_message_id)
    {
        return -1;
    }
    return 1;
}

int mqtt_publish_with_qos(t_mqtt_broke *broker, unsigned char *rdata , unsigned int rdata_len, char *dev_did, unsigned char qos, unsigned short *message_id)
{
    int ret;
    char topic[100];
    unsigned char qos_flag = MQTT_QOS0_FLAG;
    unsigned char qos_size = 0; // No QoS included
    unsigned short topiclen;

    int var_header_len;
    int fixed_header_len;
    int tdata_len;

    unsigned char *tdata = NULL;
    unsigned char *var_header = NULL;
    unsigned char *fixed_header = NULL;
    unsigned char fixed_header_size = 1;
    unsigned short remain_len;
    varc sendvarc;

    if(rdata[7] == 0x21)
    {
        memcpy(topic, "dev2ser/res/", strlen("dev2ser/res/"));
        memcpy(topic + strlen("dev2ser/res/"), dev_did ,strlen(dev_did));
        topic[strlen("dev2ser/res/") + strlen(dev_did)] = '\0';
        topiclen = strlen(topic);
    }

#if 0
    else if(rdata[7] == 0x22)
    {
        memcpy(topic, "dev2ser/req/", strlen("dev2ser/req/"));
        memcpy(topic + strlen("dev2ser/req/"), dev_did ,strlen(dev_did));
        topic[strlen("dev2ser/req/") + strlen(dev_did)] = '\0';
        topiclen = strlen(topic);
    }
#endif

    else if(rdata[7] == 0x24)
    {
        memcpy(topic, "dev2ser/noack/", strlen("dev2ser/noack/"));
        memcpy(topic + strlen("dev2ser/noack/"), dev_did ,strlen(dev_did));
        topic[strlen("dev2ser/noack/") + strlen(dev_did)] = '\0';
        topiclen = strlen(topic);
    }

    if(qos == 1) {
        qos_size = 2; // 2 bytes for QoS
        qos_flag = MQTT_QOS1_FLAG;
    }
    else if(qos == 2) {
        qos_size = 2; // 2 bytes for QoS
        qos_flag = MQTT_QOS2_FLAG;
    }

    var_header_len = topiclen + 2 + qos_size;
    var_header = (unsigned char *)malloc(var_header_len);
    if(var_header == NULL)
    {
        log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
        return -1;
    }
    var_header[0] = topiclen >> 8;
    var_header[1] = topiclen & 0xFF;
    memcpy(var_header + 2, topic, topiclen);

    if(qos_size) 
    {
        var_header[topiclen+2] = broker->seq>>8;
        var_header[topiclen+3] = broker->seq&0xFF;
        if(message_id) 
        { // Returning message id
            *message_id = broker->seq;
        }
        broker->seq++;
    }

    remain_len = var_header_len + rdata_len;
    sendvarc = Tran2varc(remain_len);
    fixed_header_size += sendvarc.varcbty;
    fixed_header_len = fixed_header_size;
    fixed_header = (unsigned char *)malloc(fixed_header_len);
    if(fixed_header == NULL)
    {
        log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
        return -1;
    }
    
    fixed_header[0] = MQTT_MSG_PUBLISH | qos_flag;
    memcpy(fixed_header + 1, sendvarc.var, sendvarc.varcbty);
    tdata_len = fixed_header_len + var_header_len + rdata_len;
    tdata = (unsigned char *)malloc(tdata_len);
    if(tdata == NULL)
    {
        log_printf(LOG_WARNING"[%s]malloc error\n",__FUNCTION__);
        return -1;
    }

    memcpy(tdata, fixed_header, fixed_header_len);
    memcpy(tdata + fixed_header_len, var_header, var_header_len);
    memcpy(tdata + fixed_header_len + var_header_len, rdata, rdata_len);

    ret = broker->mqtt_send(broker->socket_fd, tdata, tdata_len);
    if(ret < tdata_len) 
    {      
        free(fixed_header);
        free(var_header);
        free(tdata);
        return -1;
    } 
    free(fixed_header);
    free(var_header);
    free(tdata);
    return 1;
}

int cattsoft_uart_data_handle(unsigned char *rdata, char *dev_did, unsigned int rdata_len)
{
    return mqtt_publish_with_qos(&mqtt_broker, rdata, rdata_len, dev_did, 1, &mqtt_message_id);                 //qos:1
}


int mqtt_ping(int socket_fd)
{
    int ret;
    unsigned char ping_packet[] = {MQTT_MSG_PINGREQ, 0x00};
    ret = send(socket_fd, ping_packet, sizeof(ping_packet), 0);
    if(ret > 0)
    {
        log_printf(LOG_DEBUG"[%s]mqtt heartbeat send\n",__FUNCTION__);
        return 1;
    }
    return -1;
}

int cattsoft_m2m_heartbeat_request(int socket_fd)
{
    return mqtt_ping(socket_fd);
}

extern TimerHandle_t dataresp_timer;

void cattsoft_dispatch_publish_packet(int socket_fd, unsigned char *rdata, unsigned int rdata_len)
{
	unsigned int ret;
    unsigned char topic[128];
    unsigned int uart_packet_len;
    unsigned char uart_packet[128];
    unsigned int topiclen;
    unsigned char *payload;
    unsigned int payload_len;
	unsigned char tx_buff[4] = {0x40,0x02,0x00,0x00};

    memset(topic, 0, 128);
    memset(uart_packet, 0, 128);

    topiclen = mqtt_parse_pub_topic(rdata, topic);
    topic[topiclen] = '\0';
    payload_len = mqtt_parse_publish_msg(rdata, &payload); 

    printf(LOG_INFO"[%s]cloud data from topic :%s\n",__FUNCTION__,topic);

    print_hex(payload, payload_len);
    if(strncmp((const char*)topic,"ser2dev/req/",strlen("ser2dev/req/"))==0)
    {
    	tx_buff[2] = rdata[34];
		tx_buff[3] = rdata[35];
		ret = send(socket_fd, tx_buff, 4, 0);
		if(ret > 0)
		{
			printf("---->NET:\n");
			print_hex(tx_buff, 4);
		}
        uart_packet_len = data_from_m2m_parse(payload, uart_packet, payload_len);
		uart_packet[uart_packet_len - 1] = get_checksum(&uart_packet[1], uart_packet_len - 2);
        uart_data_send(uart_packet, uart_packet_len);
		//xTimerStart(dataresp_timer, 0);
    }

    else if(strncmp((const char*)topic,"ser2dev/res/",strlen("ser2dev/res/"))==0)
    {
        uart_packet_len = data_from_m2m_parse(payload, uart_packet, payload_len);
		uart_packet[uart_packet_len - 1] = get_checksum(&uart_packet[1], uart_packet_len - 2);
        uart_data_send(uart_packet, uart_packet_len);
    }
}
