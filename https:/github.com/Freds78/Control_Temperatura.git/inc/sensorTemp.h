/*=============================================================================
 * Author: Freds Amundaray Cruz <freds.amundaray@gmail.com>
 * Date: 2020/04/28
 * Version: 1
 *===========================================================================*/

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef __S_TEMP_H__
#define __S_TEMP_H__

/*=====[Inclusions of public function dependencies]==========================*/

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "sapi.h"

/*=====[C++ - begin]=========================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definition macros of public constants]===============================*/


/*=====[Public function-like macros]=========================================*/

/*=====[Definitions of public data types]====================================*/


typedef struct{
	delay_t contador; // delay para temporizar cambios
	char bufferRegistre[100];
	uint8_t Byteread;
	uint8_t Bytesend;
	uint8_t Bitread;
	uint8_t Bitsend;
	uint8_t scratchpad[9][8];
	uint8_t rom_data[8][8];
	uint8_t TH;
	uint8_t TL;
	uint8_t Conf_Register;
	uint8_t cont;
	float answer;
}conection_t;

/*=====[Prototypes (declarations) of public functions]=======================*/
void masterTx_Control(conection_t *command);
void sensorInit(conection_t *command);
void diskTickHook( void *ptr );

/*=====[Prototypes (declarations) of public interrupt functions]=============*/

/*=====[C++ - end]===========================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Avoid multiple inclusion - end]======================================*/

#endif /* __S_TEMP_H__ */
