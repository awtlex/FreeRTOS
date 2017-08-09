#include "...h"


/*----------------相对延时---------------------------*/

void vTaskFunction(void *pvParameters)
{
char *pcTaskName;

  /*使用传递进来的参数*/
  pcTaskName = (char *)pvParameters;
  for( ;; )
  {
    vPrintString(pcTaskName);

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

/*-------------------------绝对延时------------------------*/
void vTaskFunction(void *pvParameters)
{
char *pcTaskName;
portTickType xLastWakeTime;

  /*使用传递进来的参数*/
  pcTaskName = (char *)pvParameters;

  /*该变量需要被初始化为当前心跳计数值；这是该变量唯一一次被显式赋值，之后将在delayuntil函数中自动更新*/
  xLastWakeTime = xTaskGetTickCount();

  for( ;; )
  {
    vPrintString(pcTaskName);
    /*以心跳周期为单位*/
    vTaskDelayUntil(&xLastWakeTime, 250 / portTICK_RATE_MS);//精确的延时250ms
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
