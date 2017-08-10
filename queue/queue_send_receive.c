#include <...>
/*xQueueHandle xQueueCreate( unsigned portBASE_TYPE uxQueueLength,队列能够存储的最大单元数目，即队列深度。
unsigned portBASE_TYPE uxItemSize );队列中数据单元的长度，以字节为单位。
队列在使用前必须先被创建。
队列由声明为xQueueHandle 的变量进行引用。xQueueCreate()用于创建一个队
列，并返回一个xQueueHandle 句柄以便于对其创建的队列进行引用。
当创建队列时，FreeRTOS 从堆空间中分配内存空间。分配的空间用于存储队列数
据结构本身以及队列中包含的数据单元。如果内存堆中没有足够的空间来创建队列，
xQueueCreate()将返回NULL。
*/


/* 声明一个类型为 xQueueHandle 的变量. 其用于保存队列句柄，以便三个任务都可以引用此队列 */
xQueueHandle xQueue;

static void vSenderTask(void *pvParameters)
{
long lValueToSend;
portBASE_TYPE xStatus;
  /*该任务会被创建2个实例，所以写入队列的值通过任务入口参数传递——这种方式使得每个实例
  使用不同的值。队列创建时指定其数据单元为long型，所以把入口参数强制转换为数据单元要求的类型*/
  lValueToSend = (long)pvParameters;
  for( ;; )
  {
    /* 往队列发送数据
    第一个参数是要写入的队列。队列在调度器启动之前就被创建了，所以先于此任务执行。
    第二个参数是被发送数据的地址，本例中即变量lValueToSend的地址。
    第三个参数是阻塞超时时间 – 当队列满时，任务转入阻塞状态以等待队列空间有效。本例中没有设定超
    时时间，因为此队列决不会保持有超过一个数据单元的机会，所以也决不会满。
    */
    xStatus = xQueueSendToBack(xQueue, &lValueToSend, 0);
    if(xStatus != pdPASS)
    {
      /*发送操作由于队列满而无法完成——这必然存在错误，因为读队列任务的存在不可能满*/
      vPrintString("Could no send to the Queue.\n");
    }
    /* 允许其它发送任务执行。 taskYIELD()通知调度器现在就切换到其它任务，
    而不必等到本任务的时间片耗尽 */
    taskYIELD();
  }
}


static void vReceiverTask(void *pvParameters)
{
  /*声明变量，用于保存从队列中接收到的数据*/
long lReceivedValue;
portBASE_TYPE xStatus;
const portTickType xTicksToWait = 100 / portTICK_RATE_MS;
  for( ;; )
  {
    /* 此调用会发现队列一直为空，因为本任务将立即删除刚写入队列的数据单元。 */
    if(uxQueueMessgesWaiting(xQueue) != 0)
    {
      vPrintString("Queue should have been empty!\n");
    }

    /* 从队列中接收数据
    第一个参数是被读取的队列。队列在调度器启动之前就被创建了，所以先于此任务执行。
    第二个参数是保存接收到的数据的缓冲区地址，本例中即变量lReceivedValue的地址。此变量类型与
    队列数据单元类型相同，所以有足够的大小来存储接收到的数据。
    第三个参数是阻塞超时时间 – 当队列空时，任务转入阻塞状态以等待队列数据有效。本例中常量
    portTICK_RATE_MS用来将100ms绝对时间转换为以系统心跳为单位的时间值。
    */
    xStatus = xQueueReceive(xQueue, &lReceivedValue, xTicksToWait);
    if(xStatus == pdPASS)
    {
      /*成功读出数据，打印出来*/
      vPrintStringAndNumber("Received = ", lReceivedValue);
    }
    else
    {
      /*等待100ms也没有收到数据，必然存在错误，因为发送任务在不停地写队列*/
      vPrintString("Could not Receive from the Queue.\n");
    }
  }
}


int main(void)
{
  /* 创建的队列用于保存最多5个值，每个数据单元都有足够的空间来存储一个long型变量 */
  xQueue = xQueueCreate(5, sizeof(long));
  if(xQueue != NULL)
  {
    /*创建2个写队列的任务，任务入口参数用于传递发送到队列的值；所以一个实例不停往队列
    发送100，而另一个不断发送200,；优先级都设置为1*/
    xTaskCreate(vSenderTask, "Sender1", 1000, (void *)100, 1, NULL);
    xTaskCreate(vSenderTask, "Sender2", 1000, (void *)200, 1, NULL);

    /*创建一个读队列的任务，优先级设置为2，高于写队列任务*/
    xTaskCreate(vReceiverTask, "Receiver", 1000, NULL, 2, NULL);

    vTaskStartScheduler();
  }
  else
  {
    /*队列创建失败*/
  }

  for( ;; );
}
