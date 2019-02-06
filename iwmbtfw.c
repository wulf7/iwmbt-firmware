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

#include "btstack_config.h"

#include <sys/time.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#ifdef SUPPORT_UGENXX
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <dev/usb/usb_ioctl.h>
#endif

#include "btstack_debug.h"
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

// Max depth for USB 3?. Keep in sync with src/hci_transport_h2_libusb.c
#define USB_MAX_PATH_LEN 7

static void
usage(void)
{
	printf("Usage: iwmbtfw (-d(dd)) (-D) (-u path) (-f firmware path)\n");
	printf("    -d: enable debugging. Repeat to increase verbosity\n");
	printf("    -D: dump HCI packets to stdout\n");
#ifdef SUPPORT_UGENXX
	printf("    -u: usb device (ugenX.X) to operate upon\n");
#else
	printf("    -u: usb path (XX(:YY(:ZZ))) to operate upon\n");
#endif
	printf("    -f: firmware path, if not default\n");
	exit(127);
}

static void
sigint_handler(int param)
{

	UNUSED(param);
	fprintf(stderr, "CTRL-C - SIGINT received, shutting down..\n");
	log_info("sigint_handler: shutting down");

	exit(2);
}

static void
intel_firmware_timeout(int param)
{

	UNUSED(param);
	fprintf(stderr, "Firmware downloading failed\n");
	log_info("intel_firmware_timeout: shutting down");

	exit(1);
}

static void
intel_firmware_done(int result)
{

	printf("Done %x\n", result);

	exit(0);
}

static int
optarg_to_usb_path(const char *optarg, uint8_t *usb_path)
{
	int usb_path_len = 0;
#ifdef SUPPORT_UGENXX
	struct usb_device_port_path usb_dpp;
	char ugen_path[16];
	int fd, iores;

	snprintf(ugen_path, sizeof(ugen_path), "%s%s", _PATH_DEV, optarg);
	if ((fd = open(ugen_path, O_RDONLY|O_CLOEXEC)) < 0) {
		fprintf(stderr, "Failed to open %s: %s\n", ugen_path,
		    strerror(errno));
		return (-1);
	}
	iores = ioctl(fd, USB_GET_DEV_PORT_PATH, &usb_dpp);
	close(fd);
	if (iores < 0) {
		fprintf(stderr, "Failed to get USB Path for %s: %s\n",
		    ugen_path, strerror(errno));
		return (-1);
	}
	if (usb_dpp.udp_port_level > USB_MAX_PATH_LEN) {
		fprintf(stderr,
		    "Retrieved USB path is too long. Revert to autodetect\n");
		return (0);
	}
	usb_path_len = usb_dpp.udp_port_level;
	memcpy(usb_path, usb_dpp.udp_port_no, usb_path_len);
#else
	char *delimiter;
	int port;

	printf("Specified USB Path: ");
	while (1){
		port = strtol(optarg, &delimiter, 16);
		usb_path[usb_path_len] = port;
		usb_path_len++;
		printf("%02x ", port);
		if (!delimiter)
			break;
		if (*delimiter != ':' && *delimiter != '-')
			break;
		optarg = delimiter + 1;
	}
	printf("\n");
#endif
	return (usb_path_len);
}

int
main(int argc, char * argv[])
{
	const hci_transport_t *transport;
	char *firmware_path = DEFAULT_FIRMWARE_PATH;
	uint8_t usb_path[USB_MAX_PATH_LEN];
	int n;
	int do_dump = 0;
	int log_level = 0;
	int usb_path_len = 0;
	struct itimerval tv;

	/* Parse command line arguments */
	while ((n = getopt(argc, argv, "Ddf:hu:")) != -1) {
		switch (n) {
		case 'd':
			log_level++;
			break;
		case 'D':
			do_dump = 1;
			break;
		case 'u': /* USB path */
			if (usb_path_len)
				break;
			usb_path_len = optarg_to_usb_path(optarg, usb_path);
			if (usb_path_len < 0)
				exit(3);
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

	/* Activate debugging facilities */
	hci_dump_enable_log_level(HCI_DUMP_LOG_LEVEL_DEBUG, log_level > 2);
	hci_dump_enable_log_level(HCI_DUMP_LOG_LEVEL_INFO, log_level > 1);
	hci_dump_enable_log_level(HCI_DUMP_LOG_LEVEL_ERROR, log_level > 0);
	if (do_dump)
		hci_dump_open(NULL, HCI_DUMP_STDOUT);

	/* Get started with BTstack */
	btstack_run_loop_init(btstack_run_loop_posix_get_instance());
	    
	if (usb_path_len)
		hci_transport_usb_set_path(usb_path_len, usb_path);

	btstack_chipset_intel_set_firmware_path(firmware_path);

	/* Setup USB Transport */
	transport = hci_transport_usb_instance();
	btstack_chipset_intel_download_firmware(transport, &intel_firmware_done);

	/* Handle CTRL-c */
	signal(SIGINT, sigint_handler);

	/* Handle downloading timeout */
	signal(SIGALRM, intel_firmware_timeout);
	memset(&tv, 0, sizeof(tv));
	tv.it_value.tv_sec = DEFAULT_TIMEOUT / 1000;
	tv.it_value.tv_usec = (DEFAULT_TIMEOUT % 1000) * 1000;
	setitimer(ITIMER_REAL, &tv, NULL);

	/* go */
	btstack_run_loop_execute();    

	return(0);
}
