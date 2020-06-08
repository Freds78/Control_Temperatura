/*
 * Buzzer.c
 *
 *  Created on: 10 may. 2020
 *      Author: freds
 */

#include "sapi.h"
#include "Buzzer.h"


void Pwm_Buzzer(Pwm_t *Pwm){

	Pwm->dutyCycle = 0;/* 0 a 255 */
	pwmWrite( PWM7, Pwm->dutyCycle );
	delayWrite( &Pwm->delayBase, 50 );

	if( delayRead(&Pwm->delayBase) ){
		Pwm->dutyCycle++;
		if( Pwm->dutyCycle > 255 ){
			Pwm->dutyCycle = 0;
		}
	}
	pwmWrite( PWM7, Pwm->dutyCycle );


}





