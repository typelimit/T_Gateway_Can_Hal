#include "App_Task.h"

// 启动任务相关
#define START_TASK_NAME "start"
#define START_TASK_STACK_DEPTH 256
#define START_TASK_PRIORITY 9

// MQTT心跳任务相关
#define MQTT_KEEPALIVE_TASK_NAME "keepalive"
#define MQTT_KEEPALIVE_STACK_DEPTH 256
#define MQTT_KEEPALIVE_PRIORITY 7
TaskHandle_t mqttKeepaliveHandle;

// can接收任务相关
#define CAN_TASK_NAME "can"
#define CAN_STACK_DEPTH 256
#define CAN_PRIORITY 7
TaskHandle_t canHandle;

// 启动任务执行函数
void App_Task_StartFunction(void *args);
// MQTT心跳任务执行函数
void App_Task_MQTTKeepaliveFunction(void *args);
// can接收任务执行函数
void App_Task_CANFunction(void *args);

void App_Task_Start(void)
{

    // 创建启动任务
    xTaskCreate(App_Task_StartFunction, // 执行函数
                START_TASK_NAME,        // 任务名称
                START_TASK_STACK_DEPTH, // 堆栈大小
                NULL,
                START_TASK_PRIORITY,
                NULL);

    // 启动调度器
    vTaskStartScheduler();
}

/**
 * @brief 处理接受的MQTT数据
 *
 * @param datas
 * @param len
 */
void App_Task_ReceiveMQTT(uint8_t *datas, uint16_t len)
{

    debug_println("接收数据: %.*s", len, datas);

    // 解析数据
    cJSON *json = cJSON_ParseWithLength((char *)datas, len);

    if (json == NULL)
    {

        debug_println("json解析出错..");
        cJSON_Delete(json);
        return;
    }
    // 解析字段
    cJSON *id = cJSON_GetObjectItemCaseSensitive(json, "id");
    cJSON *connectType = cJSON_GetObjectItemCaseSensitive(json, "connectType");

    cJSON *targetSpeed = cJSON_GetObjectItemCaseSensitive(json, "targetSpeed");
    cJSON *motorStatus = cJSON_GetObjectItemCaseSensitive(json, "motorStatus");
    // 校验id和connectType是否存在
    if (id == NULL || connectType == NULL || targetSpeed == NULL || motorStatus == NULL)
    {
        debug_println("缺少必须要的参数..");
        cJSON_Delete(json);
        return;
    }

    // 校验id和connectType的值的类型
    if (cJSON_IsNumber(id) == 0 || cJSON_IsNumber(motorStatus) == 0 || cJSON_IsString(connectType) == 0 || cJSON_IsNumber(targetSpeed) == 0)
    {
        debug_println("数据类型出错...");
        cJSON_Delete(json);
        return;
    }

    // 判断使用哪种传输协议
    if (strcmp(connectType->valuestring, "can") == 0)
    {

        uint8_t datas[8] = {0};
        datas[0] = 0x01; // 功能码 0x01-控制 0x02-查询
        datas[1] = motorStatus->valueint;
        datas[2] = targetSpeed->valueint >> 8;
        datas[3] = targetSpeed->valueint & 0xFF;
        // 发送数据控制电机
        Driver_Can_Transmit(id->valueint, datas, 8);
        HAL_Delay(1000);
        datas[0] = 0x02;
        // 发送查询指令
        Driver_Can_Transmit(id->valueint,datas, 8 );
    }

    cJSON_Delete(json);
}

/**
 * @brief 启动任务执行函数
 *
 * @param args
 */
void App_Task_StartFunction(void *args)
{
    debug_println("启动任务开始调度...");
    debug_println("MQTT开始初始化....");
    // 注册收到数据之后的处理函数
    Inf_MQTT_RegisterReceiveCallback(App_Task_ReceiveMQTT);
    // 初始化W5500
    Inf_MQTT_Init();

    // 初始化Can
    Driver_Can_Init();

    // 创建MQTT心跳任务
    xTaskCreate(App_Task_MQTTKeepaliveFunction, MQTT_KEEPALIVE_TASK_NAME, MQTT_KEEPALIVE_STACK_DEPTH, NULL, MQTT_KEEPALIVE_PRIORITY, &mqttKeepaliveHandle);
    // can任务
    xTaskCreate(App_Task_CANFunction, CAN_TASK_NAME, CAN_STACK_DEPTH, NULL, CAN_PRIORITY, &canHandle);

    vTaskDelete(NULL);
}

// MQTT心跳任务执行函数
void App_Task_MQTTKeepaliveFunction(void *args)
{
    debug_println("MQTT心跳任务开始执行");

    TickType_t tick = xTaskGetTickCount();
    while (1)
    {
        Inf_MQTT_Keepalive();

        vTaskDelayUntil(&tick, 100);
    }
}

// can接收任务执行函数
void App_Task_CANFunction(void *args)
{
    vTaskDelay(100);
    debug_println("can接受任务开始执行...");

    uint8_t datas[8] = {0};
    uint8_t len = 0;
    uint16_t stdid = 0;
    while (1)
    {

        Driver_Can_Receive(datas, &len, &stdid);

        if (len > 0)
        {

            debug_println("收到电机数据:%.*s", len, datas);
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