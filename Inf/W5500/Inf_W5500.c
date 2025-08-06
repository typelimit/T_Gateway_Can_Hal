#include "Inf_W5500.h"

// mac地址[不能和其他人一样]
uint8_t mac[6] = {0xC4, 0xEF, 0xBB, 0x98, 0xEC, 0xA2};
// 网关[必须和电脑一样]
uint8_t gw[4] = {192, 168, 56, 1};
// 子网掩码[255.255.255.0]
uint8_t sub[4] = {255, 255, 255, 0};
// ip地址[不能和其他人一样][必须和电脑在同一个网段]
uint8_t ip[4] = {192, 168, 56, 111};

MQTTClient c = {0};
Network n = {0};

// socket编号
#define SOCKET_NUM 0

// mqtt服务器ip——————————现在不能用本地的mqtt服务器，而是用云端免费的那个
static uint8_t MQTT_SERVER_IP[4] = {166, 117, 39, 65};
#define MQTT_SERVER_PORT 1883
#define SUB_TOPIC "zhangziheng"

// 发送缓冲和接收缓冲
#define ETHERNET_BUF_MAX_SIZE 1024
uint8_t ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {0};
static uint8_t mqtt_send_ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {0};
static uint8_t mqtt_recv_ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {0};

MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

MQTT_Receive_Callback callback;

MQTTMessage pubmessage = {
    .qos = QOS0,
    .retained = 0,
    .dup = 0,
    .id = 0,
};

extern void vPortEnterCritical(void);
extern void vPortExitCritical(void);

static void Inf_W5500_RegisterCallback(void)
{
    // 注册SPI的片选和取消片选函数
    reg_wizchip_cs_cbfunc(Driver_SPI_Start, Driver_SPI_Stop);

    // 注册SPI的读写函数
    reg_wizchip_spi_cbfunc(Driver_SPI_ReadByte, Driver_SPI_WriteByte);

    // 注册临界区函数
    reg_wizchip_cris_cbfunc(vPortEnterCritical, vPortExitCritical);
}

/**
 * @brief 复位
 *
 */
static void Inf_W5500_Reset(void)
{
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET);
    vTaskDelay(400);
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);
    vTaskDelay(100);
}

/**
 * @brief 配置网络环境
 *
 */
static void Inf_W5500_ConfigNetwork(void)
{
    // 配置mac地址[每个人要不一样]
    setSHAR(mac);
    // 配置网关
    setGAR(gw);
    // 设置子网掩码
    setSUBR(sub);
    // 设置IP
    setSIPR(ip);

    // 获取设置的ip\gw\sub
    getGAR(gw);
    getSUBR(sub);
    getSIPR(ip);

    debug_println(" IP          : %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    debug_println(" Subnet Mask : %d.%d.%d.%d\r\n", sub[0], sub[1], sub[2], sub[3]);
    debug_println(" Gateway     : %d.%d.%d.%d\r\n", gw[0], gw[1], gw[2], gw[3]);
}
/**
 * @brief 初始化W5500
 *
 */
void Inf_W5500_Init(void)
{

    // 注册SPI的回调函数给W5500
    Inf_W5500_RegisterCallback();

    // 复位W5500
    Inf_W5500_Reset();

    // 配置网络环境
    Inf_W5500_ConfigNetwork();
}
/**
 * @brief 收到消息之后的处理函数
 *
 * @param args
 */
static void Inf_W5500_ReceiveCallback(MessageData *md)
{
    debug_println("%.*s", (int)md->topicName->lenstring.len, md->topicName->lenstring.data);

    // 处理数据
    callback((uint8_t *)md->message->payload, md->message->payloadlen);
}
/**
 * @brief 初始化mqtt
 *
 */
void Inf_MQTT_Init(void)
{
    // ...原有内容...
    Inf_W5500_Init();
    NewNetwork(&n, SOCKET_NUM);
    ConnectNetwork(&n, MQTT_SERVER_IP, MQTT_SERVER_PORT); // 传入uint8_t[4]
    MQTTClientInit(&c, &n, 1000, mqtt_send_ethernet_buf, ETHERNET_BUF_MAX_SIZE, mqtt_recv_ethernet_buf, ETHERNET_BUF_MAX_SIZE);

    data.willFlag = 0;    // 不使用遗嘱消息
    data.MQTTVersion = 4; // MQTT版本
    data.clientID.cstring = "user1";
    data.keepAliveInterval = 0; // 心跳间隔
    data.cleansession = 1;      // 是否清除回话
    debug_println("准备连接服务器...");
    // 连接MQTT服务器
    uint8_t ret = MQTTConnect(&c, &data);

    printf("服务器连接:%s\r\n\r\n", ret == SUCCESSS ? "success" : "failed");
    printf("准备订阅[%s]\r\n", SUB_TOPIC);
    ret = MQTTSubscribe(&c, SUB_TOPIC, QOS0, Inf_W5500_ReceiveCallback);
    printf("%s\r\n", ret == SUCCESSS ? "topic订阅成功" : "topic订阅失败");

    // --- 这里是新增的“自报家门”代码 ---
    if (ret == SUCCESSS)
    {
        // 构造hello JSON消息
        const char *hello_msg = "{\"msg\":\"hello,I am a gateway\"}";
        // 发送到zhangziheng topic
        Inf_MQTT_Transmit((uint8_t *)SUB_TOPIC, (uint8_t *)hello_msg, strlen(hello_msg));
        debug_println("网关已上线，自报家门：%s", hello_msg);
    }
}

/**
 * @brief 注册接收数据后的处理函数
 *
 * @param cb
 */
void Inf_MQTT_RegisterReceiveCallback(MQTT_Receive_Callback cb)
{
    callback = cb;
}

/**
 * @brief 向Topic发送数据
 *
 * @param datas
 * @param len
 */
void Inf_MQTT_Transmit(uint8_t *topicName, uint8_t *datas, uint16_t len)
{
    pubmessage.qos = QOS0;
    pubmessage.payload = datas;
    pubmessage.payloadlen = len;
    // 发布消息到TOPIC
    MQTTPublish(&c, (char *)topicName, &pubmessage);
}

/**
 * @brief 发送心跳
 *
 */
void Inf_MQTT_Keepalive(void)
{
    MQTTYield(&c, 30);
}
