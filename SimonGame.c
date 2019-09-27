/*
 * Author:  Jaryd Hodge         
 *           
 * Purpose:  Plays games of Simon with the user, a memory game where the user is
 * shown a pattern of lights and has to repeat the sequence of lights in the correct order
 */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <DAQlib.h>	
#include <time.h>

#define TRUE 1
#define FALSE 0

#define ON 1
#define OFF 0

/*Channel numbers for switches and LEDs*/
#define GREEN 0
#define RED 1
#define YELLOW 2
#define BLUE 3

/*One second in miliseconds*/
#define ONE_SECOND 1000

/*Time in seconds a light should flash for when playing patterns*/
#define BLINK_SPEED 0.5

/*Time in seconds a light should flash for when indicating 
the user has won or lost*/
#define NEW_GAME_BLINK_SPEED 0.8

/*Time in seconds that the program should wait between games*/
#define NEW_GAME_TIME 4

/*Length of time in seconds between the user pressing the buttons and
the next pattern being displayed*/
#define NEXT_CYCLE 1

/*Time in seconds to wait before starting the first game*/
#define FIRST_GAME_DELAY 2

/*Number of LEDs and push buttons*/
#define NUM_LEDS 4

/*Simulator view to use*/
#define SETUP_NUM 6

/*Length of memory pattern*/
#define PATTERN_LENGTH 5

/*Funtion prototypes*/
void playGame(void);
void generatePattern(int pattern[], int pattern_length);
int getRandom(int lower, int upper);
int playRound(int pattern[], int pattern_length);
void winLose(int result);
void blinkNumTimes(int light, int numTimes, double time);
void playPattern(int pattern[], int cycle_length);
int playCycle(int pattern[], int cycle_num, int pattern_length);
int readPattern(int pattern[], int cycle_Num);
int checkNextNum(int pattern[], int nextNum);
int waitForButtonPress(void);
void waitForButtonRelease(void);
int checkNextNum(int pattern[], int nextNum, int index);




int main(void)
{
	srand((unsigned)time(NULL));
	
	if (setupDAQ(SETUP_NUM) == TRUE) {
		playGame();
	}
	else {
		printf("Error initializing DAQ.");
	}
	
	/*Don't need this as there is nothing being displayed on the screen
	although I left it commented because I wasn't sure if this was required*/

	//system("PAUSE");

	
	return 0;
}



/*Work function, plays multiple games of simon with the user 
until the user quits*/

void playGame(void) {
	
	/*Waits a certain amount of time before starting the first game*/
	Sleep(FIRST_GAME_DELAY * ONE_SECOND);


	while (continueSuperLoop() == TRUE) {
		int pattern[PATTERN_LENGTH];
		int count, result;

		/*Generate the pattern for the game*/
		generatePattern(pattern, PATTERN_LENGTH);

		/*Play a round with the user*/
		result = playRound(pattern, PATTERN_LENGTH);

		/*Check to see if the user has quit*/
		if (continueSuperLoop() == FALSE) {
			return;
		}

		/*Display the flashing lights corresponding to if the user won or lost*/
		winLose(result);

		/*Wait a specified time before starting a new game, check to see if user has quit every second*/
		for (count = 0; count < NEW_GAME_TIME; count++) {

			/*Check to see if the user has quit*/
			if (continueSuperLoop() == FALSE) {
				return;
			}
			
			Sleep(ONE_SECOND);
		}
	}

}




/*Plays one round of simon with the user, returns TRUE if they won,
and FALSE if they lost*/
int playRound(int pattern[], int pattern_length) {
	int count;
	int result = FALSE;

	for (count = 1; count <= pattern_length; count++) {

		/*Play one cycle of the game*/
		result = playCycle(pattern, count, pattern_length);

		/*Check to see if the user has quit*/
		if (continueSuperLoop() == FALSE) {
			break;
		}

		/*Check if the user got the pattern right*/
		if (result == FALSE) {
			break;
		}

		/*Wait before starting the next cycle*/
		Sleep(NEXT_CYCLE * ONE_SECOND);
	}

	return result;
}




/*Plays one cycle of the game, including flashing the pattern to the user,
reading the user's input, and checking if it is correct. Returns TRUE if 
the user is correct, FALSE otherwise*/
int playCycle(int pattern[], int cycle_Num, int pattern_length) {
	int result = FALSE;

	/*Play the pattern of lights*/
	playPattern(pattern, cycle_Num);

	/*Check to see if the user has quit*/
	if (continueSuperLoop() == FALSE) {
		return result;
	}

	/*Reads the pattern the user inputs and checks to see if it is correct*/
	result = readPattern(pattern,cycle_Num);

	return result;
}


/*Flashes LEDs corresponding to the pattern up to the specified length*/
void playPattern(int pattern[], int cycle_length) {
	int index;

	for (index = 0; index < cycle_length; index++) {
		blinkNumTimes(pattern[index], 1, BLINK_SPEED);

		/*Check to see if the user has quit*/
		if (continueSuperLoop() == FALSE) {
			break;
		}
	}
}



/*Reads the pattern the user inputs, and checks each one to see if it is correct*/
int readPattern(int pattern[], int cycle_Num) {
	int index, nextNum;
	int result = TRUE;

	for (index = 0; index < cycle_Num; index++) {
		
		/*Reads the next button press from the user*/
		nextNum = waitForButtonPress();
		waitForButtonRelease();

		/*Check to see if the user has quit*/
		if (continueSuperLoop() == FALSE) {
			break;
		}

		result = checkNextNum(pattern, nextNum, index);

		if (result == FALSE) {
			break;
		}
	}

	return result;
}


/*Checks the button the user inputs with the actual pattern*/
int checkNextNum(int pattern[], int nextNum, int index) {
	int result = (nextNum == pattern[index]);
	return result;
}


/*Waits for the user to press a button, returns which button they pressed*/
int waitForButtonPress(void) {
	
	/*Initialized with 0 in case the user quits, which would mean the function
	would return before reading a button press*/
	int button_num = 0;

	while (TRUE) {

		/*Check each of the buttons*/
		if (digitalRead(GREEN) == ON) {
			button_num = GREEN;
			break;
		}
		else if (digitalRead(RED) == ON) {
			button_num = RED;
			break;
		}
		else if (digitalRead(BLUE) == ON) {
			button_num = BLUE;
			break;
		}
		else if (digitalRead(YELLOW) == ON) {
			button_num = YELLOW;
			break;
		}

		/*Check to see if the user has quit*/
		if (continueSuperLoop() == FALSE) {
			break;
		}

	}

	return button_num;
}

/*Waits until all the buttons are released*/
void waitForButtonRelease(void) {
	while (digitalRead(GREEN) + digitalRead(RED) + digitalRead(YELLOW) + digitalRead(BLUE) != OFF) {
		/*Does nothing until all buttons are released*/
	}
}


/*Function that will flash either 3 green lights if the user won
or 3 red lights if the user lost*/
void winLose(int result) {
	if (result == TRUE) {
		blinkNumTimes(GREEN, 3, NEW_GAME_BLINK_SPEED);
	}
	else {
		blinkNumTimes(RED, 3, NEW_GAME_BLINK_SPEED);
	}

}


/*Blink a specificed light a specified number of times*/
void blinkNumTimes(int light, int numTimes, double time) {
	int count;

	for (count = 0; count < numTimes; count++) {
		Sleep(time*ONE_SECOND);
		digitalWrite(light, ON);

		Sleep(time*ONE_SECOND);
		digitalWrite(light, OFF);


		/*Check to see if the user has quit*/
		if (continueSuperLoop() == FALSE) {
			break;
		}

	}

}



/*Generates a sequence of random numbers which correspond to the pattern*/
void generatePattern(int pattern[], int pattern_length) {
	int index;

	for (index = 0; index < pattern_length; index++) {
		pattern[index] = getRandom(0, NUM_LEDS - 1);
	}
}



/*Generates a random number between the lower value and the upper value*/
int getRandom(int lower, int upper) {
	int nextNum;

	/*The number of possible random numbers the function should generate*/
	int range = upper - lower + 1;

	nextNum = rand() % range;
	nextNum += lower;

	return nextNum;
}
