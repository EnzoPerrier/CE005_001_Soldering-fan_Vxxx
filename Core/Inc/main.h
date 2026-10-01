/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ENCOD_A_Pin GPIO_PIN_0
#define ENCOD_A_GPIO_Port GPIOA
#define ENCOD_B_Pin GPIO_PIN_1
#define ENCOD_B_GPIO_Port GPIOA
#define ENCOD_SW_Pin GPIO_PIN_2
#define ENCOD_SW_GPIO_Port GPIOA
#define V_BATT_Pin GPIO_PIN_4
#define V_BATT_GPIO_Port GPIOA
#define V_ALIM_Pin GPIO_PIN_5
#define V_ALIM_GPIO_Port GPIOA
#define TACHY_FAN_Pin GPIO_PIN_7
#define TACHY_FAN_GPIO_Port GPIOA
#define CMD_FAN_Pin GPIO_PIN_8
#define CMD_FAN_GPIO_Port GPIOA
#define CS_BME680_Pin GPIO_PIN_15
#define CS_BME680_GPIO_Port GPIOA
#define SPI_SCK_Pin GPIO_PIN_3
#define SPI_SCK_GPIO_Port GPIOB
#define SPI_MISO_Pin GPIO_PIN_4
#define SPI_MISO_GPIO_Port GPIOB
#define SPI_MOSI_Pin GPIO_PIN_5
#define SPI_MOSI_GPIO_Port GPIOB
#define RES_LCD_Pin GPIO_PIN_6
#define RES_LCD_GPIO_Port GPIOB
#define DC_LCD_Pin GPIO_PIN_7
#define DC_LCD_GPIO_Port GPIOB
#define CS_LCD_Pin GPIO_PIN_8
#define CS_LCD_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
