/**
 ******************************************************************************
 * @file    adc_monitor.h
 * @brief   Driver de surveillance des tensions : batterie (PA4) et alimentation (PA5).
 * @author  Enzo Perrier
 * @version 0.1
 *
 * Ce module encapsule la lecture de ADC2 (2 voies en scan) et la conversion
 * en millivolts (pont diviseur inclus). Le code applicatif ne manipule que
 * des millivolts, jamais de valeurs brutes ni le handle hadc2.
 *
 * @note    Le driver ne dépend d'aucun thread ni d'aucun RTOS.
 * @warning Les fonctions de lecture ne sont pas réentrantes : les appeler
 *          depuis UN SEUL thread.
 ******************************************************************************
 */
#ifndef ADC_MONITOR_H
#define ADC_MONITOR_H

#include <stdint.h>

/**
 * @brief Codes de retour du driver de surveillance des tensions.
 */
typedef enum
{
    ADCMON_OK = 0,              /*!< Opération réussie */
    ADCMON_ERROR,               /*!< Erreur matérielle ou timeout lors d'un appel HAL */
    ADCMON_NOT_INITIALIZED,     /*!< AdcMon_Init() n'a pas été appelée avant usage */
    ADCMON_INVALID_PARAM        /*!< Paramètre invalide (pointeur NULL) */
} AdcMon_Status_t;

/**
 * @brief Résultat d'une mesure : tensions réelles AVANT pont diviseur, en millivolts.
 */
typedef struct
{
    uint32_t battery_mv;        /*!< Tension batterie (PA4), en mV */
    uint32_t supply_mv;         /*!< Tension d'alimentation (PA5), en mV */
} AdcMon_Measurement_t;

/**
 * @brief   Calibre l'ADC2 (mode single-ended) et le rend utilisable.
 * @note    Idempotent : un appel sur un driver déjà initialisé retourne
 *          ADCMON_OK sans effet.
 * @note    À appeler après MX_ADC2_Init().
 * @retval  ADCMON_OK     Calibration réussie (ou déjà effectuée).
 * @retval  ADCMON_ERROR  Échec de la calibration.
 */
AdcMon_Status_t AdcMon_Init(void);

/**
 * @brief   Mesure les deux tensions (moyenne de ADCMON_AVG_SAMPLES échantillons).
 * @note    Fonction bloquante, durée de l'ordre de la centaine de microsecondes.
 * @param[out] p_meas  Résultat de la mesure. Ne doit pas être NULL.
 *                     Non modifié si le retour n'est pas ADCMON_OK.
 * @retval  ADCMON_OK               Mesure valide.
 * @retval  ADCMON_NOT_INITIALIZED  AdcMon_Init() n'a pas été appelée.
 * @retval  ADCMON_INVALID_PARAM    p_meas est NULL.
 * @retval  ADCMON_ERROR            Timeout ou erreur HAL pendant la conversion.
 */
AdcMon_Status_t AdcMon_Read(AdcMon_Measurement_t *p_meas);

#endif /* ADC_MONITOR_H */
