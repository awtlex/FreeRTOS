/*任务创建和删除API函数位于文件task.c中，需要包含task.h头文件*/
#include <task.h>

/*
  创建任务
  BaseType_t xTaskCreate(	TaskFunction_t pxTaskCode, 指针，指向任务函数的入口。任务永远不会返回（位于死循环内）。
							const char * const pcName, 任务描述。主要用于调试。
							const uint16_t usStackDepth, 指定任务堆栈大小，能够支持的堆栈变量数量，而不是字节数。
							void * const pvParameters, 指针，当任务创建时，作为一个参数传递给任务。
							UBaseType_t uxPriority, 任务的优先级。
							TaskHandle_t * const pxCreatedTask ); 用于回传一个句柄（ID），创建任务后可以使用这个句柄引用任务。
  删除任务
  void vTaskDelete( TaskHandle_t xTaskToDelete );参数为被删除任务的句柄。为NULL表示删除当前任务。
 */


 void vTaskCode(void *pvParameters)
 {
   for( ; ;)
   {
     /*任务具体的处理工作*/
   }
 }

 void vOtherFunction(void)
 {
   static unsigned char ucParameterToPass;//将会传递给Task
   xTaskHandle xHandle;
   /*创建任务，存储句柄。
    *注：传递的参数ucParameterToPass必须和任务具有相同的生存周期因此这里定义为静态变量。
    *如果它只是一个自动变量，可能不会有太长的生存周期，因为中断和高优先级任务可能会用到它
    */
    xTaskCreate(vTaskCode, "NAME", STACK_SIZE, &ucParameterToPass,
                tskIDLE_PRIORITY, &xHandle);

    if(xHandle != NULL)
    {
      vTaskDelete(xHandle);
    }
 }

/*--------------------------------------------------------------------------------*/
#include <...>

void vTask1(void *pvParameters)
{
const char *pcTaskName = "Task 1 is running\r\n";
volatile unsigned long u1;

  /*和大多数任务一样，处于死循环*/
  for( ;; )
  {
    /*打印任务名字*/
    vPrintString(pcTaskName);

    /*延时，以产生一个周期*/
    for(u1 = 0; u1 < mainDELAY_LOOP_COUNT; u1++)
    {
      /*空循环，最原始的延时实现方式；可以用delay、sleep代替*/
    }
  }
}

void vTask2(void *pvParameters)
{
const char *pcTaskName = "Task 2 is running\r\n";
volatile unsigned long u2;
  for( ;; )
  {
    vPrintString(pcTaskName);
    for(u2 = 0; u2 < mainDELAY_LOOP_COUNT; u2++)
    {

    }
  }
}


int main(void)
{
  /*创建第一个任务。实用的程序应当检测返回值，以确保成功*/
  xTaskCreate(vTask1,//指向任务函数的指针
              "Task 1",//任务的文本名字，只会在调试中用到
              1000,//栈深度，大多数小型微控制器会使用的值会比此值小得多
              NULL,//没有任务参数
              1,//此任务运行在优先级1上
              NULL);//不会用到任务句柄
  /*再创建一个*/
  xTaskCreate(vTask2, "Task 2", 1000, NULL, 1, NULL);

  /*启动调度器，任务开始执行*/
  vTaskStartScheduler();

  /*正常情况下，main函数不会执行到这里；否则很可能是内存堆空间不足导致任务无法创建*/
  for( ;; );
}
