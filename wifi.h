/*
 * wifi.h
 *
 */
#pragma once

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include <unistd.h>
#include <stdbool.h>
#include "xuartps_hw.h"
#include "gic.h"		/* interrupt controller interface */

#define CONFIGURE 0
#define PING 1
#define UPDATE 2

#define ID 12
#define TYPE 1

typedef struct ping ping_t;

/*
 * initialize the uart for the wifi module
 */
int wifi_init(void);

/*
 * close the wifi
 */
void wifi_close(void);


void set_state(int s);

void send_ping (void);
