/*
 * Buzzer.h
 *
 *  Created on: 10 may. 2020
 *      Author: freds
 */

#ifndef MYPROGRAM_S_TEMP_INC_BUZZER_H_
#define MYPROGRAM_S_TEMP_INC_BUZZER_H_


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "sapi.h"


typedef struct {
	uint8_t dutyCycle; // ciclo de trabajo
	delay_t delayBase; // delay para temporizar cambios en las luces
} Pwm_t;

void Pwm_Buzzer(Pwm_t *Pwm);

#endif /* MYPROGRAM_S_TEMP_INC_BUZZER_H_ */
