# Bluetooth firmware downloader for Intel Wireless 8260/8265 Controllers

**iwmbtfw** is a fork of BTstack (https://github.com/bluekitchen/btstack)
application stripped down to act as a standalone Intel Wireless 8260/8265
bluetooth adaptor firmware downloader.

It is created primarily for FreeBSD but can be easily adopted to any other
operating system that has libusb ported to. At the moment of writing the
most unportable part is Makefile.

## TODO
* Add 'usb bus.unit' to 'usb path' convertor to handle ugenX.X addresses
* Manpage
* Improve Make-fu
