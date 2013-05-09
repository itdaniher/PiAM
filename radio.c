// Fractional-N synthesizer setup
// This code is based off of the PiFm and Pihat projects
// by Oliver Mattos, Oskar Weigl, and Jon Petter Skagmo
// Application-specific modifications made by Ian Daniher

#include "radio.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <math.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <sched.h>

# define timerisset(tvp)        ((tvp)->tv_sec || (tvp)->tv_usec)
# define timerclear(tvp)        ((tvp)->tv_sec = (tvp)->tv_usec = 0)
# define timercmp(a, b, CMP)                                                  \
  (((a)->tv_sec == (b)->tv_sec) ?                                             \
   ((a)->tv_usec CMP (b)->tv_usec) :                                          \
   ((a)->tv_sec CMP (b)->tv_sec))
# define timeradd(a, b, result)                                               \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;                             \
    (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;                          \
    if ((result)->tv_usec >= 1000000)                                         \
      {                                                                       \
        ++(result)->tv_sec;                                                   \
        (result)->tv_usec -= 1000000;                                         \
      }                                                                       \
  } while (0)
# define timersub(a, b, result)                                               \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                             \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;                          \
    if ((result)->tv_usec < 0) {                                              \
      --(result)->tv_sec;                                                     \
      (result)->tv_usec += 1000000;                                           \
    }                                                                         \
  } while (0)


void usleep2(long us) {nanosleep((struct timespec[]){{0, us*1000}}, NULL);};

void delayMicrosecondsHard (unsigned int howLong) {
  struct timeval tNow, tLong, tEnd ;

  gettimeofday (&tNow, NULL) ;
  tLong.tv_sec  = howLong / 1000000 ;
  tLong.tv_usec = howLong % 1000000 ;
  timeradd (&tNow, &tLong, &tEnd) ;

  while (timercmp (&tNow, &tEnd, <))
    gettimeofday (&tNow, NULL) ;
}

//
// Set up a memory regions to access GPIO
//
void setup_io(){


  struct sched_param sp;
  memset(&sp, 0, sizeof(sp));
  sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
  sched_setscheduler(0, SCHED_FIFO, &sp);
  mlockall(MCL_CURRENT | MCL_FUTURE);


	/* open /dev/mem */
	if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
		printf("can't open /dev/mem \n");
		exit (-1);
	}

	/* mmap GPIO */

	// Allocate MAP block
	if ((gpio_mem = malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL) {
		printf("allocation error \n");
		exit (-1);
	}

	// Make sure pointer is on 4K boundary
	if ((unsigned long)gpio_mem % PAGE_SIZE)
	    gpio_mem += PAGE_SIZE - ((unsigned long)gpio_mem % PAGE_SIZE);

	// Now map it
	gpio_map = (unsigned char *)mmap(
		gpio_mem,
		BLOCK_SIZE,
		PROT_READ|PROT_WRITE,
		MAP_SHARED|MAP_FIXED,
		mem_fd,
		GPIO_BASE
		);

	if ((long)gpio_map < 0) {
	    printf("mmap error %d\n", (int)gpio_map);
	    exit (-1);
	}

	// Always use volatile pointer!
	gpio = (volatile unsigned *)gpio_map;
} // setup_io

/* Added functions to enable and disable carrier */

void setup_fm(int divider){
	allof7e = (unsigned *)mmap(
		NULL,
		0x01000000,  //len
		PROT_READ|PROT_WRITE,
		MAP_SHARED,
		mem_fd,
		0x20000000  //base
		);

	if ((int)allof7e==-1) exit(-1);

	SETBIT(GPFSEL0 , 14);
	CLRBIT(GPFSEL0 , 13);
	CLRBIT(GPFSEL0 , 12);


	struct GPCTL setupword = {6/*SRC*/, 1, 0, 0, 0, 1,0x5a};
	ACCESS(CM_GP0DIV) = (0x5a << 24) + divider;
	ACCESS(CM_GP0CTL) = *((int*)&setupword);
	//return divider
}

void askHigh(){
	struct GPCTL setupword = {6/*SRC*/, 1, 0, 0, 0, 1,0x5a};	// Set CM_GP0CTL.ENABLE to 1
	ACCESS(CM_GP0CTL) = *((int*)&setupword);
	while(!(ACCESS(CM_GP0CTL)&0x80)); // Wait for busy flag to turn on.
}

void askLow(){
	struct GPCTL setupword = {6/*SRC*/, 0, 0, 0, 0, 1,0x5a};	// Set CM_GP0CTL.ENABLE to 0
	ACCESS(CM_GP0CTL) = *((int*)&setupword);
	while(ACCESS(CM_GP0CTL)&0x80);
}

void sendByteAsk(unsigned char byte, int sleep){
	for (char i = 0; i < 8; i++){
		if ((byte&(1 << i)) > 0){
			askLow();
			delayMicrosecondsHard(sleep);
			//usleep2(sleep);
			askHigh();
			delayMicrosecondsHard(sleep);
			//usleep2(sleep);
		}
		else if ((byte&(1 << i)) == 0) {
			askHigh();
			delayMicrosecondsHard(sleep);
			//usleep2(sleep);
			askLow();
			delayMicrosecondsHard(sleep);
			//usleep2(sleep);
		}
	}
	askLow();
}

void sendStringAsk(char *string, int sleep){
	int length = strlen(string);
	for (int i = 0; i < length; i++){
		sendByteAsk(string[i], sleep);
	}
}
