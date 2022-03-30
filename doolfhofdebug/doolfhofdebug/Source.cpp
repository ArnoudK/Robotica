#include <stdio.h>

#define byte unsigned

enum stepAndState : unsigned
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

unsigned path[50] = { LEFT,LEFT,TURNAROUND,LEFT,TURNAROUND,FORWARD,LEFT,LEFT };

void printPath() {
	for (int count = 0; count < 50 && path[count] != UNKOWN; count++) {
		printf("%d",((int)path[count]));
	}
	printf("\n");
}

void optimizeRoute() {
	for (int i = 1; i < 50 && path[i] != UNKOWN; i++) {
		if (path[i] == TURNAROUND) {
			byte before = path[i - 1];
			byte after = path[i + 1];
			byte result;
			switch (before) {
			case LEFT:
				if (after == LEFT) {
					result = FORWARD;
				}
				else if (after == FORWARD) {
					result = RIGHT;
				}
				else if (after == RIGHT) {
					result = TURNAROUND;
				}
				break;
			case FORWARD:
				if (after == LEFT) {
					result = RIGHT;
				}
				else if (after == FORWARD) {
					result = TURNAROUND;
				}
				break;
			case RIGHT:
				result = TURNAROUND;
				break;
			default:
				break;
			}
			path[i - 1] = result;
			for (int j = i; j < 47; j++) {
				path[j] = path[j + 3];
			}
			i = 1;
		}
	}
}


int main() {
	optimizeRoute();
	printPath();
}

