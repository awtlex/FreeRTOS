/*延迟中断处理
 采用二值信号量同步
 二值信号量可以在某个特殊的中断发生时，让任务解除阻塞，相对于让任务与中断同步
 这样中断时间处理量大的工作在同步任务中完成
 中断服务例程（ISR）中知识快速处理少部分工作
 */

/*用于周期性产生软件中断的周期任务*/
 static void vPeriodicTask(void *pvParameters)
 {
   for( ;; )
   {
     /*此任务通过每500ms产生一个软件中断来模拟中断事件*/
     vTaskDelay(500 / portTICK_RATE_MS);
     /*产生中断，并在产生之前和之后输出信息，以便在执行结果中直观指出执行流程*/
     vPrintString("Periodic task - to generate an interrupt\n");
     __asm{int 0x82}/*这条语句产生中断*/
     vPrintString("Periodic task - interrupt generated\n");
   }
 }

 /*延迟处理任务（此任务与中断同步*/
 static void vHandlerTask(void *pvParameters)
 {
   for( ;; )
   {
     /*使用信号量等待一个事件。信号量在调度器启动之前，即此任务执行之前就已被创建
     任务被无超时阻塞， 因此该函数调用只会在成功获取信号量之后才会返回*/
     xSemaphoreTake(xBinarySemaphore, portMAX_DELAY);
     /*程序运行到这里，时间必然已经发生,本例的时间处理只是简单地打印输出一个信息*/
     vPrintString("handler task - Processing event\n");
   }
 }

/*中断服务例程，即真正的中断处理程序。
这里仅仅是给出一个信号量，让延迟处理任务解除阻塞*/
static void __interrupt __far vExampleInterruptHandler(void)
{
static portBASE_TYPE xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;

  xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);
  if(xHigherPriorityTaskWoken == pdTRUE)
  {
    /*给出信号量以使得等待此信号量的任务解除阻塞。
    如果解出阻塞的任务的优先级高于当前任务的优先级 – 强制进行一次任务切换，
    以确保中断直接返回到解出阻塞的任务(优选级更高)。

    说明：在实际使用中，ISR中强制上下文切换的宏依赖于具体移植。
    对于实际使用的平台，请参如数对应移植自带的示例程序，以决定正确的语法和符号。
   */
    portSWITCH_CONTEXT();
  }
}


/*创建二值信号量及任务，安装中断服务例程，然后启动调度器*/
int main(void)
{
  /*信号量在使用前必须先创建，本例中创建了一个二值信号量*/
  vSemaphoreCreateBinary(xBinarySemaphore);
  /*安装中断服务例程*/
  _dos_setvect(0x82, vExampleInterruptHandler);
  /*检查信号量是否成功创建*/
  if(xBinarySemaphore != NULL)
  {
    /* 创建延迟处理任务。此任务将与中断同步。延迟处理任务在创建时使用了一个较高的优先级，
    以保证中断退出后会被立即执行。在本例中，为延迟处理任务赋予优先级3 */
    xTaskCreate(vHandlerTask, "handler", 1000, NULL, 3, NULL);
    /* 创建一个任务用于周期性产生软件中断。此任务的优先级低于延迟处理任务。
    每当延迟处理任务切出阻塞态，就会抢占周期任务*/
    xTaskCreate(vPeriodicTask, "Periodic", 1000, NULL, 1, NULL);

    vTaskStartScheduler();
  }

  for( ;; );
}
