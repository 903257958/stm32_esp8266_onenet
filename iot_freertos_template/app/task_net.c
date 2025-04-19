#include "my_tasks.h"
#include "onenet.h"

TaskHandle_t net_task_handler;		// 任务句柄

/******************************************************************************
 * @brief	网络任务
 * @param	p	:  参数
 * @return	无
 ******************************************************************************/
void task_net(void *p)
{
    unsigned short timeCount = 0;	//发送间隔变量
	unsigned char *dataPtr = NULL;
    
	while (1)
	{
        /* 上发数据 */
        if(++timeCount >= 500)									//发送间隔5s
		{
			UsartPrintf(USART_DEBUG, "OneNet_SendData\r\n");
			OneNet_SendData();									//发送数据
			
			timeCount = 0;
			ESP8266_Clear();
		}
		
        /* 接收数据 */
		dataPtr = ESP8266_GetIPD(0);
		if(dataPtr != NULL)
			OneNet_RevPro(dataPtr);

        vTaskDelay(10);
	}
}
