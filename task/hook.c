#include <...>

/*空闲任务钩子函数必须具有如下的函数名和函数原型
  void vApplicationIdleHook(void);
  同时FreeRTOSConfig.h中配置常量configUSE_IDLE_HOOK必须定义为1,这样
  空闲钩子函数才会被调用
*/

/*--------------------------------------------------*/
/*延时函数带有阻塞性质，会产生大量的空闲时间；在这期间空闲任务会
得到执行，因为两个应用任务均处于阻塞态
通过空闲钩子函数来使用这些空闲时间*/
/*--------------------------------------------------*/

unsigned long ulIdleCycleCount 0UL;//这个变量将在钩子函数中被使用
/*空闲钩子函数必须命名为vApplicationIdleHook(),无参数也无返回值*/
void vApplicationIdleHook(void)
{
  ulIdleCycleCount++;
}


void vTaskFunction(void *pvParameters)
{
char *pcTaskName;

  /*使用传递进来的参数*/
  pcTaskName = (char *)pvParameters;
  for( ;; )
  {
    //vPrintString(pcTaskName);
    vPrintStringAndNumber(pcTaskName, ulIdleCycleCount);//打印任务名，以及数值
    vTaskDelay(250 / portTICK_RATE_MS);//相对于该函数调用，延时250ms
  }
}

/*任务参数*/
static const char *pcTextForTask1 = "Task 1 is running\r\n";
static const char *pcTextForTask2 = "Task 2 is running\r\n";

int main(void)
{
  xTaskCreate(vTaskFuction, "Task 1", 1000, (void *)pcTextForTask1, 1, NULL);
  xTaskCreate(vTaskFuction, "Task 2", 1000, (void *)pcTextForTask2, 1, NULL);
  vTaskStartScheduler();

  for( ;; );
}
