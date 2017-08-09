#include "...h"


/*使用同一套任务函数（任务实现代码），但是仍然可以创建多个任务实例*/

void vTaskFunction(void *pvParameters)
{
char *pcTaskName;
volatile unsigned long u1;
  /*使用传递进来的参数*/
  pcTaskName = (char *)pvParameters;
  for( ;; )
  {
    vPrintString(pcTaskName);
    for(u1 = 0; u1 < mainDELAY_LOOP_COUNT; u1++)
    {

    }
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
