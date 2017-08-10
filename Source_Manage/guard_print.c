/*守护任务使用了一个FreeRTOS 队列来对终端实现串行化访问。该任务内部实现
不必考虑互斥，因为它是唯一能够直接访问终端的任务。
守护任务大部份时间都在阻塞态等待队列中有信息到来。当一个信息到达时，守护
任务仅仅简单地将收到的信息写到标准输出上，然后又返回阻塞态，继续等待下一条信
息地到来。

在本例中，一个心跳中断钩子函数用于每200 心跳周期就输出一个消息。
心跳钩子函数(或称回调函数)由内核在每次心跳中断时调用。要挂接一个心跳钩子函数，需要
 设置FreeRTOSConfig.h 中的常量configUSE_TICK_HOOK 为1。

 心跳钩子函数在系统心跳中断的上下文上执行，所以必须保证非常短小，适度占用
栈空间，并且不要调用任何名字不带后缀”FromISR”的FreeRTOS API 函数。*/


/*定义任务和中断将会通过守护任务输出的字符串*/
static char *pcStringsToPrint[] =
{
  "Task1 *****************\n",
  "Task2 -----------------\n",
  "message printed form the tick hook interrupt #####\n"
};

/*声明xQueueHandle变量，这个变量将会用于打印任务和中断往守护任务发送消息*/
xQueueHandle xPrintQueue;

static void prvStdioGatekeeperTask(void *pvParameters)
{
char *pcMessageToPrint;
/* 这是唯一允许直接访问终端输出的任务。任何其它任务想要输出字符串，都不能直接访问终端，而是将要
输出的字符串发送到此任务。并且因为只有本任务才可以访问标准输出，所以本任务在实现上不需要考虑互斥
和串行化等问题。 */
  for( ;; )
  {
    /* 等待信息到达。指定了一个无限长阻塞超时时间，所以不需要检查返回值 – 此函数只会在成功收到
消息时才会返回。 */
    xQueueReceive(xPrintQueue, &pcMessageToPrint, portMAX_DELAY);

    printf("%s\n", pcMessageToPrint);
    fflush(stdio);
    /*Now simply go back to wait for the next message*/
  }
}
/*字符串通过队列发送到守护任务，而不是直接输出到终端*/
static void prvPrintTask(void *pvParameters)
{
int iIndexToString;
  iIndexToString = (int)pvParameters;
  for( ;; )
  {
    /* 打印输出字符串，不能直接输出，通过队列将字符串指针发送到守护任务。队列在调度器启动之前就
创建了，所以任务执行时队列就已经存在了。并有指定超时等待时间，因为队列空间总是有效。 */
    xQueueSendToBack(xPrintQueue, &(pcStringsToPrint[iIndexToString]), 0);
/* 等待一个伪随机时间。注意函数rand()不要求可重入，因为在本例中rand()的返回值并不重要。但
在安全性要求更高的应用程序中，需要用一个可重入版本的rand()函数 – 或是在临界区中调用rand()函数。 */
    vTaskDelay(rand() & 0x1FF);
  }
}

/*心跳钩子函数
仅仅是简单地对其被调用次数进行计数，计数至200就向守护任务发送信息
为了更好的演示效果，将信息发送到队列首，打印输出任务将信息发送到队列尾*/
void vApplicationTickHook(void)
{
static int iCount = 0;
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  iCount++;
  if(iCount >= 200)
  {
    xQueueSendToBackFromISR(xPrintQueue, &(pcStringToPrint[2]),
      &xHigherPriorityTaskWoken);
    iCount = 0;
  }
}


int main(void)
{
  /*创建队列，深度为5，数据单元类型为字符指针*/
  xPrintQueue = xQueueCreate(5, sizeof(char *));
  srand(567);
  if(xPrintQueue != NULL)
  {
    /*创建任务的两个实例，用于向守护任务发送信息。任务入口参数传入需要输出的字符串索引号。
    这两个任务具有不同的优先级，所以高优先级任务有时会抢占低优先级任务*/
    xTaskCreate(prvPrintTask, "Print1", (void *)0, 1, NULL);
    xTaskCreate(prvPrintTask, "Print2", (void *)1, 2, NULL);

    /*创建守护任务，这是唯一一个允许直接访问标准输出的任务*/
    xTaskCreate(prvStdioGatekeeperTask, "Gatekeeper", 1000, NULL, 0, NULL);

    vTaskStartScheduler();
  }
  for( ;; );
}
