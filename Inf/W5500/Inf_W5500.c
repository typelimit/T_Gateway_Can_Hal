#include "Inf_W5500.h"

// mac��ַ[���ܺ�������һ��]
uint8_t mac[6] = {0xC4, 0xEF, 0xBB, 0x98, 0xEC, 0xA2};
// ����[����͵���һ��]
uint8_t gw[4] = {192, 168, 56, 1};
// ��������[255.255.255.0]
uint8_t sub[4] = {255, 255, 255, 0};
// ip��ַ[���ܺ�������һ��][����͵�����ͬһ������]
uint8_t ip[4] = {192, 168, 56, 111};

MQTTClient c = {0};
Network n = {0};

// socket���
#define SOCKET_NUM 0

// mqtt������ip�����������������������ڲ����ñ��ص�mqtt���������������ƶ���ѵ��Ǹ�
static uint8_t MQTT_SERVER_IP[4] = {166, 117, 39, 65};
#define MQTT_SERVER_PORT 1883
#define SUB_TOPIC "zhangziheng"

// ���ͻ���ͽ��ջ���
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
    // ע��SPI��Ƭѡ��ȡ��Ƭѡ����
    reg_wizchip_cs_cbfunc(Driver_SPI_Start, Driver_SPI_Stop);

    // ע��SPI�Ķ�д����
    reg_wizchip_spi_cbfunc(Driver_SPI_ReadByte, Driver_SPI_WriteByte);

    // ע���ٽ�������
    reg_wizchip_cris_cbfunc(vPortEnterCritical, vPortExitCritical);
}

/**
 * @brief ��λ
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
 * @brief �������绷��
 *
 */
static void Inf_W5500_ConfigNetwork(void)
{
    // ����mac��ַ[ÿ����Ҫ��һ��]
    setSHAR(mac);
    // ��������
    setGAR(gw);
    // ������������
    setSUBR(sub);
    // ����IP
    setSIPR(ip);

    // ��ȡ���õ�ip\gw\sub
    getGAR(gw);
    getSUBR(sub);
    getSIPR(ip);

    debug_println(" IP          : %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    debug_println(" Subnet Mask : %d.%d.%d.%d\r\n", sub[0], sub[1], sub[2], sub[3]);
    debug_println(" Gateway     : %d.%d.%d.%d\r\n", gw[0], gw[1], gw[2], gw[3]);
}
/**
 * @brief ��ʼ��W5500
 *
 */
void Inf_W5500_Init(void)
{

    // ע��SPI�Ļص�������W5500
    Inf_W5500_RegisterCallback();

    // ��λW5500
    Inf_W5500_Reset();

    // �������绷��
    Inf_W5500_ConfigNetwork();
}
/**
 * @brief �յ���Ϣ֮��Ĵ�����
 *
 * @param args
 */
static void Inf_W5500_ReceiveCallback(MessageData *md)
{
    debug_println("%.*s", (int)md->topicName->lenstring.len, md->topicName->lenstring.data);

    // ��������
    callback((uint8_t *)md->message->payload, md->message->payloadlen);
}
/**
 * @brief ��ʼ��mqtt
 *
 */
void Inf_MQTT_Init(void)
{
    // ...ԭ������...
    Inf_W5500_Init();
    NewNetwork(&n, SOCKET_NUM);
    ConnectNetwork(&n, MQTT_SERVER_IP, MQTT_SERVER_PORT); // ����uint8_t[4]
    MQTTClientInit(&c, &n, 1000, mqtt_send_ethernet_buf, ETHERNET_BUF_MAX_SIZE, mqtt_recv_ethernet_buf, ETHERNET_BUF_MAX_SIZE);

    data.willFlag = 0;    // ��ʹ��������Ϣ
    data.MQTTVersion = 4; // MQTT�汾
    data.clientID.cstring = "user1";
    data.keepAliveInterval = 0; // �������
    data.cleansession = 1;      // �Ƿ�����ػ�
    debug_println("׼�����ӷ�����...");
    // ����MQTT������
    uint8_t ret = MQTTConnect(&c, &data);

    printf("����������:%s\r\n\r\n", ret == SUCCESSS ? "success" : "failed");
    printf("׼������[%s]\r\n", SUB_TOPIC);
    ret = MQTTSubscribe(&c, SUB_TOPIC, QOS0, Inf_W5500_ReceiveCallback);
    printf("%s\r\n", ret == SUCCESSS ? "topic���ĳɹ�" : "topic����ʧ��");

    // --- �����������ġ��Ա����š����� ---
    if (ret == SUCCESSS)
    {
        // ����hello JSON��Ϣ
        const char *hello_msg = "{\"msg\":\"hello,I am a gateway\"}";
        // ���͵�zhangziheng topic
        Inf_MQTT_Transmit((uint8_t *)SUB_TOPIC, (uint8_t *)hello_msg, strlen(hello_msg));
        debug_println("���������ߣ��Ա����ţ�%s", hello_msg);
    }
}

/**
 * @brief ע��������ݺ�Ĵ�����
 *
 * @param cb
 */
void Inf_MQTT_RegisterReceiveCallback(MQTT_Receive_Callback cb)
{
    callback = cb;
}

/**
 * @brief ��Topic��������
 *
 * @param datas
 * @param len
 */
void Inf_MQTT_Transmit(uint8_t *topicName, uint8_t *datas, uint16_t len)
{
    pubmessage.qos = QOS0;
    pubmessage.payload = datas;
    pubmessage.payloadlen = len;
    // ������Ϣ��TOPIC
    MQTTPublish(&c, (char *)topicName, &pubmessage);
}

/**
 * @brief ��������
 *
 */
void Inf_MQTT_Keepalive(void)
{
    MQTTYield(&c, 30);
}
