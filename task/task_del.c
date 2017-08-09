/*
任务可以使用API 函数vTaskDelete()删除自己或其它任务。
任务被删除后就不复存在，也不会再进入运行态。
空闲任务的责任是要将分配给已删除任务的内存释放掉。因此有一点很重要，那就
是使用vTaskDelete() API 函数的任务千万不能把空闲任务的执行时间饿死。

只有内核为任务分配的内存空间才会在任务被删除后自动回收。任务自己占用的内存或资源需要
由应用程序自己显式地释放。

void vTaskDelete( xTaskHandle pxTaskToDelete );
*/

xTaskHandle xTask2Handle;

void vTask2(void *pvParameters)
{
  /*任务2什么实质性的也没做，只是删除任务；
  如果传递NULL，可以删除自己，当然也可以传递自己的句柄；
  如果传递其他句柄，则删除对应的任务
  */
  vPrintString("Task2 is running and to delete itself\n");
  vTaskDelete(NULL);
  /*vTaskDelete(xTask2Handle);*/
}

void vTask1(void *pvParameters)
{
const portTickType xDelay100ms = 100 / portTICK_RATE_MS;
  for( ;; )
  {
    vPrintString("Task 1 is running\n");
    xTaskCreate(vTask2, "Task2", 1000, NULL, 2, &xTask2Handle);

    /*任务2具有最高优先级，所以任务1运行到这里，说明任务2已经完成执行，删除了自己
    任务1得以继续运行，延时100ms*/
    vTaskDelay(xDelay100ms);
  }
}


int main()
{
  xTaskCreate(vTask1, "Task1", 1000, NULL, 1, NULL);
  vTaskStartScheduler();
  for( ;; );
}
