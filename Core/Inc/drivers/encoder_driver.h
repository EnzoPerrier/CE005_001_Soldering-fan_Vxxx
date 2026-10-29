/**
 * @file    encoder_driver.h
 * @brief   Driver de l'encodeur rotatif (TIM2 en mode encodeur) et de son bouton.
 * @author  Enzo Perrier
 * @version 0.3
 *
 * Ce module encapsule l'accès matériel à TIM2 configuré en mode encodeur.
 * Le code applicatif n'a jamais à connaître le handle htim2 ni le HAL.
 *
 * @note    Le driver ne dépend d'aucun thread ni d'aucun RTOS : l'appui sur
 *          le bouton est signalé par un callback enregistré par l'application.
 */
#ifndef ENCODER_DRIVER_H
#define ENCODER_DRIVER_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Codes de retour du driver encodeur.
 * @note  Enum propre au module, pour ne pas exposer HAL_StatusTypeDef
 *        et pouvoir exprimer des erreurs métier (ex: non initialisé).
 */
typedef enum
{
    ENCODER_OK = 0,             /*!< Opération réussie */
    ENCODER_ERROR,              /*!< Erreur matérielle lors de l'appel HAL */
    ENCODER_NOT_INITIALIZED,    /*!< Encoder_Init() n'a pas été appelée avant usage */
    ENCODER_INVALID_PARAM       /*!< Paramètre invalide (ex: pointeur NULL) */
} Encoder_Status_t;

/**
 * @brief   Prototype du callback appelé lors d'un appui bouton valide.
 * @warning Appelé en CONTEXTE D'INTERRUPTION : aucune fonction bloquante,
 *          seules les fonctions FreeRTOS "FromISR" sont autorisées.
 */
typedef void (*Encoder_ButtonCallback_t)(void);

/**
 * @brief   Démarre le timer en mode encodeur.
 * @note    Idempotent : un appel sur un driver déjà initialisé retourne
 *          ENCODER_OK sans effet.
 * @retval  ENCODER_OK     Démarrage réussi (ou déjà effectué).
 * @retval  ENCODER_ERROR  L'appel HAL sous-jacent a échoué.
 */
Encoder_Status_t Encoder_Init(void);

/**
 * @brief   Met à jour et retourne la position cumulée de l'encodeur.
 * @warning Modifie l'état interne du module (mémorisation de la dernière
 *          valeur brute du compteur). Cette fonction ne doit être appelée
 *          que par UN SEUL thread (le thread UI).
 * @note    La position est exprimée en impulsions (et non en crans) :
 *          si l'encodeur produit N impulsions par cran, la conversion
 *          est à faire côté appelant.
 * @param[out] p_position  Position cumulée depuis Encoder_Init(). Ne doit pas être NULL.
 * @retval  ENCODER_OK               Lecture réussie, *p_position est valide.
 * @retval  ENCODER_NOT_INITIALIZED  Encoder_Init() n'a pas été appelée.
 * @retval  ENCODER_INVALID_PARAM    p_position est NULL.
 */
Encoder_Status_t Encoder_GetPosition(int32_t *p_position);

/**
 * @brief   Enregistre le callback d'appui bouton.
 * @param[in] callback  Fonction à appeler, ou NULL pour désactiver.
 */
void Encoder_RegisterButtonCallback(Encoder_ButtonCallback_t callback);

/**
 * @brief   Traite l'interruption EXTI du bouton (anti-rebond logiciel inclus).
 * @note    Réservé à irq_dispatch.c, ne pas appeler ailleurs.
 */
void Encoder_OnButtonIrq(void);

#endif /* ENCODER_DRIVER_H */
