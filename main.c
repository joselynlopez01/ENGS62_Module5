#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include <unistd.h>
#include <stdbool.h>
#include "xuartps.h"
#include "led.h"
#include "gic.h"		/* interrupt controller interface */
#include "io.h"

static XUartPs uart;
static bool done;

void getLine (char *str);
void callback(u32 led_num){
	led_toggle(led_num);
}

void handler(void *callBackRef, u32 event, unsigned int eventData){

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
		if (XUartPs_CfgInitialize(&uart,  ConfigPtr, ConfigPtr->BaseAddress) == XST_SUCCESS){
			u32 baudRate = 9600;
			if (XUartPs_SetBaudRate(&uart, baudRate) ==  XST_SUCCESS){
				u8 triggerLevel = 1;
				XUartPs_SetFifoThreshold(&uart, triggerLevel);
				XUartPs_SetInterruptMask(&uart, XPAR_XUARTPS_1_INTR);
//				XUartPs_SetHandler(&uart, XUartPs_Handler FuncPtr, void *CallBackRef);
				XUartPs_SetHandler(&uart, handler, void *CallBackRef);
				u32 XUartPs_Send(&uart,u8 *BufferPtr,u32 NumBytes);
				u32 XUartPs_Recv(&uart,u8 *BufferPtr,u32 NumBytes);
			}

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
	XUartPs_DisableUart(&uart);
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
