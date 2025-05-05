#include "esp8266.h"
#include "delay.h"

/* WiFi信息 */
#define ESP8266_WIFI_INFO		"AT+CWJAP=\"shouji\",\"thxd156369\"\r\n"

/* ESP8266接收缓冲区 */
uint8_t g_rx_buf[512];
uint16_t g_cnt = 0;

/* 函数声明 */
static void __esp8266_uart_init(unsigned int baud);
static void __esp8266_uart_sned_string(USART_TypeDef *USARTx, unsigned char *str, unsigned short len);
static int __esp8266_wait_recv(void);
static int __esp8266_clear(ESP8266Dev_t *dev);
static int __esp8266_send_cmd(ESP8266Dev_t *dev, char *cmd, char *res);
static int __esp8266_send_data(ESP8266Dev_t *dev, unsigned char *data, unsigned short len);
static uint8_t *__esp8266_get_ipd(ESP8266Dev_t *dev, unsigned short timeout);

/******************************************************************************
 * @brief	ESP8266初始化
 * @param	dev	:	ESP8266Dev_t 结构体指针
 * @return	0, 表示成功, 其他值表示失败
 ******************************************************************************/
int esp8266_init(ESP8266Dev_t *dev)
{
	__esp8266_uart_init(115200);
	
	dev->init_flag = true;

	/* 函数指针赋值 */
	dev->clear = __esp8266_clear;
	dev->send_cmd = __esp8266_send_cmd;
	dev->send_data = __esp8266_send_data;
	dev->get_ipd = __esp8266_get_ipd;
	
	__esp8266_clear(dev);
	
	ESP8266_DEBUG("1. AT\r\n");
	while(__esp8266_send_cmd(dev, "AT\r\n", "OK"))
		delay_ms(500);
	
	ESP8266_DEBUG("2. CWMODE\r\n");
	while(__esp8266_send_cmd(dev, "AT+CWMODE=1\r\n", "OK"))
		delay_ms(500);
	
	ESP8266_DEBUG("3. AT+CWDHCP\r\n");
	while(__esp8266_send_cmd(dev, "AT+CWDHCP=1,1\r\n", "OK"))
		delay_ms(500);
	
	ESP8266_DEBUG("4. CWJAP\r\n");
	while(__esp8266_send_cmd(dev, ESP8266_WIFI_INFO, "GOT IP"))
		delay_ms(500);
	
	ESP8266_DEBUG("5. ESP8266 Init OK\r\n");

	return 0;
}

/******************************************************************************
 * @brief	ESP8266串口初始化
 * @param	baud	:	波特率
 * @return	无
 ******************************************************************************/
static void __esp8266_uart_init(unsigned int baud)
{
	GPIO_InitTypeDef gpio_initstruct;
	USART_InitTypeDef usart_initstruct;
	NVIC_InitTypeDef nvic_initstruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	gpio_initstruct.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_initstruct.GPIO_Pin = GPIO_Pin_2;
	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_initstruct);
	
	gpio_initstruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio_initstruct.GPIO_Pin = GPIO_Pin_3;
	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_initstruct);
	
	usart_initstruct.USART_BaudRate = baud;
	usart_initstruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//无硬件流控
	usart_initstruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						//接收和发送
	usart_initstruct.USART_Parity = USART_Parity_No;									//无校验
	usart_initstruct.USART_StopBits = USART_StopBits_1;								//1位停止位
	usart_initstruct.USART_WordLength = USART_WordLength_8b;							//8位数据位
	USART_Init(USART2, &usart_initstruct);
	
	USART_Cmd(USART2, ENABLE);														//使能串口
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);									//使能接收中断
	
	nvic_initstruct.NVIC_IRQChannel = USART2_IRQn;
	nvic_initstruct.NVIC_IRQChannelCmd = ENABLE;
	nvic_initstruct.NVIC_IRQChannelPreemptionPriority = 0;
	nvic_initstruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvic_initstruct);
}

/******************************************************************************
 * @brief	ESP8266串口发送数据
 * @param	USARTx	:	串口外设
 * @param	str		:	字符串
 * @param	len		:	长度
 * @return	无
 ******************************************************************************/
static void __esp8266_uart_sned_string(USART_TypeDef *USARTx, unsigned char *str, unsigned short len)
{
	unsigned short count = 0;
	
	for(; count < len; count++)
	{
		USART_SendData(USARTx, *str++);									//发送数据
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);		//等待发送完成
	}
}

/******************************************************************************
 * @brief	ESP8266等待接收完成（循环调用检测是否接收完成）
 * @param	无
 * @return	0, 表示成功, 其他值表示失败
 ******************************************************************************/
static int __esp8266_wait_recv(void)
{
	static uint16_t cnt_pre = 0;
	
	if(g_cnt == 0) 					// 如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;
		
	if(g_cnt == cnt_pre)			// 如果上一次的值和这次相同，则说明接收完毕
	{
		g_cnt = 0;					// 清0接收计数
			
		return REV_OK;				// 返回接收完成标志
	}
		
	cnt_pre = g_cnt;				// 置为相同
	
	return REV_WAIT;				// 返回接收未完成标志
}

/******************************************************************************
 * @brief	ESP8266清空缓存
 * @param	dev	:	ESP8266Dev_t 结构体指针
 * @return	0, 表示成功, 其他值表示失败
 ******************************************************************************/
static int __esp8266_clear(ESP8266Dev_t *dev)
{
	if (!dev || !dev->init_flag)
		return -1;

	memset(g_rx_buf, 0, sizeof(g_rx_buf));
	g_cnt = 0;

	return 0;
}

/******************************************************************************
 * @brief	ESP8266发送命令
 * @param	cmd	:	命令
 * @param	res	:	需要检查的返回指令
 * @return	0, 表示成功, 其他值表示失败
 ******************************************************************************/
static int __esp8266_send_cmd(ESP8266Dev_t *dev, char *cmd, char *res)
{
	if (!dev || !dev->init_flag)
		return -1;
		
	unsigned char timeout = 200;

	__esp8266_uart_sned_string(USART2, (unsigned char *)cmd, strlen((const char *)cmd));
	
	while(timeout--)
	{
		if(__esp8266_wait_recv() == REV_OK)					// 如果收到数据
		{
			if(strstr((const char *)g_rx_buf, res) != NULL)	// 如果检索到关键词
			{
				__esp8266_clear(dev);						// 清空缓存
				
				return 0;
			}
		}
		
		delay_ms(10);
	}
	
	return 1;
}

/******************************************************************************
 * @brief	ESP8266发送数据
 * @param	data	:	数据
 * @param	len		:	长度
 * @return	0, 表示成功, 其他值表示失败
 ******************************************************************************/
static int __esp8266_send_data(ESP8266Dev_t *dev, unsigned char *data, unsigned short len)
{
	if (!dev || !dev->init_flag)
		return -1;

	char cmd_buf[32];
	
	__esp8266_clear(dev);							// 清空接收缓存
	sprintf(cmd_buf, "AT+CIPSEND=%d\r\n", len);		// 发送命令
	if(!__esp8266_send_cmd(dev, cmd_buf, ">"))		// 收到‘>’时可以发送数据
	{
		__esp8266_uart_sned_string(USART2, data, len);		// 发送设备连接请求数据
	}

	return 0;
}

/******************************************************************************
 * @brief	ESP8266获取平台返回的数据，返回格式为 "+IPD,x:y" x是数据长度，y是数据内容
 * @param	timeout	:	等待的时间(乘以10ms)
 * @return	平台返回的原始数据
 ******************************************************************************/
static uint8_t *__esp8266_get_ipd(ESP8266Dev_t *dev, unsigned short timeout)
{
	char *ipd = NULL;
	
	do
	{
		if(__esp8266_wait_recv() == REV_OK)			// 如果接收完成
		{
			ipd = strstr((char *)g_rx_buf, "IPD,");	// 搜索“IPD”头
			if(ipd == NULL)							// 如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{
				// ESP8266_DEBUG("\"IPD\" not found\r\n");
			}
			else
			{
				ipd = strchr(ipd, ':');				// 找到':'
				if(ipd != NULL)
				{
					ipd++;
					return (unsigned char *)(ipd);
				}
				else
					return NULL;
				
			}
		}
		delay_ms(5);										// 延时等待
	} while(timeout--);
	
	return NULL;	// 超时还未找到，返回空指针
}

/******************************************************************************
 * @brief	ESP8266串口中断回调函数
 * @param	无
 * @return	无
 ******************************************************************************/
void esp8266_uart_irq_callback(void)
{
	if(g_cnt >= sizeof(g_rx_buf))
	{
		g_cnt = 0; // 防止串口被刷爆
	}
	g_rx_buf[g_cnt++] = USART2->DR;
}

/******************************************************************************
 * @brief	USART2串口中断函数
 * @param	无
 * @return	无
 ******************************************************************************/
void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) // 接收中断
	{
		esp8266_uart_irq_callback();
		USART_ClearFlag(USART2, USART_FLAG_RXNE);
	}
}
