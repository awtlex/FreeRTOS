/*
 *FreeRTOS任务控制API函数
 *主要实现任务延时、任务挂起、解除任务挂起、任务优先级获取和设置等功能。
 */

/*1.相对延时
 *void vTaskDelay( const TickType_t xTicksToDelay )
 * 调用vTaskDelay()函数后，任务会进入阻塞状态
 *持续时间由vTaskDelay()函数的参数xTicksToDelay指定，单位是系统节拍时钟周期。
 *延时时间是从调用vTaskDelay()后开始计算的相对时间
 */

void vTaskFuction(void *pvParameters)
{
  /*阻塞500ms*/
  const portTickType xDelay = 500 / portTICK_RATE_MS;
  for ( ; ; )
  {
    /*每隔500ms触发一次led，触发后进入阻塞状态*/
    vToggledLED();
    vTaskDelay(xDelay);
  }
}

/*2.绝对延时
 * void vTaskDelayUntil( TickType_t * const pxPreviousWakeTime, 指针，指向一个变量，该变量保存任务最后一次解除阻塞的时间。
                                                                  第一次使用前，该变量必须初始化为当前时间。
                                                                  之后这个变量会在vTaskDelayUntil()函数内自动更新。
      const TickType_t xTimeIncrement )周期循环时间。当时间等于(*pxPreviousWakeTime + xTimeIncrement)时，任务解除阻塞。
                                        如果不改变参数xTimeIncrement的值，调用该函数的任务会按照固定频率执行。

    任务延时一个指定的时间。周期性任务可以使用此函数，以确保一个恒定的频率执行。
    这个函数不同于vTaskDelay()函数的一个重要之处在于：
    vTaskDelay()指定的延时时间是从调用vTaskDelay()之后（执行完该函数）开始算起的，
    但是vTaskDelayUntil()指定的延时时间是一个绝对时间。
 */
 //每10次系统节拍执行一次
  void vTaskFunction(void *pvParameters )
  {
      static portTickType xLastWakeTime;
      const portTickType xFrequency = 10;

      // 使用当前时间初始化变量xLastWakeTime
      xLastWakeTime = xTaskGetTickCount();

      for( ;; )
      {
          //等待下一个周期
          vTaskDelayUntil(&xLastWakeTime, xFrequency);

          // 需要周期性执行代码放在这里
      }
  }

3.获取任务优先级

UBaseType_t uxTaskPriorityGet( TaskHandle_t xTask )

voidvAFunction( void )
 {
     xTaskHandle xHandle;
     // 创建任务，保存任务句柄
     xTaskCreate( vTaskCode, "NAME",STACK_SIZE, NULL, tskIDLE_PRIORITY, &xHandle );
     // ...
     // 使用句柄获取创建的任务的优先级
     if( uxTaskPriorityGet( xHandle ) != tskIDLE_PRIORITY )
     {
         // 任务可以改变自己的优先级
     }
     // ...
     // 当前任务优先级比创建的任务优先级高？
     if( uxTaskPriorityGet( xHandle ) < uxTaskPriorityGet( NULL ) )
     {
         // 当前优先级较高
     }
 }

4.设置任务优先级
void vTaskPrioritySet( TaskHandle_t xTask, UBaseType_t uxNewPriority )

voidvAFunction( void )
 {
     xTaskHandle xHandle;
     // 创建任务，保存任务句柄。
     xTaskCreate( vTaskCode, "NAME",STACK_SIZE, NULL, tskIDLE_PRIORITY, &xHandle );
     // ...
     // 使用句柄来提高创建任务的优先级
     vTaskPrioritySet( xHandle,tskIDLE_PRIORITY + 1 );
     // ...
     // 使用NULL参数来提高当前任务的优先级，设置成和创建的任务相同。
     vTaskPrioritySet( NULL, tskIDLE_PRIORITY +1 );
 }

5.任务挂起
void vTaskSuspend( TaskHandle_t xTaskToSuspend )

voidvAFunction( void )
 {
     xTaskHandle xHandle;
     // 创建任务，保存任务句柄.
     xTaskCreate( vTaskCode, "NAME", STACK_SIZE, NULL, tskIDLE_PRIORITY, &xHandle );
     // ...
     // 使用句柄挂起创建的任务.
     vTaskSuspend( xHandle );
     // ...
     // 任务不再运行，除非其它任务调用了vTaskResume(xHandle )
     //...
     // 挂起本任务.
     vTaskSuspend( NULL );
     // 除非另一个任务使用handle调用了vTaskResume，否则永远不会执行到这里
 }

6.恢复挂起的任务
void vTaskResume( TaskHandle_t xTaskToResume );

voidvAFunction( void )
 {
         xTaskHandle xHandle;
     // 创建任务，保存任务句柄
     xTaskCreate( vTaskCode, "NAME",STACK_SIZE, NULL, tskIDLE_PRIORITY, &xHandle );
     // ...
     // 使用句柄挂起创建的任务
     vTaskSuspend( xHandle );
     // ...
     //任务不再运行，除非其它任务调用了vTaskResume(xHandle )
          //...
     // 恢复挂起的任务.
     vTaskResume( xHandle );
     // 任务再一次得到处理器时间
     // 任务优先级与之前相同
 }

7.恢复挂起的任务（在中断服务函数中使用）
BaseType_t xTaskResumeFromISR(TaskHandle_t xTaskToResume );

xTaskHandle xHandle;               //注意这是一个全局变量  

 void vAFunction( void )
 {
     // 创建任务并保存任务句柄
     xTaskCreate( vTaskCode, "NAME",STACK_SIZE, NULL, tskIDLE_PRIORITY, &xHandle );

     // ... 剩余代码.
 }

 void vTaskCode( void *pvParameters )
 {
     for( ;; )
     {
         // ... 在这里执行一些其它功能

         // 挂起自己
         vTaskSuspend( NULL );

         //直到ISR恢复它之前，任务会一直挂起
     }
 }

 void vAnExampleISR( void )
 {
     portBASE_TYPE xYieldRequired;

     // 恢复被挂起的任务
     xYieldRequired = xTaskResumeFromISR(xHandle );

     if( xYieldRequired == pdTRUE )
     {
         // 我们应该进行一次上下文切换
         // 注:  如何做取决于你具体使用，可查看说明文档和例程
         portYIELD_FROM_ISR();
     }
 }
