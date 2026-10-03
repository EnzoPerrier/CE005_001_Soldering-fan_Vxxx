/**
 ******************************************************************************
 * @author  Enzo Perrier
 * @file    encoder_driver.h
 * @brief   Driver bas niveau pour l'encodeur rotatif (LPTIM3 en mode Encoder)
 *
 * Ce module encapsule l'accès matériel au LPTIM3 configuré en mode encodeur.
 * Il expose une interface indépendante de l'implémentation HAL sous-jacente,
 * afin que le code applicatif (menu, app_state...) n'ait jamais à connaître
 * le handle hlptim3 ni les détails d'accès registre.
 ******************************************************************************
 */

/* ------------------------------------------------------------------------ */
/* INCLUDE GUARD                                                          */
/* ------------------------------------------------------------------------ */
/*
 * Un .h peut être inclus (via #include) par plusieurs fichiers .c différents.
 * Sans protection, si deux fichiers incluent tous les deux ce .h (directement
 * ou indirectement), le compilateur verrait deux fois les mêmes déclarations
 * et générerait une erreur de "redéfinition".
 *
 * Le mécanisme ci-dessous dit au préprocesseur :
 * "si ENCODER_DRIVER_H n'est pas encore défini, on le définit et on inclut
 * tout le contenu ; sinon, on saute tout le fichier."
 * C'est un standard absolu en C, à mettre dans TOUS tes .h.
 */
#ifndef ENCODER_DRIVER_H
#define ENCODER_DRIVER_H

/* ------------------------------------------------------------------------ */
/* INCLUDES                                                               */
/* ------------------------------------------------------------------------ */
/*
 * On inclut stdint.h pour avoir accès aux types explicites (uint8_t,
 * int32_t...) exigés par les règles de codage industriel (voir point 1
 * de notre discussion précédente : pas de "int"/"char" bruts).
 */
#include <stdint.h>

/* ------------------------------------------------------------------------ */
/* TYPES PUBLICS EXPOSÉS PAR LE MODULE                                   */
/* ------------------------------------------------------------------------ */

/**
 * @brief Codes de retour du driver encodeur.
 *
 * On définit notre PROPRE enum de statut plutôt que de réutiliser
 * directement HAL_StatusTypeDef, pour deux raisons :
 *  - Le code appelant (menu.c, app_state.c) n'a pas besoin de savoir que
 *    ce driver utilise du HAL en interne (encapsulation : demain, si ce
 *    driver est réécrit pour un autre MCU sans HAL, l'interface ne bouge
 *    pas).
 *  - On peut exprimer des erreurs propres à NOTRE métier, comme
 *    "pas encore initialisé", qui n'existe pas dans HAL_StatusTypeDef.
 */
typedef enum // typedef enum auto incrémente (ENCODER_OK = 0, ENCODER_ERROR = 1, ENCODER_NOT_INITIALIZED = 2, ENCODER_INVALID_PARAM = 3
{
    ENCODER_OK = 0,             /*!< Opération réussie */
    ENCODER_ERROR,              /*!< Erreur matérielle lors de l'appel HAL */
    ENCODER_NOT_INITIALIZED,    /*!< Le driver n'a pas été initialisé avant usage */
    ENCODER_INVALID_PARAM       /*!< Paramètre invalide passé à la fonction (ex: pointeur NULL) */
} Encoder_Status_t;


/* ------------------------------------------------------------------------ */
/* PROTOTYPES DES FONCTIONS PUBLIQUES                                    */
/* ------------------------------------------------------------------------ */

/**
 * @brief  Initialise et démarre le driver encodeur.
 * @note   Doit être appelée une seule fois, avant tout appel à
 *         Encoder_GetPosition().
 * @retval ENCODER_OK    si le démarrage matériel a réussi
 * @retval ENCODER_ERROR si l'appel HAL sous-jacent a échoué
 */
Encoder_Status_t Encoder_Init(void);

/**
 * @brief  Lit la position courante de l'encodeur.
 *
 * @note   Cette fonction est une lecture pure : elle ne modifie aucun état
 *         interne du module (pas d'effet de bord), conformément aux règles
 *         de codage industriel évoquées.
 *
 * @param[out] p_position  Pointeur vers la variable où sera écrite la
 *                          position lue. Ne doit pas être NULL.(
 *
 * @retval ENCODER_OK                 la lecture a réussi, *p_position est valide
 * @retval ENCODER_NOT_INITIALIZED    Encoder_Init() n'a pas été appelée avant
 * @retval ENCODER_INVALID_PARAM      p_position est NULL
 */
Encoder_Status_t Encoder_GetPosition(int32_t *p_position);

#endif /* ENCODER_DRIVER_H */
