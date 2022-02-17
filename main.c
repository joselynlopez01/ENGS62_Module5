#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "xil_printf.h"
#include <unistd.h>
#include <stdbool.h>
#include "xuartps.h"
#include "led.h"
#include "gic.h"		/* interrupt controller interface */
#include "io.h"
#include "xuartps_hw.h"
#include "wifi.h"

static bool done = false;
//u8 buff[30];

void wifi_callback(void *buffer){

	char * sbuff = (char*) buffer;
	int value = atoi(sbuff);
	printf("the buffer is %d\n", *((int*)buffer));
	printf("the value is %d\n",value);
	send_update(value);
	set_state(UPDATE);
}

void callback(u32 led_num){
	led_toggle(led_num);

	if (led_num == CONFIGURE){
		set_state(CONFIGURE);
		printf("[CONFIGURE]\n\r");
		fflush(stdout);

	} else if (led_num == PING){
		set_state(PING);
		send_ping();
		printf("[PING]\n\r");
		fflush(stdout);

	} else if (led_num == UPDATE){
		printf("Button interrupt\n\r");
		set_state(READ);
		printf("[UPDATE]\n\r");
		fflush(stdout);
	} else if (led_num == 3){
		done = true;
	}
}

int main()
{
	/* do some initialization */
	init_platform();
	led_init();

	if (gic_init() == XST_SUCCESS){
		io_btn_init(callback);
		io_sw_init(callback);
		wifi_init(wifi_callback);
	}  else {
		printf("GIC cannot connect");
		fflush(stdout);
		return 3;
	}


	printf("[hello]\n");
	while (!done){
		sleep(1);
	}
	printf("[done]\n");
	sleep(1);


	io_btn_close();
	io_sw_close();
	wifi_close();
	gic_close();
	cleanup_platform();
	return 0;
}
