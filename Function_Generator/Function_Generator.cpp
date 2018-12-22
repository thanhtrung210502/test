// UTFT_Demo_320x240
// Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
// web: http://www.RinkyDinkElectronics.com/
//
// This program is a demo of how to use most of the functions
// of the library with a supported display modules.
//
// This demo was made for modules with a screen resolution
// of 320x240 pixels.
//
// This program requires the UTFT library.
//

//################################################
// GLUE class that implements the UTFT API
// replace UTFT include and constructor statements
// remove UTFT font declaration e.g. SmallFont
//################################################

#include <Arduino.h>
#include <UTFTGLUE.h>              //use GLUE class and constructor
#include "Fonts/FreeMonoBold12pt7b.h"
#include "Fonts/FreeMonoBold24pt7b.h"

#define Pin_BTN1	11
#define Pin_BTN2	12

#define Def_PRESS		1
#define	Def_HOLD		2

#define Def_SLOW_INC	8


UTFTGLUE myGLCD(0, A2, A1, A3, A4, A0); //all dummy args

// Declare which fonts we will be using
//extern uint8_t SmallFont[];      //GLUE defines as GFXFont ref

// Set the pins to the correct ones for your development shield
// ------------------------------------------------------------
// Arduino Uno / 2009:
// -------------------
// Standard Arduino Uno/2009 shield            : <display model>,A5,A4,A3,A2
// DisplayModule Arduino Uno TFT shield        : <display model>,A5,A4,A3,A2
//
// Arduino Mega:
// -------------------
// Standard Arduino Mega/Due shield            : <display model>,38,39,40,41
// CTE TFT LCD/SD Shield for Arduino Mega      : <display model>,38,39,40,41
//
// Remember to change the model parameter to suit your display module!
//UTFT myGLCD(ITDB32S,38,39,40,41);

int16_t vrEncoderCount = 0;
int16_t vrEncoderCountPre = 0;
int16_t vrEncoderDelta = 0;
int16_t vrEncoderDeltaAbs = 0;

int16_t vrFre = 1250;
int16_t vrFrePre = 1250;
uint16_t vrCount = 10;
char str[16];
uint8_t vr_Test = 10;
uint16_t vrBTN1Cnt = 0;
uint8_t vrBTN1Stt = 0;

uint16_t vrBTN2Cnt = 0;
uint8_t vrBTN2Stt = 0;

uint16_t vrBtn1HoldCnt = 0;
uint16_t vrBtn2HoldCnt = 0;
uint8_t vrDelta1 = 10;
uint8_t vrDelta2 = 10;
void EX_ISP();

void setup()
{
	pinMode(10, OUTPUT);
	pinMode(13, OUTPUT);

	pinMode(Pin_BTN1, INPUT);
	pinMode(Pin_BTN2, INPUT);
	Serial.begin(115200);

//	attachInterrupt(digitalPinToInterrupt(2), EX_ISP, FALLING);
	TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (1<<COM1B1) | (0<<COM1B0) | (1<<WGM11) | (0<<WGM10);
	TCNT1H=0x00;
	TCNT1L=0x00;

	ASSR=(0<<EXCLK) | (0<<AS2);
	TCCR2A=(0<<COM2A1) | (0<<COM2A0) | (0<<COM2B1) | (0<<COM2B0) | (0<<WGM21) | (0<<WGM20);
	TCCR2B=(0<<WGM22) | (1<<CS22) | (0<<CS21) | (0<<CS20);
	TCNT2=0x06;
	OCR2A=0x00;
	OCR2B=0x00;

	// Timer/Counter 2 Interrupt(s) initialization
	TIMSK2=(0<<OCIE2B) | (0<<OCIE2A) | (1<<TOIE2);

	// Global enable interrupts
	sei();

	randomSeed(analogRead(0));

	// Setup the LCD
	myGLCD.InitLCD();
	myGLCD.setFont(&FreeMonoBold12pt7b);

	char str[16];

	myGLCD.clrScr();

	myGLCD.setColor(255, 255, 255);
	myGLCD.fillRoundRect(0, 0, 319, 239);

	myGLCD.setColor(255, 255, 176);
	myGLCD.fillRoundRect(20, 15, 300, 225);

	myGLCD.setColor(20, 0, 0);
	myGLCD.drawRoundRect(20, 15, 300, 225);

	myGLCD.print("Function Generator", CENTER, 30);

	myGLCD.setColor(255, 0, 0);
	myGLCD.setFont(&FreeMonoBold24pt7b);
	sprintf(str, "%d.%d Hz", vrFre, 0);
	myGLCD.print(str, CENTER, 65);

//	// Draw sin-, cos- and tan-lines
	myGLCD.setColor(0, 0, 255);
	for (int i = 0; i<122; i++)
	{
		myGLCD.drawPixel(110+i, 170 + 0.25*(sin(((i*6.13)*3.14) / 180) * 95));
		myGLCD.drawPixel(110+i, 171 + 0.25*(sin(((i*6.13)*3.14) / 180) * 95));
		myGLCD.drawPixel(110+i, 172 + 0.25*(sin(((i*6.13)*3.14) / 180) * 95));
		myGLCD.drawPixel(110+i, 173 + 0.25*(sin(((i*6.13)*3.14) / 180) * 95));
		myGLCD.drawPixel(110+i, 174 + 0.25*(sin(((i*6.13)*3.14) / 180) * 95));
	}
	myGLCD.drawLine(105, 170, 232, 170);
	myGLCD.drawLine(105, 171, 232, 171);
	myGLCD.drawLine(105, 172, 232, 172);

	vrEncoderCount = 100;
	vrEncoderCountPre = 100;

}

void loop()
{

	if (vrBtn1HoldCnt < Def_SLOW_INC)
		vrDelta1 = 10;
	else {
		vrBtn1HoldCnt = Def_SLOW_INC;
		vrDelta1 = 30;
	}

	if (vrBtn2HoldCnt < Def_SLOW_INC)
		vrDelta2 = 10;
	else {
		vrBtn2HoldCnt = Def_SLOW_INC;
		vrDelta2 = 30;
	}

	if (Def_PRESS == vrBTN1Stt) {
		vrBTN1Stt = 0;
		vrFre--;
	}
	else if (Def_HOLD == vrBTN1Stt) {
		vrBtn1HoldCnt++;
		vrBTN1Stt = 0;
		vrFre -= vrDelta1;
	}

	if (Def_PRESS == vrBTN2Stt) {
		vrBTN2Stt = 0;
		vrFre++;
	}
	else if (Def_HOLD == vrBTN2Stt) {
		vrBtn2HoldCnt++;
		vrBTN2Stt = 0;
		vrFre += vrDelta2;
	}

	if (vrFre < 0)
		vrFre = 2000;
	if (vrFre > 2000)
		vrFre = 0;

	if (vrFre >= 250) {
		vrCount = 16000000/vrFre;
		TCCR1B=(0<<ICNC1) | (0<<ICES1) | (1<<WGM13) | (1<<WGM12) | (0<<CS12) | (0<<CS11) | (1<<CS10);

	}
	else {
		vrCount = 62500/vrFre;
		TCCR1B=(0<<ICNC1) | (0<<ICES1) | (1<<WGM13) | (1<<WGM12) | (1<<CS12) | (0<<CS11) | (0<<CS10);
	}
	ICR1H=vrCount/256;
	ICR1L=vrCount%256;

	OCR1BH=(vrCount/2)/256;
	OCR1BL=(vrCount/2)%256;

	Serial.print("vrBtn1HoldCnt: ");
	Serial.println(vrBtn1HoldCnt);

	if (vrFre != vrFrePre) {
		myGLCD.setColor(255, 255, 176);
		myGLCD.fillRoundRect(30, 60, 285, 120);

		myGLCD.setColor(255, 0, 0);
		sprintf(str, "%d.%d Hz", vrFre, 0);
		myGLCD.print(str, CENTER, 65);
	}

//	vr_Test++;
	vrFrePre = vrFre;
//	delay(10);
}

ISR (TIMER2_OVF_vect)
{
	TCNT2=0x06;

	// BTN1 Begin
	if (!digitalRead(Pin_BTN1)) {
		vrBTN1Cnt++;
		if (vrBTN1Cnt == 10) {
			vrBTN1Stt = Def_PRESS;
		}
		if (vrBTN1Cnt > 1200) {
			vrBTN1Stt = Def_HOLD;
			vrBTN1Cnt = 1190;
		}
	}
	else {
		vrBtn1HoldCnt = 0;
		vrBTN1Cnt = 0;
	}
	// BTN1 End

	// BTN2 Begin
	if (!digitalRead(Pin_BTN2)) {
		vrBTN2Cnt++;
		if (vrBTN2Cnt == 10) {
			vrBTN2Stt = Def_PRESS;
		}
		if (vrBTN2Cnt > 1200) {
			vrBTN2Stt = Def_HOLD;
			vrBTN2Cnt = 1190;
		}
	}
	else {
		vrBtn2HoldCnt = 0;
		vrBTN2Cnt = 0;
	}
	// BTN2 End

	digitalWrite(13, !digitalRead(13));
}

void EX_ISP() {

	if ( digitalRead(3) ) {
		vrEncoderCount++;
		if (vrEncoderCount > 2000)
			vrEncoderCount = 2000;
	}
	else {
		vrEncoderCount--;
		if (vrEncoderCount < 0)
			vrEncoderCount = 0;
	}
}
