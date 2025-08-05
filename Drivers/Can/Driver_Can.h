#ifndef __DRIVER_CAN_H__
#define __DRIVER_CAN_H__
#include "can.h"

void Driver_Can_Init(void);

/**
 * @brief ��������
 * 
 * @param id 
 * @param datas 
 * @param len 
 */
void Driver_Can_Transmit( uint16_t id, uint8_t* datas, uint8_t len );

/**
 * @brief ��������
 * 
 * @param datas 
 * @param len 
 */
void Driver_Can_Receive( uint8_t* datas, uint8_t* len , uint16_t* id);

#endif
