/*************************************************************************
	> File Name: cattsoft_mqtt.h
	> Author: xuetong
	> Mail: 
	> Created Time: Mon 07 May 2018 02:24:20 PM CST
 ************************************************************************/
#define MQTT_USER_NAME_LEN 24
#define MQTT_PASSWORD_LEN 24


typedef struct{
	int socket_fd;
	int (*mqtt_send)(int socket_fd, unsigned char *data, unsigned int data_len);
	char clientid[50];
	char user_name[MQTT_USER_NAME_LEN];
	char password[MQTT_PASSWORD_LEN];

	unsigned char will_retain;
	unsigned char will_qos;
	unsigned char clean_session;

	unsigned short seq;
	unsigned short alive;
}t_mqtt_broke;

typedef struct _var{
	char var[4];
	int varcbty;
}varc;


#define MQTT_DUP_FLAG     (1<<3)
#define MQTT_QOS0_FLAG    (0<<1)
#define MQTT_QOS1_FLAG    (1<<1)
#define MQTT_QOS2_FLAG    (2<<1)

#define MQTT_RETAIN_FLAG  1

#define MQTT_CLEAN_SESSION  (1<<1)
#define MQTT_WILL_FLAG      (1<<2)
#define MQTT_WILL_RETAIN    (1<<5)
#define MQTT_USERNAME_FLAG  (1<<7)
#define MQTT_PASSWORD_FLAG  (1<<6)


#define MQTT_MSG_CONNECT       (1<<4)
#define MQTT_MSG_CONNACK       (2<<4)
#define MQTT_MSG_PUBLISH       (3<<4)
#define MQTT_MSG_PUBACK        (4<<4)
#define MQTT_MSG_PUBREC        (5<<4)
#define MQTT_MSG_PUBREL        (6<<4)
#define MQTT_MSG_PUBCOMP       (7<<4)
#define MQTT_MSG_SUBSCRIBE     (8<<4)
#define MQTT_MSG_SUBACK        (9<<4)
#define MQTT_MSG_UNSUBSCRIBE  (10<<4)
#define MQTT_MSG_UNSUBACK     (11<<4)
#define MQTT_MSG_PINGREQ      (12<<4)
#define MQTT_MSG_PINGRESP     (13<<4)
#define MQTT_MSG_DISCONNECT   (14<<4)

/** Extract the message type from buffer.
 * @param buffer Pointer to the packet.
 *
 * @return Message Type byte.
 */
#define MQTTParseMessageType(buffer) ( *buffer & 0xF0 )

/** Indicate if it is a duplicate packet.
 * @param buffer Pointer to the packet.
 *
 * @retval   0 Not duplicate.
 * @retval !=0 Duplicate.
 */
#define MQTTParseMessageDuplicate(buffer) ( *buffer & 0x08 )

/** Extract the message QoS level.
 * @param buffer Pointer to the packet.
 *
 * @return QoS Level (0, 1 or 2).
 */
#define MQTTParseMessageQos(buffer) ( (*buffer & 0x06) >> 1 )

/** Indicate if this packet has a retain flag.
 * @param buffer Pointer to the packet.
 *
 * @retval   0 Not duplicate.
 * @retval !=0 Duplicate.
 */
#define MQTTParseMessageRetain(buffer) ( *buffer & 0x01 )
