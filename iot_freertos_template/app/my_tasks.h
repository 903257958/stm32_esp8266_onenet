#ifndef __MY_TASKS_H
#define __MY_TASKS_H

/* C标准库 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* 驱动 */
#include "timer.h"
#include "delay.h"
#include "led.h"
#include "oled.h"
#include "usart.h"
#include "esp8266.h"
#include "dht11.h"

/* 设备 */
extern LEDDev_t led;
extern OLEDDev_t oled;
extern DHT11Dev_t dht11;

/* 开始任务 */
#define START_TASK_PRIO	1
#define START_STK_SIZE	128
extern TaskHandle_t start_task_handler;
void task_start(void *p);

/* LED任务 */
#define LED_TASK_PRIO	1
#define LED_STK_SIZE	128
extern TaskHandle_t led_task_handler;
void task_led(void *p);

/* 网络任务 */
#define NET_TASK_PRIO	10
#define NET_STK_SIZE	512
extern TaskHandle_t net_task_handler;
void task_net(void *p);

/* 传感器任务 */
#define SENSOR_TASK_PRIO	2
#define SENSOR_STK_SIZE	    128
extern TaskHandle_t sensor_task_handler;
void task_sensor(void *p);

#endif
