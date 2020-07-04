//Projekt i wykonanie: Maciej Śliwiński, 2016

////////////////////////////////////////////////////////////////////////////////////////////////////
//Urządzenie do pracy na dwóch kanałach. Usterka w konstrukcji nie pozwala na pracę na więcej
//niż na jednym kanale (domowa produkcja PCB, urządzenie pracuje w pudełku po butach) :).
//Pomiar temperatury gotowy, wystarczy zaaplikowac kod.
//**To moje pierwsze wykonane kompletne urządzenie.


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "HD44780x.h"

#define VREF 29.0

float adc_0, adc_1= 0.0;			//zmienna do obliczeń napięcia
float zadana_0, zadana_1 = 2.5;		//zmienna zadanej wartości napięcia dla wy
char wynik_0[]="           ";		//bufor tekstowy, wyczyszczenie bufora
char wynik_1[]="           ";
uint8_t step = 0;				//zmienna kroku regulacji napięcia na wy
uint8_t val=0, val_tmp =0;		//zmienne dla enkodera
uint8_t enc;					//zmienna obsługi enkodera
uint8_t mode;
uint8_t ch;
uint8_t loop;

void main(void)
{
LCD_Initalize();   				//inicjalizacja LCD
LCD_GoTo(2, 0);      			//Ustawienie kursora w pozycji (0,0)
LCD_WriteText("POWER SUPPLY");		//tekst powitalny

timer1_init();					//inicjalizacja	timera1

/////////////////////////////     kierunki portów /////////////
DDRB |= (1<<PB1) | (1<<PB6);
PORTB|= (1<<PB3) | (1<<PB4) |
		(1<<PB5);
/////////////////////////////
DDRC |= (1<<PC5);
/////////////////////////////
DDRD |= (1<<PD1);
PORTD|= (1<<PD0) | (1<<PD2) |
		(1<<PD3);
////////////////////////////////////////////////////////////////

LED_init(); 					//sygnał LED - włączenie urządzenia

sei(); 							//Globalne uruchomienie przerwań

ICR1 = 0xFFFF;					//PWM MAX
OCR1A = 0;
OCR1B = 0;

_delay_ms(1000);
LCD_Clear();

ADC_init();						//inicjalizacja ADC
//timer2_init();					//inicjalizacja timera2

////////////////////////////////////////////////////////////////////////////////////////////////////

for(;;)									//główna pętla programu
{
	switch(mode)	//obsługa trybów pracy
	{
	/////////////////////       ///////////////      ////////////     ////////////////////////////
	case 0:
		val_tmp = read_gray_code_from_encoder();

				/////////////////////////////////////////////////////////////////////
				///////////////////////     obsługa enkodera      ///////////////////
	    if(val != val_tmp)
	    {
	    	if	( /*(val==2 && val_tmp==3) ||*/
	    		(val==3 && val_tmp==1) ||
				  /*(val==1 && val_tmp==0) ||*/
	     	   	(val==0 && val_tmp==2)
	     	   	)
	    	{
	    		if(step==0)
	    		{
	    		enc++;
	     	   		if(enc == 2)
	     	   		{
	     	   		if(!ch){ zadana_0++; }
	     	   		if(ch){ zadana_1++; }
	     	   		OCR1B = OCR1B + 100;
	         	   	enc = 0;
	     	   		}
	    		}
	    		if(step==1)
	     	   	{
	     	   	enc++;
	     	   		if(enc == 2)
	     	   		{
	     	   		if(!ch){ zadana_0 = zadana_0 + 0.1; }
	     	   		if(ch){ zadana_1 = zadana_1 + 0.1; }
	         	   	enc = 0;
	     	   		}
	    	   	}
	     	}
	    	else if (/*(val==3 && val_tmp==2) ||*/
	     			(val==2 && val_tmp==0) ||
	     	   		 /*(val==0 && val_tmp==1) ||*/
	     	   		(val==1 && val_tmp==3)
	     	   	   	)
	    	{
	    		if(step==0)
		   		{
	    		enc++;
	     	   		if(enc == 2)
	     	   		{
	     	   		if(!ch){ zadana_0--; }
	     	   		if(ch){ zadana_1--; }
	         	   	enc = 0;
	     	   		}
		   		}
		   		if(step==1)
		   		{
	    	   	enc++;
	     	   		if(enc == 2)
	     	   		{
	     	   		if(!ch){ zadana_0 = zadana_0 - 0.1; }
	     	   		if(ch){ zadana_1 = zadana_1 - 0.1; }
	         	   	enc = 0;
	     	   		}
		   		}
	     	 }

	    	val = val_tmp;
	    }

	    LCD_GoTo(0, 0);
	    LCD_WriteText("CH1");
	    LCD_GoTo(0, 1);
	    LCD_WriteText("CH2");
	    LCD_GoTo(15, 0);
	    LCD_WriteText("V");
	    LCD_GoTo(15, 1);
	    LCD_WriteText("V");

	    if (!ch)						//wyświetlanie znaku strzałki przy wybranym kanale
		{
	    	LCD_GoTo(3, 0);
	    	LCD_WriteData(0b01111111);
		}
	    if (ch)
	    {
	    	LCD_GoTo(3, 1);
	    	LCD_WriteData(0b01111111);
	    }

	    if(zadana_0 < 9.95)
	    {
	    	LCD_GoTo(14, 0);
	    	LCD_WriteText(" ");
	    }
	    if(zadana_1 < 9.95)
	    {
	    	LCD_GoTo(14,1);
	    	LCD_WriteText(" ");
	    }
	    sprintf(wynik_0,"%1.1f",zadana_0);   	//konwersja na łańcuch znakowy
	    sprintf(wynik_1,"%1.1f",zadana_1);
	    LCD_GoTo(11, 0);
	    LCD_WriteText(wynik_0);   			//Wyświetlenie wyniku pomiaru ADC
	    LCD_GoTo(11, 1);
	    LCD_WriteText(wynik_1);
	break; //case 0 end


	///////////////////// STEP          STEP      STEP      STEP   ///////////////////////////////
	case 1:
    	LCD_GoTo(3, 0);

    	switch(step)											//gdy step
    	{ case 0: LCD_WriteText("STEP 1 V     "); 	break;		//0 - step 1V   |wyświetl zmianę
    	  case 1: LCD_WriteText("STEP 0.1 V   "); 	break; }	//1 - step 0.1V |wyswietl zmianę
    	_delay_ms(200);											//czas wyświetlania zmiany stanu
    	mode = 0;
    	LCD_Clear();

	break; //case 1 end

	////////////////////  RT             RT            RT      ////////////////////////////////////
	case 2:
		LCD_GoTo(0, 0);

	break; //case 2 end

	} //switch end

	///////////////////////////////////////////////////////////////////////////////////////////////////
								/////////// wybór kanału /////////////////////
	if(!(PINB & 0b00001000))
		{								//obsługa przycisku 1
		ch ^= 1;
		//LCD_Clear();
			do							//debouncing
			{
				asm("nop");
				LCD_Clear();
			}while(!(PINB & 0x08));
		}

	if(!(PINB & 0x08))					//obsługa przycisku 2
	{
		PORTC ^= (1<<PC5);
		do								//debouncing
		{
			asm("nop");
		}while(!(PINB & 0x08));
	}


	if(!(PINB & 0x20))     ////////////   obsługa przycisku 3, funkcja step   /////////////////////
    {
        LCD_GoTo(0, 0);
        LCD_Clear();					//lcd zeruj
    	step ^= 1;						//zmień stan step (xor)
    	mode = 1;
		do								//debouncing
		{
			asm("nop");
		}while(!(PINB & 0x08));
    }

    if(!(PINB & 0x10))					//obsługa przycisku 4
    {
    	if(mode == 2) { mode = 0; }
    	mode = 2;
    	do								//debouncing
    	{
    		asm("nop");
    	}while(!(PINB & 0x10));
    }
}
}//main end

/////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////     PRZERWANIA       ///////////////////////////////////////////

ISR(ADC_vect)								//obsługa przerwania po zakończeniu konwersji ADC
{

	adc_0=ADC*VREF/1023; 	//przeliczenie wyniku pomiaru na napiecie


	if(adc_0 > zadana_0)
	{
		OCR1A--;						//porównywanie pomiaru ADC na kanale do wartości zadanej
	}

	if(adc_1 > zadana_1)
	{
		OCR1B--;
	}

	if(adc_0 < zadana_0)
	{
		OCR1A++;
	}									// ^ automatyczna regulacja napiecia do wartości zadanej

	if(adc_1 < zadana_1)
	{
		OCR1B++;
	}
	if(OCR1A < 0)
	{
		OCR1A = 0;
	}

	if(OCR1B < 0)
	{
		OCR1B = 0;
	}

	if(OCR1A >= ICR1)
	{
		OCR1A = 0xFFFE;
	}

	if(OCR1B >= ICR1)
	{
		OCR1B = 0x00;
	}

	if(zadana_0 < 2.5)
	{
		zadana_0 = 2.5;
	}

	if(zadana_1 < 2.5)
	{
		zadana_1 = 2.5;
	}

	if(zadana_0 > 26)
	{
		zadana_0 = 26;
	}

	if(zadana_1 > 26)
		{
			zadana_1 = 26;
		}

	//loop++;

} //isr end

