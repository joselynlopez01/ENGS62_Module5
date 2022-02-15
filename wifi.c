#include "wifi.h"

static XUartPs uart1;
static XUartPs uart0;

static int STATE;
static u8 long_buffer[30];
static int buffer_counter = 0;
static int numBytes_requested = 1;

typedef struct ping{
	int type; // assigned to ping
	int id; // assigned to our id on the class roaster
} ping_t;



void send_ping(){

	ping_t p;
	p.type = TYPE;
	p.id = ID;

	XUartPs_Send(&uart0, (u8*) &p, sizeof(ping_t));
}

void set_state(int s){
	STATE = s;
}


// uart 0 handler function
static void uart0_handler(void *callBackRef, u32 event, unsigned int eventData){ // Wi-fly
	if(event == XUARTPS_EVENT_RECV_DATA){
		u8 buffer;
		u32 numBytes_requested = 1;



		XUartPs *uart0 = (XUartPs*)callBackRef;

		XUartPs_Recv(uart0, &buffer, numBytes_requested);

		if (STATE == CONFIGURE){
			XUartPs_Send(&uart1, &buffer, numBytes_requested);
		}

		if (STATE == PING){
			XUartPs_Recv(uart0, &buffer, numBytes_requested);
			long_buffer[buffer_counter] = buffer;
			buffer_counter += 1;
			if (buffer_counter == 8){
				ping_t* p = (ping_t *) long_buffer;
				buffer_counter = 0;
				printf("[PING %d]\n\r", p->id);
			}
		}

	}
}

/*
 * uart1 handler
 */
static void uart1_handler(void *callBackRef, u32 event, unsigned int eventData){ //Terminal, This is done
	XUartPs * uart1 = (XUartPs *) callBackRef;

	if(event == XUARTPS_EVENT_RECV_DATA){
		u8 buffer;

		XUartPs_Recv(uart1, &buffer, numBytes_requested);
		if (STATE == CONFIGURE){
			XUartPs_Send(&uart0, &buffer, numBytes_requested);
			if (buffer == (u8) '\r'){
				buffer = (u8) '\n';
				XUartPs_Send(uart1, &buffer, numBytes_requested);
			}
		}
	}
}


/*
 * initialize the uart for the wifi module
 */
int wifi_init(void){
	XUartPs_Config * ConfigPtr0;
	ConfigPtr0 = XUartPs_LookupConfig(XPAR_PS7_UART_0_DEVICE_ID);

	if (XUartPs_CfgInitialize(&uart0,  ConfigPtr0, ConfigPtr0->BaseAddress) == XST_SUCCESS){
		u8 triggerLevel = 1;
		XUartPs_SetFifoThreshold(&uart0, triggerLevel);
		u32 baudRate = 9600;
		if (XUartPs_SetBaudRate(&uart0, baudRate) == XST_SUCCESS){
			XUartPs_SetInterruptMask(&uart0, XUARTPS_IXR_RXOVR);
			XUartPs_SetHandler(&uart0, (XUartPs_Handler) uart0_handler, &uart0);
			gic_connect(XPAR_XUARTPS_0_INTR, (Xil_InterruptHandler) XUartPs_InterruptHandler, &uart0);
		}
	} else {
		printf(" Initialization of UART was not completed");
		fflush(stdout);
		return 1;
	}

	XUartPs_Config * ConfigPtr1;
	ConfigPtr1 = XUartPs_LookupConfig(XPAR_PS7_UART_1_DEVICE_ID);
	if (XUartPs_CfgInitialize(&uart1,  ConfigPtr1, ConfigPtr1->BaseAddress) == XST_SUCCESS){

		u8 triggerLevel = 1;
		XUartPs_SetFifoThreshold(&uart1, triggerLevel);


		XUartPs_SetInterruptMask(&uart1, XUARTPS_IXR_RXOVR);
		XUartPs_SetHandler(&uart1, (XUartPs_Handler) uart1_handler, &uart1);
		gic_connect(XPAR_XUARTPS_1_INTR, (Xil_InterruptHandler) XUartPs_InterruptHandler, &uart1);
	} else {
		printf(" Initialization of UART was not completed");
		fflush(stdout);
		return 1;
	}

	return 0;
}

/*
 * close the wifi
 */
void wifi_close(void){
	XUartPs_DisableUart(&uart0);
	XUartPs_DisableUart(&uart1);
	gic_disconnect(XPAR_XUARTPS_0_INTR);
	gic_disconnect(XPAR_XUARTPS_1_INTR);
}
