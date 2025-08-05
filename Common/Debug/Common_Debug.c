#include "Common_Debug.h"

void Common_Debug_Init(void){
    //Driver_Uart_Init();
}

int fputc(int c, FILE * f){
    HAL_UART_Transmit(&huart1,(uint8_t*)&c,1,2000);
    return c;
} 
