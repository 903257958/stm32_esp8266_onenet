#include <string.h>
#include "stm32f10x.h"

//网络协议层
#include "onenet.h"

//网络设备
#include "esp8266.h"

//硬件驱动
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "oled.h"

#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"mqtts.heclouds.com\",1883\r\n"

LEDDev_t led = {.config = {GPIOC, GPIO_Pin_13, GPIO_LEVEL_LOW}};
OLEDDev_t oled = {.config = {GPIOB, GPIO_Pin_6, GPIOB, GPIO_Pin_7}};

int main(void)
{
	unsigned short timeCount = 0;	//发送间隔变量
	unsigned char *dataPtr = NULL;
	
    /* 硬件初始化 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断控制器分组设置
	Delay_Init();									//systick初始化
	Usart1_Init(115200);							//串口1，打印信息用
	Usart2_Init(115200);							//串口2，驱动ESP8266用
	led_init(&led);
    oled_init(&oled);
	UsartPrintf(USART_DEBUG, " \r\nHardware init OK\r\n");
    oled.show_string(&oled, 0, 10, "Hardware init OK", OLED_6X8);
    oled.update(&oled);
	
    /* ESP8266初始化 */
    UsartPrintf(USART_DEBUG, "\r\nESP8266 init...\r\n");
	ESP8266_Init();					//初始化ESP8266
    UsartPrintf(USART_DEBUG, "ESP8266 init OK\r\n");
    oled.show_string(&oled, 0, 20, "ESP8266 init OK", OLED_6X8);
    oled.update(&oled);
	
    /* 连接MQTT服务器 */
	UsartPrintf(USART_DEBUG, "\r\nConnect MQTTs Server...\r\n");
	while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
		DelayXms(500);
    UsartPrintf(USART_DEBUG, "Connect MQTT Server Success\r\n");
    oled.show_string(&oled, 0, 30, "MQTT connect OK", OLED_6X8);
    oled.update(&oled);
	
    /* OneNET设备登录 */
	UsartPrintf(USART_DEBUG, "\r\nOneNET device login...\r\n");
    while(OneNet_DevLink())			//接入OneNET
    {
        ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT");
        DelayXms(500);
    }
		
    UsartPrintf(USART_DEBUG, "OneNET device login OK\r\n");
    oled.show_string(&oled, 0, 40, "OneNET device login OK", OLED_6X8);
    oled.update(&oled);
    
    /* OneNET订阅 */
    OneNET_Subscribe();
    
    led.on(&led);
	
	while(1)
	{
		/* 上发数据 */
        if(++timeCount >= 500)									//发送间隔5s
		{
//			SHT20_GetValue();
			
			UsartPrintf(USART_DEBUG, "OneNet_SendData\r\n");
			OneNet_SendData();									//发送数据
			
			timeCount = 0;
			ESP8266_Clear();
		}
		
        /* 接收数据 */
		dataPtr = ESP8266_GetIPD(0);
		if(dataPtr != NULL)
			OneNet_RevPro(dataPtr);
		
		DelayXms(10);
	}
}
