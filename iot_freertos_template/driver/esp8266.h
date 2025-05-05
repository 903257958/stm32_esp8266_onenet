#ifndef __ESP8266_H
#define __ESP8266_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#if defined(STM32F10X_HD) || defined(STM32F10X_MD)
	#include "stm32f10x.h"
	
	typedef GPIO_TypeDef*	ESP8266GPIOPort_t;
	
#elif defined(STM32F40_41xxx) || defined(STM32F411xE) || defined(STM32F429_439xx)
	#include "stm32f4xx.h"
	
	typedef GPIO_TypeDef*	ESP8266GPIOPort_t;

#else
    #error esp8266.h: No processor defined!
#endif

#if 1
	#include "uart.h"
	extern UARTDev_t debug;
	#define ESP8266_DEBUG(str)	debug.send_string(&debug, str);
#else
	#define ESP8266_DEBUG(str)
#endif

#define REV_OK		0	// 接收完成标志
#define REV_WAIT	1	// 接收未完成标志

typedef struct ESP8266Dev {
	bool init_flag;							// 初始化标志
	void *priv_data;						// 私有数据指针
	int (*clear)(struct ESP8266Dev *dev);
    int (*send_cmd)(struct ESP8266Dev *dev, char *cmd, char *res);
    int (*send_data)(struct ESP8266Dev *dev, unsigned char *data, unsigned short len);
    uint8_t *(*get_ipd)(struct ESP8266Dev *dev, unsigned short timeout);
	int (*deinit)(struct ESP8266Dev *dev);  // 去初始化
}ESP8266Dev_t;

int esp8266_init(ESP8266Dev_t *dev);
void esp8266_uart_irq_callback(void);

#endif
