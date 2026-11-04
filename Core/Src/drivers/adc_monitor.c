/**
 ******************************************************************************
 * @file    adc_monitor.c
 * @brief   Implémentation du driver de surveillance des tensions. Doc publique : voir adc_monitor.h
 * @author  Enzo Perrier
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"                       /* Inclut stm32g4xx_hal.h et les labels CubeMX */
#include "drivers/adc_monitor.h"

#include <stdbool.h>

/* Extern variables ----------------------------------------------------------*/
extern ADC_HandleTypeDef hadc2;         /* Généré par CubeMX : ADC2, scan 2 rangs (voir note ci-dessous) */

/* Private constants ---------------------------------------------------------*/

/*
 * Configuration CubeMX attendue pour ADC2 (à vérifier, le driver en dépend) :
 *   - Rang 1 : ADC2_IN17 (PA4) -> batterie
 *   - Rang 2 : ADC2_IN13 (PA5) -> alimentation
 *   - Number Of Conversion = 2 (le mode scan est implicite sur STM32G4)
 *   - Resolution 12 bits, Continuous = Disabled, Discontinuous = Disabled
 *   - End Of Conversion Selection = End of single conversion
 *   - Data alignment = Right, trigger = logiciel
 *   - Sampling Time long (ex: 92.5 ou 247.5 cycles) : pont diviseur = source haute impédance
 */

/** Tension de référence analogique (VDDA), en mV. À ajuster selon la carte. */
#define ADCMON_VREF_MV              (3300U)

/** Pleine échelle de l'ADC en 12 bits. */
#define ADCMON_FULL_SCALE           (4095U)

/** Pont diviseur batterie : R haute (vers la batterie) et R basse (vers GND), en ohms. */
#define ADCMON_BATT_R_TOP_OHM       (100000U)   /* A MODIFIER selon le schéma */
#define ADCMON_BATT_R_BOT_OHM       (33000U)    /* A MODIFIER selon le schéma */

/** Pont diviseur alimentation : R haute et R basse, en ohms. */
#define ADCMON_SUPPLY_R_TOP_OHM     (100000U)   /* A MODIFIER selon le schéma */
#define ADCMON_SUPPLY_R_BOT_OHM     (160000U)    /* A MODIFIER selon le schéma */

/** Nombre de mesures moyennées pour réduire le bruit. */
#define ADCMON_AVG_SAMPLES          (8U)

/** Timeout d'une conversion, en ms. */
#define ADCMON_TIMEOUT_MS           (10U)

/* Private variables ---------------------------------------------------------*/
static bool s_initialized = false;

/* Private function prototypes -----------------------------------------------*/
static uint32_t RawToMillivolts(uint32_t raw, uint32_t r_top_ohm, uint32_t r_bot_ohm);
static AdcMon_Status_t ReadRawOnce(uint32_t *p_raw_batt, uint32_t *p_raw_supply);

/* Exported functions --------------------------------------------------------*/

AdcMon_Status_t AdcMon_Init(void)
{
    if (s_initialized)
    {
        return ADCMON_OK;
    }

    if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK)
    {
        return ADCMON_ERROR;
    }

    s_initialized = true;

    return ADCMON_OK;
}

AdcMon_Status_t AdcMon_Read(AdcMon_Measurement_t *p_meas)
{
    if (p_meas == NULL)
    {
        return ADCMON_INVALID_PARAM;
    }

    if (!s_initialized)
    {
        return ADCMON_NOT_INITIALIZED;
    }

    uint32_t sum_batt   = 0U;
    uint32_t sum_supply = 0U;

    for (uint32_t i = 0U; i < ADCMON_AVG_SAMPLES; i++)
    {
        uint32_t raw_batt;
        uint32_t raw_supply;

        if (ReadRawOnce(&raw_batt, &raw_supply) != ADCMON_OK)
        {
            return ADCMON_ERROR;    /* *p_meas non modifié */
        }

        sum_batt   += raw_batt;
        sum_supply += raw_supply;
    }

    p_meas->battery_mv = RawToMillivolts(sum_batt / ADCMON_AVG_SAMPLES,
                                         ADCMON_BATT_R_TOP_OHM,
                                         ADCMON_BATT_R_BOT_OHM);

    p_meas->supply_mv  = RawToMillivolts(sum_supply / ADCMON_AVG_SAMPLES,
                                         ADCMON_SUPPLY_R_TOP_OHM,
                                         ADCMON_SUPPLY_R_BOT_OHM);

    return ADCMON_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Convertit une valeur brute en tension avant pont diviseur.
 * @details V_pin = raw * VREF / 4095 ; V_reelle = V_pin * (R_top + R_bot) / R_bot.
 *          Calcul en 64 bits pour éviter tout dépassement intermédiaire.
 * @param[in] raw        Valeur ADC (0 à 4095).
 * @param[in] r_top_ohm  Résistance haute du pont, en ohms.
 * @param[in] r_bot_ohm  Résistance basse du pont, en ohms (non nulle).
 * @return    Tension réelle en mV.
 */
static uint32_t RawToMillivolts(uint32_t raw, uint32_t r_top_ohm, uint32_t r_bot_ohm)
{
    uint64_t numerator   = (uint64_t)raw * ADCMON_VREF_MV * (r_top_ohm + r_bot_ohm);
    uint64_t denominator = (uint64_t)ADCMON_FULL_SCALE * r_bot_ohm;

    return (uint32_t)(numerator / denominator);
}

/**
 * @brief   Effectue une séquence de conversion (2 voies) et lit les valeurs brutes.
 * @param[out] p_raw_batt    Valeur brute rang 1 (PA4).
 * @param[out] p_raw_supply  Valeur brute rang 2 (PA5).
 * @retval ADCMON_OK     Les deux conversions ont abouti.
 * @retval ADCMON_ERROR  Erreur HAL ou timeout.
 */
static AdcMon_Status_t ReadRawOnce(uint32_t *p_raw_batt, uint32_t *p_raw_supply)
{
    AdcMon_Status_t status = ADCMON_OK;

    if (HAL_ADC_Start(&hadc2) != HAL_OK)
    {
        return ADCMON_ERROR;
    }

    /* Rang 1 : batterie */
    if (HAL_ADC_PollForConversion(&hadc2, ADCMON_TIMEOUT_MS) == HAL_OK)
    {
        *p_raw_batt = HAL_ADC_GetValue(&hadc2);
    }
    else
    {
        status = ADCMON_ERROR;
    }

    /* Rang 2 : alimentation */
    if ((status == ADCMON_OK) &&
        (HAL_ADC_PollForConversion(&hadc2, ADCMON_TIMEOUT_MS) == HAL_OK))
    {
        *p_raw_supply = HAL_ADC_GetValue(&hadc2);
    }
    else
    {
        status = ADCMON_ERROR;
    }

    (void)HAL_ADC_Stop(&hadc2);     /* Toujours arrêter l'ADC, même en cas d'erreur */

    return status;
}
