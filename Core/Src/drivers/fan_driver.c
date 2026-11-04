/**
 ******************************************************************************
 * @file    fan_driver.c
 * @brief   Implémentation du driver ventilateur. Doc publique : voir fan_driver.h
 * @author  Enzo Perrier
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"                       /* Inclut stm32g4xx_hal.h et les labels CubeMX */
#include "drivers/fan_driver.h"

#include <stdbool.h>

/* Extern variables ----------------------------------------------------------*/
extern TIM_HandleTypeDef htim1;         /* Généré par CubeMX : TIM1_CH1, PWM (PA8) */
extern TIM_HandleTypeDef htim3;         /* Généré par CubeMX : TIM3_CH2, Input Capture (tachymètre) */

/* Private constants ---------------------------------------------------------*/

/**
 * Fréquence de comptage de TIM3, en Hz (1 tick = 1 µs).
 * @warning Doit correspondre au prescaler configuré dans CubeMX.
 */
#define FAN_TACH_TIMER_HZ       (1000000U)

/** Nombre d'impulsions tachymètre par tour du ventilateur (selon fiche technique). */
#define FAN_TACH_PULSES_PER_REV (2U)

/** Valeur attendue de TIM3->ARR : le calcul de période suppose un compteur 16 bits complet. */
#define FAN_TACH_EXPECTED_ARR   (0xFFFFU)

/* Private variables ---------------------------------------------------------*/

/* Partie commande (thread uniquement) */
static bool     s_initialized     = false;
static uint8_t  s_speedPercent    = 0U;     /* Dernière consigne appliquée */

/* Partie tachymètre : écrite par l'ISR, lue par Fan_GetRpm() */
static volatile bool     s_hasReference  = false;   /* Une capture précédente existe */
static volatile bool     s_periodValid   = false;   /* s_periodTicks est exploitable */
static volatile uint16_t s_lastCapture   = 0U;      /* Dernière valeur capturée (16 bits) */
static volatile uint32_t s_periodTicks   = 0U;      /* Période entre 2 impulsions, en ticks */
static volatile uint32_t s_lastPulseMs   = 0U;      /* HAL_GetTick() de la dernière impulsion */

/* Exported functions --------------------------------------------------------*/

Fan_Status_t Fan_Init(void)
{
    if (s_initialized)
    {
        return FAN_OK;
    }

    /* Garde-fou : le calcul de période en 16 bits n'est valide que si ARR = 0xFFFF. */
    if (__HAL_TIM_GET_AUTORELOAD(&htim3) != FAN_TACH_EXPECTED_ARR)
    {
        return FAN_ERROR;
    }

    /* Deux appels HAL = deux tests séparés (un "&&" entre les retours masquerait une erreur). */
    if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK)
    {
        return FAN_ERROR;
    }

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0U);   /* Départ à 0 % */

    if (HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2) != HAL_OK)
    {
        (void)HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);  /* Retour à un état connu */
        return FAN_ERROR;
    }

    s_speedPercent = 0U;
    s_initialized  = true;

    return FAN_OK;
}

Fan_Status_t Fan_SetSpeedPercent(uint8_t percent)
{
    if (!s_initialized)
    {
        return FAN_NOT_INITIALIZED;
    }

    if (percent > FAN_SPEED_PERCENT_MAX)
    {
        return FAN_INVALID_PARAM;
    }

    /* Le rapport cyclique vaut CCR / (ARR + 1). En mode PWM 1, CCR > ARR donne 100 %.
     * On lit ARR dans le timer au lieu de le recopier : si CubeMX change, le driver suit. */
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim1) + 1U;
    uint32_t ccr    = ((uint32_t)percent * period) / FAN_SPEED_PERCENT_MAX;

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr);
    s_speedPercent = percent;

    return FAN_OK;
}

Fan_Status_t Fan_GetSpeedPercent(uint8_t *p_percent)
{
    if (p_percent == NULL)
    {
        return FAN_INVALID_PARAM;
    }

    if (!s_initialized)
    {
        return FAN_NOT_INITIALIZED;
    }

    *p_percent = s_speedPercent;

    return FAN_OK;
}

Fan_Status_t Fan_GetRpm(uint32_t *p_rpm)
{
    if (p_rpm == NULL)
    {
        return FAN_INVALID_PARAM;
    }

    *p_rpm = 0U;    /* Valeur sûre par défaut pour tous les chemins d'erreur */

    if (!s_initialized)
    {
        return FAN_NOT_INITIALIZED;
    }

    /* Copie cohérente des variables partagées avec l'ISR (section critique très courte). */
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    bool     valid   = s_periodValid;
    uint32_t period  = s_periodTicks;
    uint32_t lastMs  = s_lastPulseMs;
    __set_PRIMASK(primask);

    if (!valid || (period == 0U))
    {
        return FAN_NO_TACH;
    }

    if ((HAL_GetTick() - lastMs) > FAN_TACH_TIMEOUT_MS)
    {
        return FAN_NO_TACH;     /* Signal perdu : la période mémorisée est périmée */
    }

    /* 60 * 1e6 = 6e7 : tient dans 32 bits. */
    *p_rpm = (60U * FAN_TACH_TIMER_HZ) / (period * FAN_TACH_PULSES_PER_REV);

    return FAN_OK;
}

void Fan_OnTachCapture(void)
{
    uint32_t now_ms  = HAL_GetTick();
    uint16_t capture = (uint16_t)HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_2);

    if (s_hasReference && ((now_ms - s_lastPulseMs) <= FAN_TACH_TIMEOUT_MS))
    {
        /* Soustraction non signée en 16 bits : le débordement du compteur
         * (65535 -> 0) est géré automatiquement par l'arithmétique modulo 2^16. */
        s_periodTicks = (uint16_t)(capture - s_lastCapture);
        s_periodValid = true;
    }
    else
    {
        /* Première impulsion, ou reprise après un arrêt : pas de période fiable. */
        s_periodValid = false;
    }

    s_lastCapture = capture;
    s_lastPulseMs = now_ms;
    s_hasReference = true;
}
