#include "rooms.h"

extern Room titleScreenRoom;
extern Room chamber0;
extern Room chamber1;
extern Room chamber2;
extern Room chamber3;
extern Room chamber4;
extern Room chamber5;
extern Room chamber6;
extern Room chamber7;
extern Room chamber8;
extern Room chamber9;
extern Room transitionRoom;
extern Room wipeTransitionRoom;
extern Room getReadyRoom;

const Room* const g_rooms[] = 
{
	&chamber0,
	&chamber1,
	&chamber2,
	&chamber3,
	&chamber4,
	&chamber5,
	&chamber6,
	&chamber7,
	&chamber8,
	&chamber9,
	&titleScreenRoom,
	&transitionRoom,
	&wipeTransitionRoom,
	&getReadyRoom,
};