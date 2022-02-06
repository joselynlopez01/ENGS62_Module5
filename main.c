#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include <unistd.h>
#include <stdbool.h>
#include "xuartps.h"
#include "led.h"
#include "gic.h"		/* interrupt controller interface */
#include "io.h"
#include "xuartps_hw.h"

static XUartPs uart1;
static bool done = false;

void getLine (char *str);
void callback(u32 led_num){
	led_toggle(led_num);
	if (led_num == 3){
		done = true;
	}
}

void uart_handler(void *callBackRef, u32 event, unsigned int eventData){
	if(event == XUARTPS_EVENT_RECV_DATA){
		u8 buffer;
		u32 numBytes_requested = 1;

		XUartPs_Recv(&uart1, &buffer, numBytes_requested);
		XUartPs_Send(&uart1, &buffer, numBytes_requested);
		if (buffer == (u8) '\r'){
			buffer = (u8) '\n';
			XUartPs_Send(&uart1, &buffer, numBytes_requested);
		}
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

		XUartPs_Config * ConfigPtr;
		ConfigPtr = XUartPs_LookupConfig(XPAR_PS7_UART_1_DEVICE_ID);
		if (XUartPs_CfgInitialize(&uart1,  ConfigPtr, ConfigPtr->BaseAddress) == XST_SUCCESS){

			u8 triggerLevel = 1;
			XUartPs_SetFifoThreshold(&uart1, triggerLevel);


			XUartPs_SetInterruptMask(&uart1, XUARTPS_IXR_RXOVR);
			XUartPs_SetHandler(&uart1, (XUartPs_Handler) uart_handler, &uart1);
			gic_connect(XPAR_XUARTPS_1_INTR, (Xil_InterruptHandler) XUartPs_InterruptHandler, &uart1);

		}
	}


	printf("[hello]\n");
	while (!done){
		sleep(1);
	}
	printf("[done]\n");
	sleep(1);


	io_btn_close();
	io_sw_close();
	XUartPs_DisableUart(&uart1);
	gic_close();
	cleanup_platform();
	return 0;
}

void getLine(char * str){
	char c;
	int i = 0;

	c = getchar();
	printf("%c", c);
	fflush(stdout);

	while ( c != 13 ){
		str[i] = c;
		i++;
		c = getchar();
		printf("%c", c);
		fflush(stdout);
	}
	str[i] = '\0';
}
