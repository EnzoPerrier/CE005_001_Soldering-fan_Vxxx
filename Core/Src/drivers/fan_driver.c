/**
 ******************************************************************************
 * @author  Enzo Perrier
 * @file    fan_driver.c
 * @brief   Driver bas niveau pour le ventilateur (commande PWM)
 *
 ******************************************************************************
 */


#include "main.h"
#include "drivers/fan_driver.h"

extern TIM_HandleTypeDef htim1; // Génrateur PWM
extern TIM_HandleTypeDef htim3; // Tachymètre

#include "fan.h"

#define FAN_TIMER_FREQ_HZ       1000000U  // TIM3 configuré à 1 MHz donc 1 tick = 1µs 100 = 1ms...
#define FAN_PULSES_PER_REV      2U // Nombre de ticks par tour du ventilateur
#define FAN_TIMEOUT_MS          1000U

static volatile uint32_t s_lastCapture = 0;
static volatile uint32_t s_periodTicks = 0;
static volatile uint8_t  s_captureReady = 0;
static volatile uint32_t s_lastPulseTick = 0;

Fan_Status_t Fan_Init(void){

	if((HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) && HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1)) != HAL_OK) // Initialisation PWM + compteur tachymètre
	{
		Error_Handler();
		return FAN_ERROR;
	}
	else {

		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
		return FAN_OK;
	}

}

//ARR=39 donc 0 = 0%; 20=50% et 40=100%

Fan_Status_t Fan_SetSpeedPercent(uint8_t percent){

	uint8_t val_PWM; // min = 0 max = 39

	if(Fan_status != FAN_OK){
		return FAN_ERROR;
	}

	if(percent < 0 || percent > 100){
		return FAN_INVALID_PARAM;
	}

	val_PWM = (percent*39)/100;

	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,val_PWM);

	return FAN_OK;
}

Fan_Status_t Fan_GetSpeedPercent(uint8_t *p_percent){



}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if ((htim->Instance == TIM3) &&
        (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2))
    {
        uint32_t capture;

        capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);

        if (s_lastCapture != 0U)
        {
            uint32_t delta;

            if (capture >= s_lastCapture)
            {
                delta = capture - s_lastCapture;
            }
            else
            {
                delta = (htim->Init.Period + 1U)
                        - s_lastCapture + capture;
            }

            s_periodTicks = delta;
            s_captureReady = 1U;
        }

        s_lastCapture = capture;
        s_lastPulseTick = HAL_GetTick();
    }
}


