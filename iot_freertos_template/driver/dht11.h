#ifndef __DHT11_H
#define __DHT11_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#if defined(STM32F10X_HD) || defined(STM32F10X_MD)
    #include "stm32f10x.h"
	
    typedef GPIO_TypeDef*	DHT11_GPIO_Port;
	
#elif defined(STM32F40_41xxx)
	#include "stm32f4xx.h"
	
	typedef GPIO_TypeDef*	DHT11_GPIO_Port;
	
#else
    #error dht11.h: No processor defined!
#endif

typedef struct {
	DHT11_GPIO_Port port;	// 端口
    uint32_t pin;			// 引脚
}DHT11Config_t;

typedef struct DHT11Dev {
    DHT11Config_t config;
	bool init_flag;                         // 初始化标志
    uint8_t temperature;                    // 温度
    uint8_t humidity;                       // 湿度
	int (*get_data)(struct DHT11Dev *pDev);	// 获取数据
	int (*deinit)(struct DHT11Dev *pDev);   // 去初始化
}DHT11Dev_t;

int dht11_init(DHT11Dev_t *pDev);

#endif
