/*
 Name:    arduino_test_2.ino
 Created: 3/2/2022 1:55:53 PM
 Author:  Du
*/




/*  -----------------------------
		STATES
-----------------------------*/

enum stepAndState : byte {
	UNKOWN,
	LEFT,
	RIGHT,
	END,
	FORWARD,
	TURNAROUND,
	CHECKJUNCTION,
	BACKWARD
};

/*----LETTERS FOR SGDP--- */
byte digitData[14][7]{
	{0,1,1,1,1,1,1},
	{0,0,0,0,1,1,0},
	{1,0,1,1,0,1,1},
	{1,0,0,1,1,1,1},
	{1,1,0,0,1,1,0},
	{1,1,0,1,1,0,1},
	{1,1,1,1,1,0,1},
	{0,0,0,0,1,1,1},
	{1,1,1,1,1,1,1},
	{1,0,0,1,1,1,1},
	// LETTER L
	{0,1,1,1,0,0,0},
	// LETTER R
	{1,1,1,0,1,1,1},
	// LETTER F
	{1,1,1,0,0,0,1},
	// LETTER I
	{0,1,1,0,0,0,0}


};



/*  -----------------------------
	INFRARED SENSOR
-----------------------------*/

const auto IR_OUTER_LEFT = A0;
const auto IR_INNER_LEFT = A1;
const auto IR_MIDDLE = A2;
const auto IR_INNER_RIGHT = A3;
const auto IR_OUTER_RIGHT = A4;

bool outerleft, innerleft, middle, innerright, outerright;


/*   ----------------
*   MOTER LEFT
  _________________*/
  //PWM moter left E2
const auto MOTER_LEFT_PWM = 6;
// moter direction (on forward) M2
const auto MOTER_LEFT_DIRECTION = 7;
// change direction 

//SPEED
int moter_left_speed;



/*  -----------------------------
*   MOTER RIGHT
  -----------------------------*/
  // E1
const auto MOTER_RIGHT_PWM = 5;
//m1
const auto MOTER_RIGHT_DIRECTION = 4;
//speed
int moter_right_speed;


/*-----------------------------------
*	ULTRA SOUND SENSOR
	---------------------------------*/
const int ECHO_PIN = 2;
const int TRIG_PIN = 3;

long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement




/*----------------------
		OTHER STUFF
------------------------*/

// number of encounters passed
int encouter = 0;
long lastEncounter;
byte state;

int juncPoss[3];



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

	Serial.begin(9600);
	lastEncounter = millis();
	state = FORWARD;
}

// the loop function runs over and over again until power down or reset
void loop() {

	/*-------------UPDATE SENSORS------------*/
		//updateUltraSoundDistance();

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
	outerright = digitalRead(IR_OUTER_RIGHT);
	innerright = digitalRead(IR_INNER_RIGHT);
	middle = digitalRead(IR_MIDDLE);
	innerleft = digitalRead(IR_INNER_LEFT);
	outerleft = digitalRead(IR_OUTER_RIGHT);



	/*---------------
		DO LOGICA
		-------------*/
	switch (state)
	{
	case FORWARD:
		moter_left_speed = 60;
		moter_right_speed = 60;
		if (!outerright || !outerleft) {
			lastEncounter = millis();
			state = CHECKJUNCTION;
		}
		break;
	case LEFT:
		if (outerright || outerleft) {
			moter_right_speed = 60;
			moter_left_speed = 0;
		}
		else {
			state = FORWARD;
			encouter++;
		}
		break;
	case RIGHT:
		if (outerright || outerleft) {
			moter_left_speed = 60;
			moter_right_speed = 0;
		}
		else {
			state = FORWARD;
			encouter++;
		}
		break;
	case CHECKJUNCTION:
		if (millis() - lastEncounter < 100) {

		}

		break;
	default:
		break;
	}




	/*----------UPDATE MOTER----------*/







	moter_left_speed = 60;
	moter_right_speed = 60;


	if (distance < 20) {
		moter_left_speed = 0;
		moter_right_speed = 0;
	}
	digitalWrite(MOTER_LEFT_DIRECTION, LOW);
	digitalWrite(MOTER_RIGHT_DIRECTION, HIGH);
	analogWrite(MOTER_LEFT_PWM, moter_left_speed);   //PWM Speed Control
	analogWrite(MOTER_RIGHT_PWM, moter_right_speed);   //PWM Speed Control

	//Serial.print("Afstand: ");
	//Serial.println(distance);

	//for (int i = A0; i <= A4; i++) {
	//	Serial.print(i);
	//	Serial.print(": ");
	//	Serial.println(digitalRead(i));
	//}


}


