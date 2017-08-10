/*使用mutex重写vPrintString()*/


static void prvNewPrintString(const portCHAR *pcString)
{
  /* 互斥量在调度器启动之前就已创建，所以在此任务运行时信号量就已经存在了。试图获得互斥量。
  如果互斥量无效，则将阻塞，进入无超时等待。
  xSemaphoreTake()只可能在成功获得互斥量后返回，所以无需检测返回值。
  如果指定了等待超时时间，则代码必须检测到xSemaphoreTake()返回pdTRUE后，
  才能访问共享资源(此处是指标准输出)。 */
  xSemaphoreTake(xMutex, portMAX_DELAY);
  {
  /*程序执行到这里表示已经成功获得互斥量，现在可自由访问stdout，因为任意时刻
  只有一个任务能持有互斥量*/
    printf("%s\n", pcString);
    fflush(stdout);
    /*互斥量必须归还*/
  }
  xSemaphoreGive(xMutex);

  if(kbhit())
  {
    vTaskEndScheduler();
  }
}


static void prvPrintTask(void *pvParameters)
{
char *pcStringToPrint;
  pcStringToPrint = (char *)pvParameters;
  for( ;; )
  {
    prvNewPrintString(pcStringToPrint);
    /* 等待一个伪随机时间。注意函数rand()不要求可重入，因为在本例中rand()的返回值并不重要。但
在安全性要求更高的应用程序中，需要用一个可重入版本的rand()函数 – 或是在临界区中调用rand()
函数。 */
    vTaskDelay((rand() & 0x1FF));
  }
}


int main(void)
{
  /*信号量在使用前必须先创建；这里创建了一个互斥量类型的信号量*/
  xMutex = xSemaphoreCreateMutex();
  /*使用一个随机延时，这里给随机数发生器生成种子*/
  srand(567);
  if(xMutex != NULL)
  {
    xTaskCreate(prvPrintTask, "Print1", 1000, "Task1************\n", 1, NULL);
    xTaskCreate(prvPrintTask, "Print2", 1000, "Task2------------\n", 2, NULL);

    vTaskStartScheduler();
  }
  for( ;; );
}
