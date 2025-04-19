#include "my_tasks.h"

TaskHandle_t sensor_task_handler;		// 任务句柄

/******************************************************************************
 * @brief	传感器任务
 * @param	p	:  参数
 * @return	无
 ******************************************************************************/
void task_sensor(void *p)
{
    
	while (1)
	{
		dht11.get_data(&dht11);
		
		vTaskDelay(500);
	}
}
