/**
 * @file    encoder_driver.c
 * @brief   Implémentation du driver encodeur (LPTIM2, 16 bits). Doc publique : voir encoder_driver.h
 * @author  Enzo Perrier
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"                       /* Inclut stm32g4xx_hal.h et les labels CubeMX */
#include "drivers/encoder_driver.h"

#include <stdbool.h>

/* Extern variables ----------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;         /* Généré par CubeMX : TIM2 en Encoder Mode */

/* Private constants ---------------------------------------------------------*/

/** Délai minimum entre deux appuis valides, en millisecondes. */
#define ENCODER_BUTTON_DEBOUNCE_MS  (50U)

/* Private variables ---------------------------------------------------------*/
static bool                     encoder_initialized = false;
static uint32_t                 raw_previous        = 0U;   /* Dernière valeur BRUTE du compteur */
static int32_t                  position_total      = 0;
static Encoder_ButtonCallback_t button_callback     = NULL;

/* Exported functions --------------------------------------------------------*/

Encoder_Status_t Encoder_Init(void)
{
    if (encoder_initialized)
    {
        return ENCODER_OK;
    }

    if (HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL) != HAL_OK)
    {
        return ENCODER_ERROR;
    }

    raw_previous        = __HAL_TIM_GET_COUNTER(&htim2);
    position_total      = 0;
    encoder_initialized = true;

    return ENCODER_OK;
}

Encoder_Status_t Encoder_GetPosition(int32_t *p_position)
{
    if (p_position == NULL)
    {
        return ENCODER_INVALID_PARAM;
    }

    if (!encoder_initialized)
    {
        return ENCODER_NOT_INITIALIZED;
    }

    uint32_t raw_now = __HAL_TIM_GET_COUNTER(&htim2);

    /* Choix assumé : soustraction non signée puis conversion en signé.
     * Le compteur étant sur 32 bits, le passage par zéro est géré par
     * l'arithmétique modulo 2^32. */
    int32_t delta = (int32_t)(raw_now - raw_previous);

    position_total += delta;
    raw_previous    = raw_now;      /* On mémorise la valeur BRUTE, jamais la position cumulée */

    *p_position = position_total;

    return ENCODER_OK;
}

void Encoder_RegisterButtonCallback(Encoder_ButtonCallback_t callback)
{
    button_callback = callback;
}

void Encoder_OnButtonIrq(void)
{
    static uint32_t last_press_tick = 0U;   /* Anti-rebond logiciel */
    uint32_t now = HAL_GetTick();

    if ((now - last_press_tick) >= ENCODER_BUTTON_DEBOUNCE_MS)
    {
        last_press_tick = now;

        if (button_callback != NULL)
        {
            button_callback();
        }
    }
}
