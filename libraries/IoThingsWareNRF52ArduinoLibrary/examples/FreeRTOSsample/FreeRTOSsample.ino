#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
//#include "bsp.h"
#include "nordic_common.h"
#include "nrf_drv_clock.h"
#include "sdk_errors.h"
#include "app_error.h"

TaskHandle_t  led_toggle_task_handle;


static void led_toggle_task_function (void * pvParameter)
{
  UNUSED_PARAMETER(pvParameter);
  while (true)
  {
    digitalWrite(PIN_LED1, !digitalRead(PIN_LED1));

    /* Delay a task for a given number of ticks */
    vTaskDelay(200);

    /* Tasks must be implemented to never return... */
  }
}

void setup() {
  // put your setup code here, to run once:
  ret_code_t err_code;

  /* Initialize clock driver for better time accuracy in FREERTOS */
  err_code = nrf_drv_clock_init();
  APP_ERROR_CHECK(err_code);
  UNUSED_VARIABLE(xTaskCreate(led_toggle_task_function, "LED0", configMINIMAL_STACK_SIZE + 200, NULL, 2, &led_toggle_task_handle));
  /* Activate deep sleep mode */
  suspendLoop();
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

  /* Start FreeRTOS scheduler. In this case useless because Arduino environment yet started it*/
  //vTaskStartScheduler();

}

void loop() {
  // put your main code here, to run repeatedly:

}
