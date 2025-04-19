#include "my_tasks.h"

TaskHandle_t led_task_handler;		// 任务句柄

/******************************************************************************
 * @brief	LED任务
 * @param	p	:  参数
 * @return	无
 ******************************************************************************/
void task_led(void *p)
{

	while (1)
	{
//		led.on(&led);
//        vTaskDelay(500);
//        led.off(&led);
        vTaskDelay(500);
	}
}
