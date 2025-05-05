/******************************************************************************
 *                          物联网FreeRTOS模板程序
 * 
 * 一、使用前需修改以下配置：
 * 
 * 1. onenet.c 文件中：
 *    #define PROID         "你的产品ID"
 *    #define ACCESS_KEY    "你的access_key"
 *    #define DEVICE_NAME   "你的设备名称"
 * 
 * 2. esp8266.c 文件中：
 *    #define ESP8266_WIFI_INFO "AT+CWJAP=\"你的WiFi名称\",\"你的WiFi密码\"\r\n"
 * 
 * 
 * 二、硬件连接说明：
 * 
 * 1. ESP8266模块：
 *    VCC  -> 3.3V
 *    GND  -> GND
 *    RX   -> PA2
 *    TX   -> PA3
 * 
 * 2. LED指示灯：
 *    正极 -> PC13
 *    负极 -> GND
 * 
 * 3. OLED显示屏：
 *    SCL -> PB6
 *    SDA -> PB7
 * 
 * 4. DHT11温湿度传感器：
 *    DATA -> PB0
 * 
 ******************************************************************************/

#include "main.h"

TaskHandle_t start_task_handler;	// 开始任务句柄

/* 硬件设备 */
TimerDev_t timer_delay = {.config = {TIM4, 71, 49999, NULL}}; // 用于FreeRTOS下的微秒级延时，计数周期1us
LEDDev_t led = {.config = {GPIOC, GPIO_Pin_13, GPIO_LEVEL_LOW}};
OLEDDev_t oled = {.config = {GPIOB, GPIO_Pin_6, GPIOB, GPIO_Pin_7}};
DHT11Dev_t dht11 = {.config = {GPIOB, GPIO_Pin_0}};
ESP8266Dev_t esp8266;
UARTDev_t debug = {.config = {
    .uartx = USART1, 
    .baud = 115200,
    .tx_port = GPIOA, 
    .tx_pin = GPIO_Pin_9, 
    .rx_port = GPIOA, 
    .rx_pin = GPIO_Pin_10
}};

/* 开始任务 */
void task_start(void *pvParameters)
{
    taskENTER_CRITICAL();               // 进入临界区   
        
    /* 创建任务 */
    xTaskCreate(task_led, "task_led", LED_STK_SIZE, NULL, LED_TASK_PRIO, &led_task_handler);
    xTaskCreate(task_net, "task_net", NET_STK_SIZE, NULL, NET_TASK_PRIO, &net_task_handler);
    xTaskCreate(task_sensor, "task_sensor", SENSOR_STK_SIZE, NULL, SENSOR_TASK_PRIO, &sensor_task_handler);
				
    vTaskDelete(start_task_handler);    // 删除开始任务
    taskEXIT_CRITICAL();                // 退出临界区
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);     // 设置系统中断优先级分组4
    
    /* 硬件设备初始化 */
	delay_init(&timer_delay);
    uart_init(&debug);
	led_init(&led);
    oled_init(&oled);
    debug.send_string(&debug, " \r\nHardware init OK\r\n");
    oled.show_string(&oled, 0, 10, "Hardware init OK", OLED_6X8);
    oled.update(&oled);
    dht11_init(&dht11);
    
    /* ESP8266初始化 */
    debug.send_string(&debug, "\r\nESP8266 init...\r\n");
	esp8266_init(&esp8266);					//初始化ESP8266
    debug.send_string(&debug, "ESP8266 init OK\r\n");
    oled.show_string(&oled, 0, 20, "ESP8266 init OK", OLED_6X8);
    oled.update(&oled);
	
    /* 连接MQTT服务器 */
    debug.send_string(&debug, "\r\nConnect MQTTs Server...\r\n");
    while (esp8266.send_cmd(&esp8266, ESP8266_ONENET_INFO, "CONNECT"))
		delay_ms(500);
    debug.send_string(&debug, "Connect MQTT Server Success\r\n");
    oled.show_string(&oled, 0, 30, "MQTT connect OK", OLED_6X8);
    oled.update(&oled);
	
    /* OneNET设备登录 */
    debug.send_string(&debug, "\r\nOneNET device login...\r\n");
    while(OneNet_DevLink())			//接入OneNET
    {
        esp8266.send_cmd(&esp8266, ESP8266_ONENET_INFO, "CONNECT");
        delay_ms(500);
    }
		
    debug.send_string(&debug, "OneNET device login OK\r\n");
    oled.show_string(&oled, 0, 40, "OneNET device login", OLED_6X8);
    oled.update(&oled);
    
    /* OneNET订阅 */
    OneNET_Subscribe();
    
    led.on(&led);
    
    oled.clear(&oled);
    oled.show_string(&oled, 0, 10, "OK", OLED_6X8);
    oled.update(&oled);
	
    /* 创建开始任务 */
    xTaskCreate(task_start, "task_start", START_STK_SIZE, NULL, START_TASK_PRIO, &start_task_handler);        
                    
    vTaskStartScheduler();          // 开启任务调度
}
