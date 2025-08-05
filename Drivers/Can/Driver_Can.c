#include "Driver_Can.h"

void Driver_Can_Init(void){

    //���ù�����
    CAN_FilterTypeDef filter;
    filter.FilterBank = 0;
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterIdHigh = 0;
    filter.FilterIdLow = 0;
    filter.FilterMaskIdHigh = 0x00;
    filter.FilterMaskIdLow= 0x00; //���е�������������ݶ�����
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterActivation = CAN_FILTER_ENABLE;

    HAL_CAN_ConfigFilter( &hcan, &filter );

    //����can
    HAL_CAN_Start(&hcan);
}

CAN_TxHeaderTypeDef  header;
/**
 * @brief ��������
 * 
 * @param id 
 * @param datas 
 * @param len 
 */
void Driver_Can_Transmit( uint16_t id, uint8_t* datas, uint8_t len ){

    //�ȴ����������
    while( HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0 );

    header.StdId = id;
    header.RTR = CAN_RTR_DATA;
    header.IDE = CAN_ID_STD;
    header.DLC = len;
    uint32_t box = 0;
    //��������
    HAL_CAN_AddTxMessage( &hcan, &header, datas, &box );
}

CAN_RxHeaderTypeDef rxHeader;
/**
 * @brief ��������
 * 
 * @param datas 
 * @param len 
 */
void Driver_Can_Receive( uint8_t* datas, uint8_t* len, uint16_t* id ){

    
    if( HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) > 0  ){


        HAL_CAN_GetRxMessage( &hcan, CAN_RX_FIFO0, &rxHeader, datas );

        *len = rxHeader.DLC;
        *id = rxHeader.StdId;
    }else{
        *len = 0;
        *id = 0;
    }
}
