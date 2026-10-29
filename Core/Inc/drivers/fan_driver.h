/**
 * @file    fan_driver.h
 * @brief   Driver du ventilateur : commande PWM (TIM1_CH1) et mesure tachymètre (TIM3_CH2).
 * @author  Enzo Perrier
 * @version 0.1
 *
 * Ce module encapsule l'accès matériel aux timers du ventilateur. Le code
 * applicatif n'a jamais à connaître les handles htim1 / htim3 ni le HAL.
 *
 * @note    Le driver ne dépend d'aucun thread ni d'aucun RTOS.
 */
#ifndef FAN_DRIVER_H
#define FAN_DRIVER_H

#include <stdint.h>

/** @brief Vitesse maximale acceptée par Fan_SetSpeedPercent(), en pourcentage. */
#define FAN_SPEED_PERCENT_MAX   (100U)

/**
 * @brief Codes de retour du driver ventilateur.
 * @note  Enum propre au module, pour ne pas exposer HAL_StatusTypeDef.
 */
typedef enum
{
    FAN_OK = 0,             /*!< Opération réussie */
    FAN_ERROR,              /*!< Erreur matérielle lors d'un appel HAL */
    FAN_NOT_INITIALIZED,    /*!< Fan_Init() n'a pas été appelée avant usage */
    FAN_INVALID_PARAM,      /*!< Paramètre invalide (pointeur NULL, pourcentage > 100) */
    FAN_NO_TACH             /*!< Aucun signal tachymètre valide (arrêt, débranché, ou ventilateur sans tachymètre) */
} Fan_Status_t;

/**
 * @brief   Démarre la sortie PWM (à 0 %) et la capture du tachymètre.
 * @note    Idempotent : un appel sur un driver déjà initialisé retourne
 *          FAN_OK sans effet.
 * @note    À appeler après MX_TIM1_Init() et MX_TIM3_Init().
 * @retval  FAN_OK     Démarrage réussi (ou déjà effectué).
 * @retval  FAN_ERROR  Échec d'un appel HAL (PWM ou capture).
 */
Fan_Status_t Fan_Init(void);

/**
 * @brief   Applique une consigne de vitesse au ventilateur.
 * @note    Une valeur hors plage est REJETÉE (pas de saturation silencieuse) :
 *          la consigne précédente reste appliquée.
 * @param[in] percent  Vitesse en pourcentage, de 0 (arrêt) à FAN_SPEED_PERCENT_MAX (100).
 * @retval  FAN_OK               Consigne appliquée.
 * @retval  FAN_NOT_INITIALIZED  Fan_Init() n'a pas été appelée.
 * @retval  FAN_INVALID_PARAM    percent est supérieur à FAN_SPEED_PERCENT_MAX.
 */
Fan_Status_t Fan_SetSpeedPercent(uint8_t percent);

/**
 * @brief   Retourne la dernière consigne de vitesse appliquée.
 * @note    Il s'agit de la CONSIGNE, pas de la vitesse réelle (voir Fan_GetRpm()).
 * @param[out] p_percent  Consigne courante en pourcentage. Ne doit pas être NULL.
 * @retval  FAN_OK               Lecture réussie, *p_percent est valide.
 * @retval  FAN_NOT_INITIALIZED  Fan_Init() n'a pas été appelée.
 * @retval  FAN_INVALID_PARAM    p_percent est NULL.
 */
Fan_Status_t Fan_GetSpeedPercent(uint8_t *p_percent);

/**
 * @brief   Retourne la vitesse de rotation mesurée par le tachymètre.
 * @note    Le calcul est effectué dans cette fonction, jamais dans l'interruption.
 * @note    Sans nouvelle impulsion depuis FAN_TACH_TIMEOUT_MS, le signal est
 *          considéré perdu : la dernière période mémorisée n'est PAS utilisée.
 * @param[out] p_rpm  Vitesse en tours par minute. Ne doit pas être NULL.
 *                    Valeur forcée à 0 si le retour est FAN_NO_TACH.
 * @retval  FAN_OK               Mesure valide, *p_rpm est valide.
 * @retval  FAN_NOT_INITIALIZED  Fan_Init() n'a pas été appelée.
 * @retval  FAN_INVALID_PARAM    p_rpm est NULL.
 * @retval  FAN_NO_TACH          Pas de mesure valide (première capture non reçue, ou signal perdu).
 */
Fan_Status_t Fan_GetRpm(uint32_t *p_rpm);

/**
 * @brief   Traite une capture du tachymètre (appelé à chaque front détecté).
 * @warning Appelé en CONTEXTE D'INTERRUPTION : code court, aucune division,
 *          aucune fonction bloquante.
 * @note    Réservé à irq_dispatch.c, ne pas appeler ailleurs. Le filtrage
 *          de l'instance (TIM3) et du canal (CH2) est fait par l'appelant.
 */
void Fan_OnTachCapture(void);

#endif /* FAN_DRIVER_H */
