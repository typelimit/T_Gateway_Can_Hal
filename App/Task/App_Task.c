#include "App_Task.h"

// �����������
#define START_TASK_NAME "start"
#define START_TASK_STACK_DEPTH 256
#define START_TASK_PRIORITY 9

// MQTT�����������
#define MQTT_KEEPALIVE_TASK_NAME "keepalive"
#define MQTT_KEEPALIVE_STACK_DEPTH 256
#define MQTT_KEEPALIVE_PRIORITY 7
TaskHandle_t mqttKeepaliveHandle;

// can�����������
#define CAN_TASK_NAME "can"
#define CAN_STACK_DEPTH 256
#define CAN_PRIORITY 7
TaskHandle_t canHandle;

// ��������ִ�к���
void App_Task_StartFunction(void *args);
// MQTT��������ִ�к���
void App_Task_MQTTKeepaliveFunction(void *args);
// can��������ִ�к���
void App_Task_CANFunction(void *args);

void App_Task_Start(void)
{

    // ������������
    xTaskCreate(App_Task_StartFunction, // ִ�к���
                START_TASK_NAME,        // ��������
                START_TASK_STACK_DEPTH, // ��ջ��С
                NULL,
                START_TASK_PRIORITY,
                NULL);

    // ����������
    vTaskStartScheduler();
}

/**
 * @brief ������ܵ�MQTT����
 *
 * @param datas
 * @param len
 */
void App_Task_ReceiveMQTT(uint8_t *datas, uint16_t len)
{

    debug_println("��������: %.*s", len, datas);

    // ��������
    cJSON *json = cJSON_ParseWithLength((char *)datas, len);

    if (json == NULL)
    {

        debug_println("json��������..");
        cJSON_Delete(json);
        return;
    }
    // �����ֶ�
    cJSON *id = cJSON_GetObjectItemCaseSensitive(json, "id");
    cJSON *connectType = cJSON_GetObjectItemCaseSensitive(json, "connectType");

    cJSON *targetSpeed = cJSON_GetObjectItemCaseSensitive(json, "targetSpeed");
    cJSON *motorStatus = cJSON_GetObjectItemCaseSensitive(json, "motorStatus");
    // У��id��connectType�Ƿ����
    if (id == NULL || connectType == NULL || targetSpeed == NULL || motorStatus == NULL)
    {
        debug_println("ȱ�ٱ���Ҫ�Ĳ���..");
        cJSON_Delete(json);
        return;
    }

    // У��id��connectType��ֵ������
    if (cJSON_IsNumber(id) == 0 || cJSON_IsNumber(motorStatus) == 0 || cJSON_IsString(connectType) == 0 || cJSON_IsNumber(targetSpeed) == 0)
    {
        debug_println("�������ͳ���...");
        cJSON_Delete(json);
        return;
    }

    // �ж�ʹ�����ִ���Э��
    if (strcmp(connectType->valuestring, "can") == 0)
    {

        uint8_t datas[8] = {0};
        datas[0] = 0x01; // ������ 0x01-���� 0x02-��ѯ
        datas[1] = motorStatus->valueint;
        datas[2] = targetSpeed->valueint >> 8;
        datas[3] = targetSpeed->valueint & 0xFF;
        // �������ݿ��Ƶ��
        Driver_Can_Transmit(id->valueint, datas, 8);
        HAL_Delay(1000);
        datas[0] = 0x02;
        // ���Ͳ�ѯָ��
        Driver_Can_Transmit(id->valueint,datas, 8 );
    }

    cJSON_Delete(json);
}

/**
 * @brief ��������ִ�к���
 *
 * @param args
 */
void App_Task_StartFunction(void *args)
{
    debug_println("��������ʼ����...");
    debug_println("MQTT��ʼ��ʼ��....");
    // ע���յ�����֮��Ĵ�����
    Inf_MQTT_RegisterReceiveCallback(App_Task_ReceiveMQTT);
    // ��ʼ��W5500
    Inf_MQTT_Init();

    // ��ʼ��Can
    Driver_Can_Init();

    // ����MQTT��������
    xTaskCreate(App_Task_MQTTKeepaliveFunction, MQTT_KEEPALIVE_TASK_NAME, MQTT_KEEPALIVE_STACK_DEPTH, NULL, MQTT_KEEPALIVE_PRIORITY, &mqttKeepaliveHandle);
    // can����
    xTaskCreate(App_Task_CANFunction, CAN_TASK_NAME, CAN_STACK_DEPTH, NULL, CAN_PRIORITY, &canHandle);

    vTaskDelete(NULL);
}

// MQTT��������ִ�к���
void App_Task_MQTTKeepaliveFunction(void *args)
{
    debug_println("MQTT��������ʼִ��");

    TickType_t tick = xTaskGetTickCount();
    while (1)
    {
        Inf_MQTT_Keepalive();

        vTaskDelayUntil(&tick, 100);
    }
}

// can��������ִ�к���
void App_Task_CANFunction(void *args)
{
    vTaskDelay(100);
    debug_println("can��������ʼִ��...");

    uint8_t datas[8] = {0};
    uint8_t len = 0;
    uint16_t stdid = 0;
    while (1)
    {

        Driver_Can_Receive(datas, &len, &stdid);

        if (len > 0)
        {

            debug_println("�յ��������:%.*s", len, datas);
            //{"id": 5, "dir":"forward", "speed": 120}
            cJSON *root = cJSON_CreateObject();
            cJSON_AddNumberToObject(root, "id", stdid * 1.0);
            cJSON_AddStringToObject(root, "dir", datas[1] == 0 ? "backward" : "forward");
            cJSON_AddNumberToObject(root, "speed", ((datas[2] << 8) | datas[3]) * 1.0);
            char *json = cJSON_PrintUnformatted(root);
            Inf_MQTT_Transmit("response", json, strlen(json));
            cJSON_Delete(root);
        }
    }
}