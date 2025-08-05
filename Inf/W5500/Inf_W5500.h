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
//连接状态
typedef enum{
    CONNECTED, //连接
    NO_CONNECT //未连接
} ConnectionStatus;

//接收数据之后的处理函数
typedef void (*MQTT_Receive_Callback) (uint8_t* , uint16_t );

/**
 * @brief 初始化mqtt
 * 
 */
void Inf_MQTT_Init(void);

/**
 * @brief 注册接收数据后的处理函数
 * 
 * @param cb 
 */
void Inf_MQTT_RegisterReceiveCallback(MQTT_Receive_Callback cb);

/**
 * @brief 向Topic发送数据
 * 
 * @param datas 
 * @param len 
 */
void Inf_MQTT_Transmit(uint8_t* topicName, uint8_t* datas, uint16_t len);

/**
 * @brief 发送心跳
 * 
 */
void Inf_MQTT_Keepalive(void);
#endif
