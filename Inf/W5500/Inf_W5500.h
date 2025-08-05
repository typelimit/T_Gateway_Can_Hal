#ifndef __INF_W5500_H__
#define __INF_W5500_H__
#include "spi.h"
#include "wizchip_conf.h"
#include "Common_Debug.h"
#include "socket.h"
#include "FreeRTOS.h"
#include "task.h"
#include "MQTTClient.h"
#include "mqtt_interface.h"
//����״̬
typedef enum{
    CONNECTED, //����
    NO_CONNECT //δ����
} ConnectionStatus;

//��������֮��Ĵ�����
typedef void (*MQTT_Receive_Callback) (uint8_t* , uint16_t );

/**
 * @brief ��ʼ��mqtt
 * 
 */
void Inf_MQTT_Init(void);

/**
 * @brief ע��������ݺ�Ĵ�����
 * 
 * @param cb 
 */
void Inf_MQTT_RegisterReceiveCallback(MQTT_Receive_Callback cb);

/**
 * @brief ��Topic��������
 * 
 * @param datas 
 * @param len 
 */
void Inf_MQTT_Transmit(uint8_t* topicName, uint8_t* datas, uint16_t len);

/**
 * @brief ��������
 * 
 */
void Inf_MQTT_Keepalive(void);
#endif
