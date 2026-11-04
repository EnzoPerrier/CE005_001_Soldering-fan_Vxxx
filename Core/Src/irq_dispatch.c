/**
 ******************************************************************************
 * @file    irq_dispatch.c
 * @brief   Point unique de définition des callbacks HAL ; dispatch vers les drivers.
 * @author  Enzo Perrier
 *
 * Règle : chaque callback HAL n'est défini qu'ICI (sinon : symbole défini deux
 * fois, erreur d'édition de liens). Les drivers exposent des fonctions
 * "OnXxx" ; ce fichier filtre l'instance/le canal puis les appelle.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "drivers/encoder_driver.h"
#include "drivers/fan_driver.h"

/* HAL callbacks -------------------------------------------------------------*/

/**
 * @brief Callback EXTI : dispatch selon la broche à l'origine de l'interruption.
 * @param GPIO_Pin  Broche ayant déclenché l'interruption.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == ENCOD_SW_Pin)
    {
        Encoder_OnButtonIrq();
    }
    /* Futurs modules EXTI : ajouter un "else if" ici, et nulle part ailleurs. */
}

/**
 * @brief Callback Input Capture : dispatch selon le timer et le canal.
 * @param htim  Handle du timer ayant capturé.
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if ((htim->Instance == TIM3) &&
        (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2))
    {
        Fan_OnTachCapture();
    }
    /* Futures captures : ajouter un "else if" ici, et nulle part ailleurs. */
}
