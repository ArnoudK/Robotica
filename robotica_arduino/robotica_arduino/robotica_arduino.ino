/*
 Name:    arduino_test_2.ino
 Created: 3/2/2022 1:55:53 PM
 Author:  Du
*/




/*  -----------------------------
  STATES
-----------------------------*/

enum stepAndState : byte {
	UNKOWN = 0,
	LEFT = 1,
	FORWARD = 2,
	RIGHT = 3,
	TURNAROUND = 4,
	CHECKJUNCTION = 5,
	BACKWARD = 6,
	FINISHED = 7
};


byte path[20] = { 1,1,2,2,3,3,0 };


/*----LETTERS FOR SGDP--- */
const byte digitData[15][7]{
  {0,1,1,1,1,1,1},
  {0,0,0,0,1,1,0},
  {1,0,1,1,0,1,1},
  {1,0,0,1,1,1,1},
  {1,1,0,0,1,1,0},
  {1,1,0,1,1,0,1},
  {1,1,1,1,1,0,1},
  {0,0,0,0,1,1,1},
  {1,1,1,1,1,1,1},
  {1,1,0,1,1,1,1},
  // LETTER L
  {0,1,1,1,0,0,0},
  // LETTER R
  {1,1,1,0,1,1,1},
  // LETTER F
  {1,1,1,0,0,0,1},
  // LETTER I
  {0,1,1,0,0,0,0},
  //NIKS
  {0,0,0,0,0,0,0}
};



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
  //PWM moter left E2
constexpr auto MOTER_LEFT_PWM = 6;
// moter direction (on forward) M2
constexpr auto MOTER_LEFT_DIRECTION = 7;
// change direction 

//SPEED
unsigned  moter_left_speed = 0;



/*  -----------------------------
*   MOTER RIGHT
  -----------------------------*/
  // E1
constexpr auto MOTER_RIGHT_PWM = 5;
//m1
constexpr auto MOTER_RIGHT_DIRECTION = 4;;
//speed
unsigned moter_right_speed = 0;
bool leftForward = true;
bool rightForward = true;


/*-----------------------------------
* ULTRA SOUND SENSOR
  ---------------------------------*/
constexpr auto ECHO_PIN = 2;
constexpr auto TRIG_PIN = 3;

long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement


/*------------------------
  DISPLAYS
--------------------------*/
//G = 1 en 7 = A
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

/*set 0 to true for left 1 straight 2 forward if possible*/
bool juncPoss[3] = {};


/*-----------------
  MAGIC VALUES
-------------------*/
constexpr auto DELAYBACKWARDS = 350;
constexpr auto DELAYFORWARDS = 250;
constexpr int DEFSPEED = 240;
constexpr int FASTERSPEED = 255;
constexpr int LOWSPEED = 200;
constexpr auto MINTURNTIME = 300;

// the setup function runs once when you press reset or power the board
void setup() {

	pinMode(MOTER_LEFT_PWM, OUTPUT);
	pinMode(MOTER_LEFT_DIRECTION, OUTPUT);
	pinMode(MOTER_RIGHT_PWM, OUTPUT);
	pinMode(MOTER_RIGHT_DIRECTION, OUTPUT);
	pinMode(ECHO_PIN, INPUT); // Sets the trigPin as an OUTPUT
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

	timeSinceLastStateChange = millis();
	state = FORWARD; delay(1000);
}

// the loop function runs over and over again until power down or reset
void loop() {
	leftForward = true;
	rightForward = true;
	/*-------------UPDATE SENSORS------------*/
	  //updateUltraSoundDistance();
	//---UPDATING THE SENSOR DELAYS FOR ~15 ms
	digitalWrite(TRIG_PIN, LOW);
	delayMicroseconds(2);
	//
	digitalWrite(TRIG_PIN, HIGH);
	delayMicroseconds(10);
	digitalWrite(TRIG_PIN, LOW);
	//
	duration = pulseIn(ECHO_PIN, HIGH);
	//distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
	distance = duration * 0.017;

	//update road dection
	outerright = !digitalRead(IR_OUTER_RIGHT);
	innerright = !digitalRead(IR_INNER_RIGHT);
	middle = !digitalRead(IR_MIDDLE);
	innerleft = !digitalRead(IR_INNER_LEFT);
	outerleft = !digitalRead(IR_OUTER_LEFT);



	/*---------------
	  DO LOGICA
	  -------------*/



	switch (state)
	{
	case FORWARD:
		moter_left_speed = DEFSPEED;
		moter_right_speed = DEFSPEED;
		juncPoss[0] = 0;
		juncPoss[1] = 0;
		juncPoss[2] = 0;

		if (outerright || outerleft) {
			timeSinceLastStateChange = millis();
			juncPoss[0] = outerleft;
			juncPoss[2] = outerright;
			state = CHECKJUNCTION;

		}
		else if (!(middle || innerleft || innerright)) {
			timeSinceLastStateChange = millis();
			state = TURNAROUND;
		}
		else {
			//Try to straighten;
			if (innerleft) {
				moter_left_speed = LOWSPEED;
			}
			else if (innerright) {
				moter_right_speed = LOWSPEED;
			}
		}
		break;
	case LEFT:
		if ((!outerright && !outerleft && middle) || millis() - timeSinceLastStateChange < MINTURNTIME) {
			moter_right_speed = DEFSPEED;
			moter_left_speed = DEFSPEED;
			leftForward = false;
		}
		else {
			state = FORWARD;
			encouter_count++;
		}
		break;
	case RIGHT:
		if ((!outerright && !outerleft && middle) || millis() - timeSinceLastStateChange < MINTURNTIME) {
			moter_left_speed = DEFSPEED;
			moter_right_speed = DEFSPEED;
			rightForward = false;
		}
		else {
			state = FORWARD;
			encouter_count++;
		}
		break;
	case CHECKJUNCTION:
		// first 100 mill check if we can go left and or right and keep driving forward
		if (millis() - timeSinceLastStateChange < DELAYFORWARDS) {
			moter_left_speed = DEFSPEED;
			moter_right_speed = DEFSPEED;
			/*Check if we can go left and or right*/
			juncPoss[0] = juncPoss[0] || outerleft;
			juncPoss[2] = juncPoss[2] || outerright;
		}
		// after 100 millis check if we can go straight and go backwards
		else {
			juncPoss[1] = middle;

			state = BACKWARD;
		}
		break;
	case BACKWARD:
		if (millis() - timeSinceLastStateChange < (DELAYFORWARDS + DELAYBACKWARDS)
			&& outerleft == juncPoss[0] && outerright == juncPoss[2] && middle) {
			leftForward = false;
			rightForward = false;
			moter_left_speed = FASTERSPEED;
			moter_right_speed = FASTERSPEED;
		}
		else {
			if (juncPoss[1] && juncPoss[0] && juncPoss[2]
				&& innerleft && innerright && outerright && outerleft && middle) {
				finish();
				return;
			}
			else if (juncPoss[0]) {
				timeSinceLastStateChange = millis();
				state = LEFT;
			}
			else if (juncPoss[1]) {
				state = FORWARD;
			}
			else {
				state = RIGHT;
				timeSinceLastStateChange = millis();
			}
		}
		break;
	case FINISHED:

		return;
		//@TODO LED stuff
	case TURNAROUND:
		moter_right_speed = DEFSPEED;
		moter_left_speed = DEFSPEED;
		rightForward = false;
		if (millis() - timeSinceLastStateChange > 300) {
			if (middle) {
				state = FORWARD;
			}
		}
		break;
	default:
		break;
	}

	/*----------UPDATE MOTER----------*/



	if (distance < 10) {
		moter_left_speed = 0;
		moter_right_speed = 0;
	}
	//set moter power;
	digitalWrite(MOTER_LEFT_DIRECTION, !leftForward);
	digitalWrite(MOTER_RIGHT_DIRECTION, rightForward);
	analogWrite(MOTER_LEFT_PWM, moter_left_speed);   //PWM Speed Control
	analogWrite(MOTER_RIGHT_PWM, moter_right_speed);   //PWM Speed Control



	//for (int i = 0; i < 14; i++) {
	//  long timer = millis();

	//  while (millis() - timer < 2000) {
	//    display(0, 1, i);
	//    delay(7);
	//    display(1, 0, 13 - i);
	//    delay(7);
	//  }
	//}


}

void finish() {
	analogWrite(MOTER_LEFT_PWM, 0);   //PWM Speed Control
	analogWrite(MOTER_RIGHT_PWM, 0);   //PWM Speed Control
	showPath();
	state = FINISHED;
}



void display(int toDisplay, int displayOff, int num) {
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

void showPath() {
	//count down from 10
	for (int i = 9; i >= 0; i--) {
		display(0, 1, i);
		delay(1000);
	}
	//show path;
	int count = 0;
	while (path[count] != 0) {
		byte toDisplay;
		switch (path[count]) {
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
		long   timer = millis();
		while (millis() - timer < 1000) {
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