/*
 * ShiftReg with string.c
 *
 * Created: 25/01/2022 11:39:16 AM
 * This program takes in three strings each seven characters long and converts them into ASCII hex characters then uses an custom library(FinalCahrlibrary.h) to fetch the relevant letter or number.
 * To print the characters to the LED matrix it first stores each character in an array to then send each character to the LED matrix.
 * To make the character visible the program lights up each row at a time but it is done quickly enough that the whole character appears on the screen to the human eye.
 *
 * Note
 * a. The program was designed with the 74HC595 shift which has a data pin,shift clock, store clock and output enable(currently connected to ground).
 * b. All the Shift registers(Shr) share the data pin and the store clock pin but each has its own shift clock.
 * c. There are four functions used to control the shift registers:
 *		1. LetterToRetrieve - This function is defined in the "FinlCharLibrary.h" it takes in a ASCII hex character and a row number and returns an 8-bit number that represents the character given on that row
 *		2. ShrInitial(Shift register initialize) - This function assumes that all the shift registers are connected in in series on a single port, therefore it only takes the port, the first pin and the last 
 *			pin connected to the Shr and sets all the pins as outputs.
 *		2. pulse - This function takes in a pin number and a port and sets the pin on that port high for 10ms then low.
 *		3. ShrWrite(Shift register Write) - This function takes in the ascii hex character, shift pin of the Shr, port of the register and the row to be displayed on the register.
 *			It shifts the data retrieved from the LetterToRetrieve function to the shift register through the data pin one bit at a time as it pulses the shift clock.
 *		4. PrintString - This function takes in three separate strings and stores each character in three separate arrays(Note: the function removes any white spaces in the string), each character is 
 *			then sent to the ShrWrite function along with the shift pin, port and the row number.
 */ 

#include <avr/io.h>
#define F_CPU 10000UL
#include <util/delay.h>
#include "FinalCharLibrary.h"

//Macros
#define ShrPrtData PORTC //Port of the shift register shared data pin and store clock.
#define ShrData 0		 //Pin used for the shift register data.
#define Storeck 1		 //Pin used for the shift register store clock.
#define regSet1 PORTC	 //Port of the first shift register.
#define regSet2 PORTB	 //Port of the second shift register.
#define regSet3 PORTD	 //Port of the third shift register.
#define clockdelay 10

void ShrInitial(volatile uint8_t *ddr, int fnum, int lnum);								//Function to initialize the shift registers.
void pulse(int r, volatile uint8_t *prt1);												//function to pulse a pin.
void ShrWrite(char singleCharacter, int pinno, volatile uint8_t *prt2, int rowNumber);	//Function to shift data to the shift register storage one bit at a time.
void  PrintString(char *string, char *string1, char *string2);							//Function to print a string to the shift register.

//--------------------------------------------------Main function start-------------------------------------------------------------------
int main(void) {
	*(&ShrPrtData -1) |= (1 << Storeck) | (1 << ShrData);	//Setting the shared storage clock and the data pins as outputs.
	ShrInitial(&regSet1, 2, 7);								//Initializing the first set of shift registers.
	ShrInitial(&regSet2, 0, 7);								//Initializing the second set of shift registers.
	ShrInitial(&regSet3, 0, 7);								//Initializing the third set of shift registers.
	while (1) {
			PrintString("KAA 850K", "1029384", "MESSAGE"); //Print three strings to the shift register.
	}
}
//--------------------------------------------------Main function end-------------------------------------------------------------------


void ShrInitial(volatile uint8_t *ddr, int fnum, int lnum) {
	for (int i = fnum; i <= lnum; i++){		//For loop that sets i to the pin of the first pin specified, the loop is stopped by the last pin specified
		*(ddr - 1) |= (1 << i);				//Sets the pin at i on the port provided as an output
	}
}

void pulse(int r, volatile uint8_t *prt1) {
	*prt1 |= (1 << r);						//Sets the pin on the provided port to high
	_delay_us(clockdelay);
	*prt1 &= ~(1 << r);						//Sets the pin on the provided port to low
}
void ShrWrite(char singleCharacter, int pinno, volatile uint8_t *prt2, int rowNumber) {
	uint8_t byte = LetterToRetrieve(singleCharacter, rowNumber);	//Stores the 8-bit number returned from the function LetterToRetrieve in a variable called byte
	for (int i = 0; i < 8; i++) {									//A loop that runs 8 times to send each bit to the shift register one at a time
		if (byte & 0b10000000) {									//Checking if the 8th bit is 1 if true set the data pin to high else set the data pin to low
			ShrPrtData |= (1 << ShrData);							//Setting the data pin to high
		}
		else {
			ShrPrtData &= ~(1 << ShrData);							//Setting the data pin to low
		}
		byte = byte << 1;											//Shifts the 8-bit number 1 bit to the left so that when the loop runs again it will be checking the seventh bit
		pulse(pinno,prt2);											//pulsing the shift clock so that whatever state the data pin was in will now be shifted into the shift register
	}
}
void  PrintString(char *string, char *string1, char *string2) {
	char charArray[7];								//Store the seven characters received from the first string in an array.
	char charArray1[7];								//Store the seven characters received from the second string in an array.
	char charArray2[7];								//Store the seven characters received from the third string in an array.
	for (int i = 0; i<7; i++) {					//A loop to repeat seven times to go through the seven characters.
		//Checking if any of to characters being received are spaces(" ") and if they are they are skipped.
		//Store the character from the first string in the array at position 1.
		//Move to the next character These three steps are repeated thrice.
		if (*string == 0x20) string++;			
		charArray[i] = *string;					
		string++;								
		if (*string1 == 0x20) string1++;
		charArray1[i] = *string1;
		string1++;
		if (*string2 == 0x20) string2++;
		charArray2[i] = *string2;
		string2++;
		
	}
	for (int j = 0; j<8; j++) {			//A loop to go through all the characters stored in the array 8 times to print each row to the LCD.
		ShrWrite(Rtd[j], 2, &regSet1, j); //Controls the row that is visible.
		
		//Characters from the first array to be shifted to the shift register.
		ShrWrite(charArray[0], 3, &regSet1, j);
		ShrWrite(charArray[1], 4, &regSet1, j);
		ShrWrite(charArray[2], 5, &regSet1, j);
		ShrWrite(charArray[3], 6, &regSet1, j);
		ShrWrite(charArray[4], 7, &regSet1, j);
		ShrWrite(charArray[5], 0, &regSet2, j);
		ShrWrite(charArray[6], 1, &regSet2, j);
		
		//Characters from the second array to be shifted to the shift register.
		ShrWrite(charArray1[0], 2, &regSet2, j);
		ShrWrite(charArray1[1], 3, &regSet2, j);
		ShrWrite(charArray1[2], 4, &regSet2, j);
		ShrWrite(charArray1[3], 5, &regSet2, j);
		ShrWrite(charArray1[4], 6, &regSet2, j);
		ShrWrite(charArray1[5], 7, &regSet2, j);
		ShrWrite(charArray1[6], 0, &regSet3, j);
		
		//Characters from the third array to be shifted to the shift register.
		ShrWrite(charArray2[0], 1, &regSet3, j);
		ShrWrite(charArray2[1], 2, &regSet3, j);
		ShrWrite(charArray2[2], 3, &regSet3, j);
		ShrWrite(charArray2[3], 4, &regSet3, j);
		ShrWrite(charArray2[4], 5, &regSet3, j);
		ShrWrite(charArray2[5], 6, &regSet3, j);
		ShrWrite(charArray2[6], 7, &regSet3, j);
		pulse(Storeck, &regSet1);					//Pulsing the storage clock.
		//_delay_ms(1);
	}
}
