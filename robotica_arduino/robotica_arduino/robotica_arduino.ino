/*
 Name:    arduino_test_2.ino
 Created: 3/2/2022 1:55:53 PM
 Author:  Du
*/




/*  -----------------------------
		STATES
-----------------------------*/

#define ENABLE_SERIAL

enum stepAndState : byte {
	UNKOWN,
	LEFT,
	RIGHT,
	FORWARD,
	TURNAROUND,
	CHECKJUNCTION,
	BACKWARD,
	FINISHED
};

/*----LETTERS FOR SGDP--- */
const byte digitData[14][7]{
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
bool goingForward = true;


/*-----------------------------------
*	ULTRA SOUND SENSOR
	---------------------------------*/
constexpr int ECHO_PIN = 2;
constexpr int TRIG_PIN = 3;

long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement




/*----------------------
		OTHER STUFF
------------------------*/

// number of encounters passed
int encouter_count = 0;
long lastEncounter;
byte state;

/*set 0 to true for left 1 straight 2 forward if possible*/
bool juncPoss[3] = {};



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

#ifdef ENABLE_SERIAL
	Serial.begin(9600);
#endif // ENABLE_SERIAL



	lastEncounter = millis();
	state = FORWARD;
}

// the loop function runs over and over again until power down or reset
void loop() {

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
	outerright = digitalRead(IR_OUTER_RIGHT);
	innerright = digitalRead(IR_INNER_RIGHT);
	middle = digitalRead(IR_MIDDLE);
	innerleft = digitalRead(IR_INNER_LEFT);
	outerleft = digitalRead(IR_OUTER_RIGHT);



	/*---------------
		DO LOGICA
		-------------*/

	goingForward = true;


	switch (state)
	{
	case FORWARD:
		moter_left_speed = 60;
		moter_right_speed = 60;
		if (!outerright || !outerleft) {
			lastEncounter = millis();
			state = CHECKJUNCTION;
		}
		else if (!(middle && innerleft && innerright)) {
			lastEncounter = millis();
			state = TURNAROUND;
		}
		else {
			//Try to straighten;
			if (!innerleft) {
				moter_right_speed = 40;
			}
			else if (!innerleft) {
				moter_right_speed = 40;
			}
		}
		break;
	case LEFT:
		if (outerright || outerleft) {
			moter_right_speed = 60;
			moter_left_speed = 0;
		}
		else {
			state = FORWARD;
			encouter_count++;
		}
		break;
	case RIGHT:
		if (outerright || outerleft) {
			moter_left_speed = 60;
			moter_right_speed = 0;
		}
		else {
			state = FORWARD;
			encouter_count++;
		}
		break;
	case CHECKJUNCTION:
		// first 100 mill check if we can go left and or right and keep driving forward
		if (millis() - lastEncounter < 100) {
			moter_left_speed = 60;
			moter_right_speed = 60;
			/*Check if we can go left and or right*/
			juncPoss[0] = juncPoss[0] || !outerleft;
			juncPoss[2] = juncPoss[2] || !outerright;
		}
		// after 100 millis check if we can go straight and go backwards
		else {
			juncPoss[1] = middle;
			state = BACKWARD;
		}
		break;
	case BACKWARD:
		if (millis() - lastEncounter < 120) {
			goingForward = false;
			moter_left_speed = 60;
			moter_right_speed = 60;
		}
		else {
			if (juncPoss[0] && juncPoss[1] && juncPoss[2]) {
				state = FINISHED;
			}
			if (juncPoss[0]) {
				state = LEFT;
			}
			else if (juncPoss[1]) {
				state = FORWARD;
			}
			else {
				state = RIGHT;
			}
		}
		break;
	case FINISHED:
		moter_left_speed = 0;
		moter_right_speed = 0;
		return;
		//@TODO LED stuff
		break;
	case TURNAROUND:
		moter_right_speed = 60;
		moter_left_speed = 0;
		if (millis() - lastEncounter > 100) {
			if (middle) {
				encouter_count++;
				state = FORWARD;
			}
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
	//set moter power;
	digitalWrite(MOTER_LEFT_DIRECTION, !goingForward);
	digitalWrite(MOTER_RIGHT_DIRECTION, goingForward);
	analogWrite(MOTER_LEFT_PWM, moter_left_speed);   //PWM Speed Control
	analogWrite(MOTER_RIGHT_PWM, moter_right_speed);   //PWM Speed Control

#ifdef ENABLE_SERIAL
	Serial.print("Afstand: ");
	Serial.println(distance);

	for (int i = A0; i <= A4; i++) {
		Serial.print(i);
		Serial.print(": ");
		Serial.println(digitalRead(i));
	}
#endif // ENABLE_SERIAL

	


}


