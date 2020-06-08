/*=============================================================================
 * Author: Freds Amundaray Cruz <freds.amundaray@gmail.com>
 * Date: 2020/04/28
 * Version: 1
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/
#include "Leds.h"
#include "Buzzer.h"
#include "sensorTemp.h"
#include "sapi.h"
#include <stdint.h>
#include <stdbool.h>
#include "ff.h"       // <= Biblioteca FAT FS
#include "fssdc.h"	// API de bajo nivel para unidad "SDC:" en FAT FS
#include <math.h>
/*=====[Definition macros of private constants]==============================*/
#define AMARILLO LED1
#define ROJO LED2
#define VERDE LED3
#define READ_ROMCOM 0x33
#define MATCH_ROMCOM 0x55
#define SEARCH_ROMCOM 0xF0
#define ALARM_SEARCHCOM 0xEC
#define SKIP_ROMCOM 0xCC
#define CONVERT_TEMP 0x44
#define COPY_SCRATCH 0x48
#define WRITE_SCRATCH 0x4E
#define READ_SCRATCH 0xBE
#define RE_CALLE2 0xB8
#define READ_POWERSUP 0xB4
#define FILENAME "SDC:/Temperature.txt"


/*=====[Definitions of extern global variables]==============================*/

static Pwm_t Pwm;
static  FIL fp;             // <-- File object needed for each open file
static rtc_t rtc = {
   2020,
   5,
   18,
   4,
   20,
   10,
   0
};
/*=====[Definitions of public global variables]==============================*/

/*=====[Definitions of private global variables]=============================*/

static void Read_RomCommand(conection_t *pcom);
static void Skip_RomCommand(conection_t *pcom);
static void Convert_Temperature(conection_t *pcom);
static void Read_Scratchpad(conection_t *pcom);
static void Copy_Scratchpad(conection_t *pcom);
static uint8_t crc8 (conection_t *pcom);
static bool_t inicialize_secuence();

static void control_Temperature(conection_t *pcom);
static void writeRegister(conection_t *pcom);

static void send_bit(conection_t *pcom);
static bool_t read_bit();
static void send_byte(conection_t *pcom);
static uint8_t read_byte(conection_t *pcom);





static void Copy_Scratchpad(conection_t *pcom){

	int delay_time = 750; //94 para resolucion de 9
	pcom->Bytesend = COPY_SCRATCH;
	send_byte(pcom);
	gpioConfig( GPIO1, GPIO_INPUT_PULLUP );// colocar el puerto como entra  Rx

	while(gpioRead( GPIO1 ) != 1){
		delay(delay_time);
	}
}

static void Write_Scratchpad(conection_t *pcom){

	pcom->Bytesend = WRITE_SCRATCH;
	send_byte(pcom);
	pcom->Bytesend = pcom->TH;
	send_byte(pcom);
	pcom->Bytesend = pcom->TL;
	send_byte(pcom);
	pcom->Bytesend = pcom->Conf_Register;
	send_byte(pcom);
	inicialize_secuence(pcom);

}

static void Read_RomCommand(conection_t *pcom){
	pcom->Bytesend = READ_ROMCOM;
	send_byte(pcom);

	int8_t i, j;
	for (j = 0; j < 8; j++){
		for (i = 0; i < 8; i++) {
			pcom->rom_data[j][i] = read_byte(pcom);
		}
	}

	if (crc8(pcom) != 0) {
		printf("\tROM data read ERROR.");
		inicialize_secuence(pcom);
	}
}


static void Skip_RomCommand(conection_t *pcom){
	 pcom->Bytesend = SKIP_ROMCOM;
	 send_byte(pcom);
}


static void Convert_Temperature(conection_t *pcom){

	int delay_time = 750; //94 para resolucion de 9
	pcom->Bytesend = CONVERT_TEMP;
	send_byte(pcom);
	gpioConfig( GPIO1, GPIO_INPUT_PULLUP );// colocar el puerto como entra  Rx

	while(gpioRead( GPIO1 ) != 1){
		delay(delay_time);
	}
}


static void Read_Scratchpad(conection_t *pcom){
	/* Reads the whole scratchpad to the buffer (9 bytes) */

	pcom->Bytesend = READ_SCRATCH;
	send_byte(pcom);
	int8_t i, j;
	for(j = 0; j<9; j++){
		for (i = 0; i < 8; i++) {
			pcom->scratchpad[j][i] = read_byte(pcom);
		}
	}

	if (crc8(pcom) != 0){
		printf("\tScratchpad data read ERROR, try again.\n");
		inicialize_secuence(pcom);
	}
}


static uint8_t crc8 (conection_t *pcom) {
 uint8_t div = 0b10001100; // Rotated poly
 uint8_t crc = 0;

 int8_t i,j;
 for(j=0; i< 8 ; j++){
	 for (i = 0; i < 8; i++) {
		 uint8_t byte = pcom->scratchpad[j][i];

		 int8_t j;
		 for (j = 0; j < 8; j++) {

			 // Shift CRC
			 uint8_t crc_carry = crc & 1;
			 crc = crc >> 1;

			 // Shift Byte
			 uint8_t byte_carry = byte & 1;
			 byte = byte >>1;

			 // If crc_carry XOR byte_carry we make crc XOR div
			 if (crc_carry ^ byte_carry)
				 crc ^= div;
		 }
	 }
 }
 return crc;
}

/* Sends 8 bit in a row, LSB first */
static void send_byte(conection_t *pcom) {
 int8_t i;

 for (i = 0; i < 8; i++) {
	 pcom->Bitsend = pcom->Bytesend & 1;
	 send_bit(pcom);
	 pcom->Bytesend = pcom->Bytesend >> 1;
 }
}

static void send_bit(conection_t *pcom) {

	gpioConfig( GPIO1, GPIO_OUTPUT );
	gpioWrite( GPIO1, 0 );
	delay(0.003);

    if (pcom->Bitsend) {
    	gpioWrite( GPIO1, 1 );
    	delay(0.060);//55

    } else {
    	delay(0.060);//55
    	gpioWrite( GPIO1, 1 );
    	delay(0.010);
    }
}

/* Reads a byte, LSB first */
static uint8_t read_byte(conection_t *pcom) {
 pcom->Byteread = 0;
 int8_t i;
 uint8_t j;

 for (i=0; i < 8; i++) {

  j = read_bit();
  j = j << i;
  pcom->Byteread = pcom->Byteread | j;
 }

 return pcom->Byteread;
}

static bool_t read_bit() {
	bool_t read;
	gpioConfig( GPIO1, GPIO_OUTPUT );
	gpioWrite( GPIO1, 0 );
	delay(0.003);
	gpioConfig( GPIO1, GPIO_INPUT_PULLUP );
	delay(0.010);
	read = gpioRead( GPIO1 );				// pin->read();
	delay(0.055); //45
	return read;
}


// FUNCION que se ejecuta cada vezque ocurre un Tick
 void diskTickHook( void *ptr )
{
   disk_timerproc();   // Disk timer process
}

static void writeRegister(conection_t *pcom){
	rtcRead( &rtc );
	if( f_open( &fp, FILENAME, FA_WRITE | FA_OPEN_APPEND ) == FR_OK ){
		UINT nbytes;
		UINT n = sprintf( pcom->bufferRegistre, "Temperature is %3.1fC %04d-%02d-%02d %02d:%02d:%02d\r\n",
				pcom-> answer,
				rtc.year,
				rtc.month,
				rtc.mday,
				rtc.hour,
				rtc.min,
				rtc.sec
		);
		f_write( &fp, pcom->bufferRegistre, n, &nbytes );

		f_close(&fp);

		if( nbytes == n ){
			printf("Escribio correctamente\n");
			gpioWrite( LEDG, ON );
		} else {
			gpioWrite( LEDR, ON );
			printf("Escribio %d bytes\n", nbytes);
		}
	} else{
		printf("Error al abrir el archivo\n");
		gpioWrite( LEDR, ON );
	}

}

static void control_Temperature(conection_t *pcom){


	if(pcom-> answer  < 0
			 || pcom-> answer  > 10){ //pcom->TL pcom->TH

		 delayWrite(&pcom->contador, 1000);
		 if(delayRead(&pcom->contador)){
		 		turnOff(AMARILLO);
		 }else{
		 		turnOn(AMARILLO);
		 }
		 Pwm_Buzzer(&Pwm);
	 }
}




  void sensorInit(conection_t *pcommand){

  	 rtcWrite( &rtc ); // Establecer fecha y hora
  	 delayWrite(&pcommand->contador,2000);
  	 if(delayRead(&pcommand->contador)){
  	 }

   }
 static bool_t inicialize_secuence(){


	    bool_t presence = FALSE;
	    gpioConfig( GPIO1, GPIO_OUTPUT );
	    gpioWrite( GPIO1, 0 );
	    delay(0.500);
	    gpioConfig( GPIO1, GPIO_INPUT_PULLUP );
	    delay(0.090);
	    if (gpioRead( GPIO1 )==0) // see if any devices are pulling the data line low
	        presence = TRUE;
	    delay(0.410);
	    return presence;

 }


 void masterTx_Control(conection_t *pcommand){

	volatile float remaining_count, count_per_degree;
	volatile int reading;

	 if (inicialize_secuence(pcommand)) {
		 printf ("Device present!\n");
		 Skip_RomCommand(pcommand);
		 Convert_Temperature(pcommand);
	 }else {
		 printf  ("Device not present, no response :(\n");
		 inicialize_secuence(pcommand);

	 }
	 if (inicialize_secuence(pcommand)) {
		 printf ("Device present!\n");
		 Skip_RomCommand(pcommand);
		 Read_Scratchpad(pcommand);

		 reading = (pcommand->scratchpad[1][8] << 8) + pcommand->scratchpad[0][8];


		 //	remaining_count = pcommand->scratchpad[6][8];
		 //	count_per_degree = pcommand->scratchpad[7][8];
		 //	pcommand->answer = floor(pcommand->answer/2.0f) - 0.25f + (count_per_degree - remaining_count) / count_per_degree;

		 if (reading & 0x8000) { // negative degrees C
			 reading = 0-((reading ^ 0xffff) + 1); // 2's comp then convert to signed int
		 }

		 pcommand->answer = reading +0.0; // convert to floating point
		 pcommand->answer = pcommand->answer / 16.0f;
		 printf("It is %3.1fC\r\n", pcommand->answer );
		 delay(1000);


	 }else {
		 printf  ("Device not present, no response :(\n");
		 inicialize_secuence(pcommand);
	 }

 }


