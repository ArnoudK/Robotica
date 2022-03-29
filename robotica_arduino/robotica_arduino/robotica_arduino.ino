/*
 Name:    arduino_test_2.ino
 Created: 3/2/2022 1:55:53 PM
 Author:  Du
*/

#include "Ticker.h"
#include <avr/sleep.h>

void objectDetectie();
Ticker ultrasoonCheckup(objectDetectie, 100);
void lijnDetectie();
Ticker updateRoadDetection(lijnDetectie, 100);
void setMotorPower();
Ticker motorPowerUpdate(setMotorPower, 100);
void displayLEDS();
Ticker updateDisplayLEDS(displayLEDS, 7);
// void setRobotStop();
// Ticker robotStopper(setRobotStop, 0);

/*  -----------------------------
  STATES
-----------------------------*/

enum stepAndState : byte
{
	UNKOWN = 0,
	LEFT = 1,
	FORWARD = 2,
	RIGHT = 3,
	TURNAROUND = 4,
	CHECKJUNCTION = 5,
	BACKWARD = 6,
	FINISHED = 7
};

byte path[36] = {};

/*----LETTERS FOR SGDP--- */
const byte digitData[15][7]{
	{0, 1, 1, 1, 1, 1, 1},
	{0, 0, 0, 0, 1, 1, 0},
	{1, 0, 1, 1, 0, 1, 1},
	{1, 0, 0, 1, 1, 1, 1},
	{1, 1, 0, 0, 1, 1, 0},
	{1, 1, 0, 1, 1, 0, 1},
	{1, 1, 1, 1, 1, 0, 1},
	{0, 0, 0, 0, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1},
	{1, 1, 0, 1, 1, 1, 1},
	// LETTER L
	{0, 1, 1, 1, 0, 0, 0},
	// LETTER R
	{1, 1, 1, 0, 1, 1, 1},
	// LETTER F
	{1, 1, 1, 0, 0, 0, 1},
	// LETTER I
	{0, 1, 1, 0, 0, 0, 0},
	// NIKS
	{0, 0, 0, 0, 0, 0, 0}};

/*  -----------------------------
  INFRARED SENSOR
-----------------------------*/

constexpr auto IR_OUTER_LEFT = A0;
constexpr auto IR_INNER_LEFT = A1;
constexpr auto IR_MIDDLE = A2;
constexpr auto IR_INNER_RIGHT = A3;
constexpr auto IR_OUTER_RIGHT = A4;

bool outerleft, innerleft, middle, innerright, outerright;

/*   ----------------
*   MOTER LEFT
  _________________*/
// PWM moter left E2
constexpr auto MOTER_LEFT_PWM = 6;
// moter direction (on forward) M2
constexpr auto MOTER_LEFT_DIRECTION = 7;
// change direction
bool leftForward = true;
// SPEED
unsigned moter_left_speed = 0;

/*  -----------------------------
*   MOTER RIGHT
  -----------------------------*/
// E1
constexpr auto MOTER_RIGHT_PWM = 5;
// m1
constexpr auto MOTER_RIGHT_DIRECTION = 4;
;
// speed
unsigned moter_right_speed = 0;
bool rightForward = true;

/*-----------------------------------
* ULTRA SOUND SENSOR
  ---------------------------------*/
constexpr auto ECHO_PIN = 2;
constexpr auto TRIG_PIN = 3;
long duration; // variable for the duration of sound wave travel
int distance;  // variable for the distance measurement

/*------------------------
  DISPLAYS
--------------------------*/
// G = 1 en 7 = A
constexpr auto LED1 = 11;
constexpr auto LED2 = 12;
constexpr auto LED3 = 8;
constexpr auto LED4 = 9;
constexpr auto LED5 = 10;
constexpr auto LED6 = A5;
constexpr auto LED7 = 13;
constexpr auto LEDMULTIPLEX = 0;
constexpr auto LEDMULTIPLEX2 = 1;

/*----------------------
  OTHER STUFF
------------------------*/

// number of encounters passed
int encouter_count = 0;
long timeSinceLastStateChange;
byte state;
byte ledTimer = 0;

/*set 0 to true for left 1 straight 2 right if possible*/
bool juncPoss[3] = {};

/*-----------------
  MAGIC VALUES
-------------------*/
constexpr auto DELAYBACKWARDS = /*350*/ -300;
constexpr auto DELAYFORWARDS = /*250*/ 400;
constexpr int DEFSPEED = 240;	 // 66; //240
constexpr int FASTERSPEED = 255; // 72; //255
constexpr int LOWSPEED = 100;	 // 100
constexpr auto MINTURNTIME = 630;

// the setup function runs once when you press reset or power the board
void setup()
{
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	ultrasoonCheckup.start();
	updateRoadDetection.start();
	motorPowerUpdate.start();
	updateDisplayLEDS.start();
	// robotStopper.start();

	pinMode(MOTER_LEFT_PWM, OUTPUT);
	pinMode(MOTER_LEFT_DIRECTION, OUTPUT);
	pinMode(MOTER_RIGHT_PWM, OUTPUT);
	pinMode(MOTER_RIGHT_DIRECTION, OUTPUT);
	pinMode(ECHO_PIN, INPUT);  // Sets the trigPin as an OUTPUT
	pinMode(TRIG_PIN, OUTPUT); // Sets the echoPin as an INPUT

	pinMode(IR_OUTER_LEFT, INPUT);
	pinMode(IR_INNER_LEFT, INPUT);
	pinMode(IR_MIDDLE, INPUT);
	pinMode(IR_INNER_RIGHT, INPUT);
	pinMode(IR_OUTER_RIGHT, INPUT);

	pinMode(LED1, OUTPUT);
	pinMode(LED2, OUTPUT);
	pinMode(LED3, OUTPUT);
	pinMode(LED4, OUTPUT);
	pinMode(LED5, OUTPUT);
	pinMode(LED6, OUTPUT);
	pinMode(LED7, OUTPUT);
	pinMode(LEDMULTIPLEX, OUTPUT);
	pinMode(LEDMULTIPLEX2, OUTPUT);

	state = FORWARD;
	delay(302);
}

/*-------------UPDATE SENSORS------------*/
void objectDetectie()
{
	// updateUltraSoundDistance();
	//---UPDATING THE SENSOR DELAYS FOR ~15 ms
	digitalWrite(TRIG_PIN, LOW);
	delayMicroseconds(2);
	updateDisplayLEDS.update();
	digitalWrite(TRIG_PIN, HIGH);
	delayMicroseconds(10);
	updateDisplayLEDS.update();
	digitalWrite(TRIG_PIN, LOW);
	//
	duration = pulseIn(ECHO_PIN, HIGH);
	// distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
	distance = duration * 0.017;

}

// update road dection
void lijnDetectie()
{
	outerright = !digitalRead(IR_OUTER_RIGHT);
	innerright = !digitalRead(IR_INNER_RIGHT);
	middle = !digitalRead(IR_MIDDLE);
	innerleft = !digitalRead(IR_INNER_LEFT);
	outerleft = !digitalRead(IR_OUTER_LEFT);
}

/*----------UPDATE MOTER----------*/
void setMotorPower()
{
	// set moter power;
	digitalWrite(MOTER_LEFT_DIRECTION, !leftForward);
	digitalWrite(MOTER_RIGHT_DIRECTION, rightForward);
	analogWrite(MOTER_LEFT_PWM, moter_left_speed);			 // PWM Speed Control
	analogWrite(MOTER_RIGHT_PWM, moter_right_speed * 0.97f); // PWM Speed Control
}

void displayLEDS()
{
	ledTimer++;
	if (ledTimer % 2)
	{
		display(0, 1, encouter_count % 10);
	}
	else
	{
		int toDisplay = 0;
		switch (path[encouter_count - 1])
		{
		case LEFT:
			toDisplay = 10;
			break;
		case RIGHT:
			toDisplay = 11;
			break;
		case FORWARD:
			toDisplay = 12;
			break;

		default:
			toDisplay = 0;
			break;
		}
		display(1, 0, toDisplay);
	}
}

void loop()
{
	updateDisplayLEDS.update();

	ultrasoonCheckup.update();
	updateRoadDetection.update();
	updateDisplayLEDS.update();
	leftForward = true;
	rightForward = true;

	/*---------------
	  DO LOGICA
	  -------------*/

	switch (state)
	{
	case FORWARD:
		moter_left_speed = FASTERSPEED;
		moter_right_speed = FASTERSPEED;
		juncPoss[0] = 0;
		juncPoss[1] = 0;
		juncPoss[2] = 0;

		if (((outerright && innerright) || (outerleft && innerleft)) && millis() - timeSinceLastStateChange < 100)
		{
			timeSinceLastStateChange = millis();
			juncPoss[0] = outerleft;
			juncPoss[2] = outerright;
			state = CHECKJUNCTION;
		}
		else if (!(middle || innerleft || innerright) || distance < 10 /*&& millis() > timeSinceLastStateChange + 500*/)
		{
			timeSinceLastStateChange = millis();
			state = TURNAROUND;
		}
		else
		{
			// Try to straighten;
			if (innerleft)
			{
				moter_left_speed = LOWSPEED;
				timeSinceLastStateChange = millis();
			}
			else if (innerright)
			{
				moter_right_speed = LOWSPEED;
				timeSinceLastStateChange = millis();
			}
		}
		break;
	case LEFT:
		if ((!outerright && !outerleft && middle) || millis() - timeSinceLastStateChange < MINTURNTIME)
		{
			moter_right_speed = FASTERSPEED;
			moter_left_speed = FASTERSPEED;
			leftForward = false;
		}
		else
		{
			state = FORWARD;
		}
		break;
	case RIGHT:
		if ((!outerright && !outerleft && middle) || millis() - timeSinceLastStateChange < MINTURNTIME)
		{
			moter_left_speed = FASTERSPEED;
			moter_right_speed = FASTERSPEED;
			rightForward = false;
		}
		else
		{
			state = FORWARD;
		}
		break;
	case CHECKJUNCTION:
		// first 100 mill check if we can go left and or right and keep driving forward
		if (millis() - timeSinceLastStateChange < DELAYFORWARDS)
		{
			moter_left_speed = DEFSPEED;
			moter_right_speed = DEFSPEED;
			/*Check if we can go left and or right*/
			juncPoss[0] = juncPoss[0] || outerleft;
			juncPoss[2] = juncPoss[2] || outerright;
		}
		// after 100 millis check if we can go straight and go backwards
		else
		{
			juncPoss[1] = middle;
			timeSinceLastStateChange = millis();
			state = BACKWARD;
		}
		break;
	case BACKWARD:
		if (millis() - timeSinceLastStateChange < (DELAYFORWARDS + DELAYBACKWARDS)  && outerleft == juncPoss[0] && outerright == juncPoss[2] && middle)
		{
			leftForward = false;
			rightForward = false;
			moter_left_speed = FASTERSPEED;
			moter_right_speed = FASTERSPEED;
			break;
		}
		else
		{
			// check finish
			if (juncPoss[1] && juncPoss[0] && juncPoss[2] && innerleft && innerright && outerright && outerleft && middle)
			{
				if (millis() > timeSinceLastStateChange + 200)
				{
					finish();
					return;
				}
			}
			else if (juncPoss[0])
			{
				state = LEFT;
				save_route(LEFT);
				break;
			}
			else if (juncPoss[1])
			{
				state = FORWARD;
				save_route(FORWARD);
				break;
			}
			else
			{
				state = RIGHT;
				save_route(RIGHT);
				break;
			}
		}
		break;
	case FINISHED:
		sleep_mode();
		return;
		//@TODO LED stuff
	case TURNAROUND:
		moter_right_speed = FASTERSPEED;
		moter_left_speed = FASTERSPEED;
		rightForward = false;
		if (middle && millis() - timeSinceLastStateChange > MINTURNTIME)
		{
			state = FORWARD;
		}
		break;
	default:
		state = FORWARD;
		break;
	}
	updateDisplayLEDS.update();

	motorPowerUpdate.update();
	updateDisplayLEDS.update();

	// the loop function runs over and over again until power down or reset
}

void save_route(byte direction)
{
	timeSinceLastStateChange = millis();
	path[encouter_count] = direction;
	encouter_count++;
}

void finish()
{
	analogWrite(MOTER_LEFT_PWM, 0);	 // PWM Speed Control
	analogWrite(MOTER_RIGHT_PWM, 0); // PWM Speed Control
	updateDisplayLEDS.stop();
	showPath();
	state = FINISHED;
}

void display(int toDisplay, int displayOff, int num)
{
	digitalWrite(toDisplay, 0);
	digitalWrite(displayOff, 1);
	digitalWrite(LED1, digitData[num][0]);
	digitalWrite(LED2, digitData[num][1]);
	digitalWrite(LED3, digitData[num][2]);
	digitalWrite(LED4, digitData[num][3]);
	digitalWrite(LED5, digitData[num][4]);
	digitalWrite(LED6, digitData[num][5]);
	digitalWrite(LED7, digitData[num][6]);
}

void showPath()
{
	// count down from 10
	for (int i = 9; i >= 0; i--)
	{
		display(0, 1, i);
		delay(1000);
	}
	// show path;
	int count = 0;
	while (path[count] != 0)
	{
		byte toDisplay;
		switch (path[count])
		{
		case LEFT:
			toDisplay = 10;
			break;
		case RIGHT:
			toDisplay = 11;
			break;
		case FORWARD:
			toDisplay = 12;
			break;

		default:
			toDisplay = 0;
			break;
		}

		// display(0, 1, toDisplay);
		// delay(1000);
		// display(0, 1, 14);
		long timer = millis();
		while (millis() - timer < 1000)
		{
			display(0, 1, toDisplay);
			delay(7);
			display(1, 0, (count + 1) % 10);
			delay(7);
		}
		display(1, 0, 14);
		delay(500);
		count++;
	}
}
