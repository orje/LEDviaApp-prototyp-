#include "stdafx.h"
#include "stdint.h"

const uint16_t PIXELS = 8;			// Anzahl der LED			(PIXELS)
const uint16_t zero = PIXELS + 1;	// shifted Zero
const uint16_t pixels = 2 * PIXELS + 1;	// shifted PIXELS
const uint16_t run_nr = 4;			// Anzahl der aktiven Gruppe (run_nr)
uint16_t led_x;						// LED Zeiger				(led_x)
uint16_t led_index;					// LED Index				(led_index)
uint16_t run_nr_index;				// LED Gruppenindex			(run_nr_index)
bool minus = false;					// false = vorwärts, true = rückwärts
uint16_t ar[PIXELS] = { 0 };		// LED Tabelle
uint16_t m;							// Index für drucke
void drucke(void);					// printf debug
int dummy;

/*----------------------------------------------------------------------------*/

void main(void)
{
	printf("PIXELS = %d \t run_nr = %d \n \n", PIXELS, run_nr);
start:
	if (!minus)
	{
		for (led_x = zero; led_x <= pixels - run_nr; led_x++)	// exit action
		{
			printf("start led_x++ = %d \n", led_x);
			for (led_index = zero; led_index <= pixels - run_nr; led_index++)
				// entry action
			{
				printf("\t");
				printf("start led_index++ = %d \n", led_index);
				if (led_index == led_x)
				{
					printf("\t");
					printf("led_index = %d == led_x = %d \n", led_index, led_x);
					for (run_nr_index = 0; run_nr_index < run_nr; 
						run_nr_index++, led_index++) // led_index muss mitzählen
					{
						printf("\t \t");
						printf("start run_nr_index++ = %d \n", run_nr_index);
						printf("\t \t \t");
						printf("write led_index [%d] = 1", led_index);
						printf("\n");
						ar[led_index - PIXELS - 1] = 1;		// Zellenwert = 1	
					}
					led_index--;					// Korrektur wg. 2*led_index
					printf("\t \t");
					printf("end run_nr_index++ = %d \n", run_nr_index);
				}
				else
				{
					printf("\t");
					printf("led_index != led_x");
					printf("\n");
					printf("\t \t \t");
					printf("write led_index [%d] = 0", led_index);
					printf("\n");
					ar[led_index - PIXELS - 1] = 0;			// Zellenwert = 0
				}
				printf("\t");
				printf("end led_index++ = %d \n", led_index);
			}
			drucke();
			dummy = 0;
		}
		printf("**********");
		printf("\n");
		printf("end led_x++ = %d \n", led_x);
		printf("**********");
		printf("\n");
		printf("\n");
		minus = true;
	}
	if (minus)
	{
		for (led_x = pixels; led_x >= run_nr + zero; led_x--) // exit action
		{
			printf("start led_x-- = %d \n", led_x);
			for (led_index = pixels; led_index >= run_nr + zero; led_index--)
				// entry action
			{
				printf("\t");
				printf("start led_index-- = %d \n", led_index);
				if (led_index == led_x)
				{
					printf("\t");
					printf("led_index = %d == led_x = %d \n", led_index, led_x);
					for (run_nr_index = run_nr; run_nr_index > 0; 
						run_nr_index--, led_index--) // led_index muss mitzählen
					{
						printf("\t \t");
						printf("start run_nr_index-- = %d \n", run_nr_index);
						printf("\t \t \t");
						printf("write led_index [%d] = 1", led_index);
						printf("\n");
						ar[led_index - PIXELS - 2] = 1;		// Zellenwert = 1
					}
					led_index++;					// Korrektur wg. 2*led_index
					printf("\t \t");
					printf("end run_nr_index-- = %d \n", run_nr_index);
				}
				else
				{
					printf("\t");
					printf("led_index != led_x");
					printf("\n");
					printf("\t \t \t");
					printf("write led_index [%d] = 0", led_index);
					printf("\n");
					ar[led_index - PIXELS - 2] = 0;			// Zellenwert = 0
				}
				printf("\t");
				printf("end led_index-- = %d \n", led_index);
			}
			drucke();
			dummy = 0;
		}
		printf("**********");
		printf("\n");
		printf("end led_x-- = %d \n", led_x);
		printf("**********");
		printf("\n");
		printf("\n");
		minus = false;
	}
	goto start;
}

void drucke(void)
{
	printf("----------");
	printf("\n");
	printf("show = ");
	for (m = 0; m < PIXELS; m++) {
		printf("%d ", ar[m]);
	}
	printf("\n");
	printf("----------");
	printf("\n \n");
}