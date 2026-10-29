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

extern TIM_HandleTypeDef htim1;

uint8_t Fan_status = FAN_NOT_INITIALIZED;

Fan_Status_t Fan_Init(void){

	if(HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK)
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

	if(Fan_status != FAN_OK){
		Fan_status = FAN_ERROR;
		Error_Handler();
	}

	if(percent < 0 || percent > 100){
		Fan_status = FAN_INVALID_PARAM;
	}



}
