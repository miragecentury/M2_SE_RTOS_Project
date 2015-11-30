#ifndef __MYPWM_H__
#define __MYPWM_H__

struct myQueueMsgOrderMyPWM {
	int indexPin;
	char* order;
	void* args;
};

void vInitMyPWM(void);

#endif
