/**
 * @file    irq_dispatch.c
 * @brief   Point unique de définition des callbacks HAL ; dispatch vers les drivers.
 * @author  Enzo Perrier
 */

#include "main.h"
#include "drivers/encoder_driver.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == ENCOD_SW_Pin)
    {
        Encoder_OnButtonIrq();
    }
    /* Futurs modules EXTI : ajouter un "else if" ici, et nulle part ailleurs. */
}
