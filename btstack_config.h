//
// btstack_config.h for Intel bluetooth firmware downloader
//

#ifndef __BTSTACK_CONFIG
#define __BTSTACK_CONFIG

// Hack to disable kernel driver detachment in hci_transport_h2_libusb.c
#ifndef __APPLE__
#define __APPLE__ 1
#endif

// Place missing hci_dump.c #include here to avoid source patching
#include <sys/time.h>

// FreeBSD version of libusb does not have libusb_pollfds_handle_timeouts
// function. Return something as BTstack discards returned value anyway.
#ifdef __FreeBSD__
#define libusb_pollfds_handle_timeouts(arg) 0
#endif

// Port related features
#define HAVE_POSIX_FILE_IO

// BTstack features that can be enabled
//#define ENABLE_LOG_DEBUG
//#define ENABLE_LOG_INFO

// BTstack configuration. buffers, sizes, ...
#define HCI_ACL_PAYLOAD_SIZE (1691 + 4)
#define HCI_INCOMING_PRE_BUFFER_SIZE 14 // sizeof BNEP header, avoid memcpy

#endif
