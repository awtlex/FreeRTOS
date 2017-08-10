/*二值信号量做多可以锁存一个中断事件，
在锁存的事件还未被处理之前，如果还有中断发生，那么后续发生的中断将会丢失
采用计数信号量可以避免——可以看做计数信号量是深度大于1的队列*/

/*1.事件处理：每次事件发生，信号量+1，每处理一个，信号量-1.只要信号量有差值，说明就需要处理
2.资源管理：信号量的计数值表示可用资源的数目，获取资源，-1；归还资源+1.减至0表示无资源*/

/*利用计数信号量对任务和中断进行同步*/




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

/* 在信号量使用之前必须先创建。本例中创建了一个计数信号量。
此信号量的最大计数值为10，初始计数值为0 */
xCountingSemaphore = xSemaphoreCreateCounting(10, 0);

static void __interrupt __far vExampleInterruptHandler(void)
{
static portBASE_TYPE xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  /*多次给出信号量。第一次给出时使得延迟处理任务解除阻塞。后续给出用于演示利用被信号量锁存事件，
以便延迟处理任何依序对这些中断事件进行处理而不会丢中断。用这种方式来模拟处理器产生多个中断，尽管
这些事件只是在单次中断中模拟出来的 */
  xSemaphoreGiveFromISR(xCountingSemaphore, &xHigherPriorityTaskWoken);
  xSemaphoreGiveFromISR(xCountingSemaphore, &xHigherPriorityTaskWoken);
  xSemaphoreGiveFromISR(xCountingSemaphore, &xHigherPriorityTaskWoken);

  if(xHigherPriorityTaskWoken == pdTRUE)
  {
    /* 给出信号量以使得等待此信号量的任务解除阻塞。如果解出阻塞的任务的优先级高于当前任务的优先
    级–强制进行一次任务切换，以确保中断直接返回到解出阻塞的任务(优选级更高)。
    说明：在实际使用中，ISR中强制上下文切换的宏依赖于具体移植。此处调用的是基于Open Watcom DOS
    移植的宏。其它平台下的移植可能有不同的语法要求。对于实际使用的平台，请参如数对应移植自带的示
    例程序，以决定正确的语法和符号。
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
