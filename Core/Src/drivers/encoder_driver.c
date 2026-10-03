/**
 ******************************************************************************
 * @author  Enzo Perrier
 * @file    encoder_driver.c
 * @brief   Driver bas niveau pour l'encodeur rotatif (LPTIM3 en mode Encoder)
 *
 * Ce module encapsule l'accès matériel au LPTIM3 configuré en mode encodeur.
 * Il expose une interface indépendante de l'implémentation HAL sous-jacente,
 * afin que le code applicatif (menu, app_state...) n'ait jamais à connaître
 * le handle hlptim3 ni les détails d'accès registre.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32g4xx_hal.h"
#include "drivers/encoder_driver.h"


#include <stdbool.h>

/* Extern variables ---------------------------------------------------------*/
// ----- Encodeur rotatif
extern LPTIM_HandleTypeDef hlptim2;

// ----- Encodeur SW (BP)
extern TX_EVENT_FLAGS_GROUP ui_events_group;

/* Private constants ---------------------------------------------------------*/

// Taille du cycle du compteur matériel du LPTIM3 (Period = 65535 dans MX_LPTIM3_Init,
// donc le registre boucle sur 16 bits, de 0 à 65535, puis repart à 0)
#define ENCODER_RAW_COUNTER_RANGE   (65536)
#define ENCODER_RAW_COUNTER_HALF    (ENCODER_RAW_COUNTER_RANGE / 2) // Moitié du cycle, seuil de détection du débordement

// Délai minimum entre deux appuis valides, en millisecondes
#define BUTTON_DEBOUNCE_MS   (50)

/* Private macros ------------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
// ----- Encodeur rotatif
static bool encoder_status = false;
static uint16_t valeur_brute_prec = 0; // Dernière valeur BRUTE (0-65535) lue du registre, jamais autre chose
static int32_t position_cumulee = 0;  // Position cumulée totale, retournée à l'appelant (pas de limite pratique)



/* Private function prototypes -----------------------------------------------*/


/* Exported functions --------------------------------------------------------*/

/**
 * @brief  				 Initialisation du module.
 * @retval ENCODER_OK    Encodeur initialisé.
 * @retval ENCODER_ERROR Initialization failed.
 */
Encoder_Status_t Encoder_Init(void)
{
	if(encoder_status) // Si l'encodeur est déjà initialisé
	{
		return ENCODER_OK;
	}

	if(HAL_LPTIM_Encoder_Start(&hlptim3) != HAL_OK)
	{
		encoder_status = false;
		return ENCODER_ERROR;
	}

	// Synchronise la valeur précédente avec le compteur matériel.
	valeur_brute_prec = (int32_t)HAL_LPTIM_ReadCounter(&hlptim3);
	position_cumulee = 0;
	encoder_status = true;
	return ENCODER_OK;
}

/**
 * @brief 				 		   Cette fonction permet de récupérer la position cumulée de l'encodeur.
 * @param[out] p_position  		   Pointeur vers la variable qui recevra la position lue. Ne doit pas être NULL.
 * @retval ENCODER_OK    		   Lecture OK.
 * @retval ENCODER_NOT_INITIALIZED driver non initialisé.
 * @retval ENCODER_INVALID_PARAM   paramètre invalide passé dans la fonction (ex: pointeur = NULL).
 */
Encoder_Status_t Encoder_GetPosition(int32_t *p_position)
{
	uint16_t valeur_brute_act = 0;
	int32_t delta = 0;

	if(p_position == NULL) // Si adresse du pointeur = NULL
	{
		return(ENCODER_INVALID_PARAM);
	}

	if(!encoder_status) // Si l'encodeur n'a pas été initialisé
	{
		return(ENCODER_NOT_INITIALIZED);
	}

	valeur_brute_act = (uint16_t)HAL_LPTIM_ReadCounter(&hlptim3); // Valeur brute (0-65535), comparable à valeur_brute_prec

	delta = valeur_brute_act - valeur_brute_prec;

	// Corrige le delta en cas de passage par 0/65535.
	if(delta > ENCODER_RAW_COUNTER_HALF) // Si le delta est supérieur à la moitié de 2^16 (32768)
	{
		delta -= ENCODER_RAW_COUNTER_RANGE;
	}
	else if(delta < -ENCODER_RAW_COUNTER_HALF)
	{
		delta += ENCODER_RAW_COUNTER_RANGE;
	}
	else
	{
		// Pas de débordement, le delta calculé est déjà correct
	}

	position_cumulee += delta; // On accumule le mouvement corrigé dans la position cumulée

	*p_position = position_cumulee; // On stock à l'adresse de la variable (*p_pointeur) la position cumulée de l'encodeur

	valeur_brute_prec = valeur_brute_act; // On garde uniquement la valeur BRUTE pour le prochain calcul de delta (jamais la position cumulée !)

	return ENCODER_OK;
}

/**
 * @brief Handle the encoder button external interrupt.
 * @details
 * This callback is called by the STM32 HAL when a GPIO external interrupt
 * occurs. If the interrupt comes from the encoder push button, the
 * corresponding UI event flag is set in the ThreadX event flags group.
 * @param[in] GPIO_Pin GPIO pin that triggered the external interrupt.
 * @retval None
 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	static uint32_t last_button_press_tick = 0; // Variable utilisée pour le debounce logiciel du BP encodeur
	uint32_t now = HAL_GetTick();

    if (GPIO_Pin == ENCOD_SW_Pin)
    {
    	if ((now - last_button_press_tick) >= BUTTON_DEBOUNCE_MS) // Debounce logiciel
    	{
    		tx_event_flags_set(&ui_events_group, UI_EVENT_BUTTON_PRESSED, TX_OR);
    		last_button_press_tick = now;
    	}

    }
}


/* Private functions ---------------------------------------------------------*/

