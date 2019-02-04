/*-
 * Copyright (c) 2019 Vladimir Kondratyev <vladimir@kondratyev.su>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 *      
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 */

#define __BTSTACK_FILE__ "main.c"

/* minimal setup for HCI code */

#include <sys/time.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "btstack_config.h"

#include "btstack_defines.h"
#include "btstack_run_loop.h"
#include "btstack_run_loop_posix.h"
#include "btstack_chipset_intel_firmware.h"
#include "hci_dump.h"

#ifndef DEFAULT_FIRMWARE_PATH
#define DEFAULT_FIRMWARE_PATH "firmware"
#endif

#ifndef DEFAULT_TIMEOUT
#define DEFAULT_TIMEOUT 5000 /* msec. */
#endif

#define USB_MAX_PATH_LEN 7

static void
usage(void)
{
	printf("Usage: iwmbtfw (-D) (-u 11:22) (-f firmware path)\n");
	printf("    -D: enable debugging\n");
	printf("    -u: usb path to operate upon\n");
	printf("    -f: firmware path, if not default\n");
	exit(127);
}

static void
sigint_handler(int param)
{

	UNUSED(param);
	fprintf(stderr, "CTRL-C - SIGINT received, shutting down..\n");   

	exit(2);
}

static void
intel_firmware_timeout(int param)
{

	UNUSED(param);
	fprintf(stderr, "Firmware downloading failed\n");   

	exit(1);
}

static void
intel_firmware_done(int result)
{

	printf("Done %x\n", result);

	exit(0);
}

int
main(int argc, char * argv[])
{
	const hci_transport_t *transport;
	char *firmware_path = DEFAULT_FIRMWARE_PATH;
	uint8_t usb_path[USB_MAX_PATH_LEN];
	int n;
	int do_debug = 0;
	int usb_path_len = 0;
	const char *usb_path_string = NULL;
	struct itimerval tv;

	/* Parse command line arguments */
	while ((n = getopt(argc, argv, "Dd:f:hu:")) != -1) {
		switch (n) {
		case 'D':
			do_debug = 1;
			break;
		case 'u': /* parse command line options for "-u 11:22:33" */
			if (usb_path_len)
				break;
			usb_path_string = optarg;
			printf("Specified USB Path: ");
			while (1){
				char * delim;
				int port = strtol(usb_path_string, &delim, 16);
				usb_path[usb_path_len] = port;
				usb_path_len++;
				printf("%02x ", port);
				if (!delim)
					break;
				if (*delim != ':' && *delim != '-')
					break;
				usb_path_string = delim + 1;
			}
			printf("\n");
			break;
		case 'f': /* firmware path */
			firmware_path = optarg;
			break;
		case 'h':
			usage();
			break;
		default:
			break;
			/* NOT REACHED */
		}
	}

	/* Get started with BTstack */
	btstack_run_loop_init(btstack_run_loop_posix_get_instance());
	    
	if (usb_path_len)
		hci_transport_usb_set_path(usb_path_len, usb_path);

	btstack_chipset_intel_set_firmware_path(firmware_path);

	if (do_debug)
		hci_dump_open(NULL, HCI_DUMP_STDOUT);

	/* Setup USB Transport */
	transport = hci_transport_usb_instance();
	btstack_chipset_intel_download_firmware(transport, &intel_firmware_done);

	/* Handle CTRL-c */
	signal(SIGINT, sigint_handler);

	/* Handle downloading timeout */
	signal(SIGALRM, intel_firmware_timeout);
	memset(&tv, 0, sizeof(tv));
	tv.it_value.tv_sec = DEFAULT_TIMEOUT / 1000;
	tv.it_value.tv_usec = DEFAULT_TIMEOUT % 1000;;
	setitimer(ITIMER_REAL, &tv, NULL);

	/* go */
	btstack_run_loop_execute();    

	return(0);
}
