#include <...>

xQueueHandle xQueue;

typedef struct
{
  unsigned char ucValue;
  unsigned char ucSource;
}xData;

static const xData xStructsToSend[2] =
{
  {100, mainSENDER_1},//used by sender1
  {200, mainSENDER_2}//used by sender2
}


static void vSenderTask(void *pvParameters)
{
portBASE_TYPE xStatus;
const portTickType xTicksToWait = 100 / portTICK_RATE_MS;

  for( ;; )
  {
    /*第三个参数是阻塞超时时间 – 当队列满时，任务转入阻塞状态以等待队列空间有效。
    指定超时时间是因为写队列任务的优先级高于读任务的优先级；所以队列如预期一样很快写满，
    写队列任务就会转入阻塞态，此时读队列任务才会得以执行，才能从队列中把数据读走。*/
    xStatus = xQueueSendToBack(xQueue, pvParameters, xTicksToWait);
    if(xStatus != pdPASS)
    {
      /* 写队列任务无法将数据写入队列，直至100毫秒超时。
      这必然存在错误，因为只要写队列任务进入阻塞态，读队列任务就会得到执行，从而读走数据，
      腾出空间 */
      vPrintString("Could not send to the queue\n");
    }
    /*其他写队列任务得到执行*/
    taskYIELD();
  }
}

static void vReceiverTask(void *pvParameters)
{
/*声明结构体变量以保存从队列中读出的数据单元*/
xData xReceivedStructure;
portBASE_TYPE xStatus;
  for( ;; )
  {
    /*读队列任务的优先级最低，所以只能在写队列任务阻塞（队列满）时得以执行*/
    if(uxQueueMessgesWaiting(xQueue) != 3)//本例中队列深度为3
    {
      vPrintString("Queue should have been full!\n");
    }
    /*这里不需要指定阻塞超时时间*/
    xStatus = xQueueReceive(xQueue, &xReceivedStructure, 0);
    if(xStatus == pdPASS)
    {
      /*数据成功读出，打印输出数值及数据来源*/
      if(xReceivedStructure.ucSource == mainSENDER_1)
      {
        vPrintStringAndNumber("From Sender1 = ", xReceivedStructure.ucValue);
      }
      else
      {
        vPrintStringAndNumber("From sender2 = ", xReceivedStructure.ucValue);
      }
    }
    else
    {
      /*没有读到任何数据，这一定是发生了错误，因为此任务只会在队列满时才会得到执行*/
      vPrintString("Could not Receive from the queue.\n");
    }
  }
}

int main(void)
{
  /*创建队列用于保存最多3个xData类型的数据单元，即队列深度为3*/
  xQueue = xQueueCreate(3, sizeof(xData));
  if(xQueue != NULL)
  {
    xTaskCreate(vSenderTask, "Sender1", 1000, &(xReceivedStructure[0]), 2, NULL);
    xTaskCreate(vSenderTask, "Sender2", 1000, &(xReceivedStructure[1]), 2, NULL);

    xTaskCreate(vSenderTask, "Receiver", 1000, NULL, 1, NULL);

    vTaskStartScheduler();
  }
  else
  {
    /*创建队列失败*/
  }

  for( ;; );
}
