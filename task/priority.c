/*可以用于在调度器启动后改变任何任务的优先级
void vTaskPrioritySet( xTaskHandle pxTask, unsigned portBASE_TYPE uxNewPriority );

获取查询一个任务的优先级
unsigned portBASE_TYPE uxTaskPriorityGet( xTaskHandle pxTask );

如果pxTask为NULL,则默认为是自己（当前任务）
*/


/*声明变量用于保存任务2的句柄*/
xTaskHandle xTask2Handle;

void vTask1(void *pvParameters)
{
unsigned portBASE_TYPE uxPriority;
  /*本任务将会比任务2更先运行，因为本任务创建在更高的优先级上
  任务1,2都不会阻塞，所以两者要么处于就绪态，要么处于运行态
  */
  /*查询本任务当前的优先级——传递NULL即可*/
  uxPriority = uxTaskPriorityGet(NULL);
  for( ;; )
  {
    vPrintString("Task 1 is running\n");
    /*把任务2的优先级设置到高于任务1的优先级，会使得任务2立即得到执行
    注意调用set时用到的任务2的句柄*/
    vPrintString("to raise the priority of Task2\n");
    vTaskPrioritySet(xTask2Handle, (uxPriority + 1));
    /*本任务只会在优先级高于2的时候才会执行，如果允许到这里了，那么任务2必然已经执行过了
    并且将其自身的优先级设置回比任务1更低的优先级*/
  }
}

void Task2(void *pvParameters)
{
unsigned portBASE_TYPE uxPriority;
  uxPriority = uxTaskPriorityGet(NULL);
  for( ;; )
  {
    /*当任务运行到这里，任务1必然已经运行过了，并将2的优先级设置到高于任务1了*/
    vPrintString("Task 2 is running\n");

    /*将自己的优先级设置回原来的值，则任务1优先级变高，抢占本任务*/
    vPrintString("to lower the Task2 priority\n");
    vTaskPrioritySet(NULL, (uxPriority - 2));
  }
}


int main(void)
{
  /*任务1创建在优先级2上。任务参数没有用到，设为NULL。任务句柄也不会用到，也设为NULL*/
  /* The task is created at priority 2 */
  xTaskCreate(vTask1, "Task1", 1000, NULL, 2, NULL);

  /*任务2优先级低于任务1。
  任务参数没有用到，设为NULL。但任务2的任务句柄会被用到，故将xTask2Handle的地址传入*/
  xTaskCreate(vTask2, "Task2", 1000, NULL, 1, &xTask2Handle);

  vTaskStartScheduler();

  for( ;; );
}
