//
// btstack_config.h for Intel bluetooth firmware downloader
//

#ifndef __BTSTACK_CONFIG
#define __BTSTACK_CONFIG

// Detach kernel driver on FreeBSD to workaround firmware bootloader lock up
// caused by FreeBSD bluetooth stack initialization procedure. This action
// does not fix lock up but allow firmware downloader to win OS boot-time race
// against bt-stack init script (/etc/rc.d/bluetooth) so at the init time the
// bootloader is already replaced with fully functional firmware.
#ifdef __FreeBSD__
#define DETACH_KERNEL_DRIVER
#endif

// Specify USB device path in FreeBSD format (ugenX.X) rather than libusb one.
#ifdef __FreeBSD__
#define SUPPORT_UGENXX
#endif

// This hack disables USB reset issued by firmware downloader at the very
// beginning and inserts it after succesful firmware downloading has happened.
// Original code triggers USB bus probe/attach sequence at wrong place thus
// negating effect of kernel driver detachment
#include <libusb.h>
void iwmbtfw_libusb_close(libusb_device_handle *handle);
#ifndef DONT_OVERRIDE_LIBUSB
#define libusb_reset_device(arg) 0
#define libusb_close(arg) iwmbtfw_libusb_close(arg)
#endif

// Defining __APPLE__ is a hack to disable kernel driver detachment
// in hci_transport_h2_libusb.c
#if !defined(__APPLE__) && !defined(DETACH_KERNEL_DRIVER)
#define __APPLE__
#endif

// Place missing hci_dump.c #include here to avoid source patching
#include <sys/time.h>

// FreeBSD version of libusb does not have libusb_pollfds_handle_timeouts
// function. Return something as BTstack discards returned value anyway.
#ifdef __FreeBSD__
#define libusb_pollfds_handle_timeouts(arg) 0
#endif

// FreeBSD version of libusb have broken libusb_set_debug implementation
// It sets debug level rather than log level. So disable it to stop spam.
#ifdef __FreeBSD__
#define libusb_set_debug(arg1, arg2) 0
#endif

// Port related features
#define HAVE_POSIX_FILE_IO

// BTstack features that can be enabled
#define ENABLE_LOG_DEBUG
#define ENABLE_LOG_INFO
#define ENABLE_LOG_ERROR

// BTstack configuration. buffers, sizes, ...
#define HCI_ACL_PAYLOAD_SIZE (1691 + 4)
#define HCI_INCOMING_PRE_BUFFER_SIZE 14 // sizeof BNEP header, avoid memcpy

#endif
