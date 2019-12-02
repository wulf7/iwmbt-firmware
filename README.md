# Deprecation notice (FreeBSD)

This piece of software is obsoleted (on FreeBSD) by native firmware
downloader which is committed to the source tree as r351196 (13-CURRENT) and
merged to 12-STABLE as r352101.

If you still have to use this downloader and are observing "Firmware
downloading failed" error message at the start, please see comment #9
to PR/237083 in FreeBSD bugzilla:

  https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=237083

# Bluetooth firmware downloader for Intel Wireless 8260/8265 Controllers

**iwmbtfw** is a fork of BTstack (https://github.com/bluekitchen/btstack)
application stripped down to act as a standalone Intel Wireless 8260/8265
bluetooth adaptor firmware downloader.

It supports only devices with USB VID/PID of 0x8087/0x0a2b.
Devices having other VID/PID combinations are not supported, although
it looks trivial to make some newer models e.g. PIDs 0x8087/0x0025,
0x8087/0x0026, 0x8087/0x0029 and 0x8087/0x0aaa working. Look btusb.c file in
Linux kernel sources for devices tagged with BTUSB_INTEL_NEW attribute.

Support for older Intel bluetooth controllers like 0x8087/0x07dc and
0x8087/0x0a2a is not planned due to different firmware downloading scheme.

## Requirements

* gcc/clang, GNU make, pkg-config
* libusb as external dependency
* FreeBSD (Mac OS X and NetBSD should work after minor tweaking)
* GNU make, git, curl and extended regex-awared find is required for bootstrapping from upstream source repository

It is created primarily for FreeBSD but can be easily adopted to any other
operating system that has libusb ported to.

## Downloading

This project does not have a special home page. The source code and the
issue tracker are hosted on Github:

  https://github.com/wulf7/iwmbt-firmware

Note: This project is a tiny wrapper over BTstack and heavily uses its
source code in unmodified form. So in most cases suggestions, bug reports
and push requests should be sent directly upstream:

  https://github.com/bluekitchen/btstack

## Building

Install libusb if it is not bundled with your OS. E.g. with homebrew on MacOSX:

  $ brew install libusb-compat

Install pkg-config. E.g. with homebrew on MacOSX:

  $ brew install pkg-config

You may edit some Makefile variables like CFLAGS and LDFLAGS with hands
to handle libusb files location if pkg-config does not do it properly.

Build iwmbtfw from a git snaphost (assuming GNU make name is gmake):

  $ gmake

Bootstrapping of vendor code from upstream repository can be made with:

  $ gmake BTSTACK_COMMIT=<git commit-hash> clean-sources sources

Of course, it is up to you to make sources buildable if upstream commit
hash differs from one specified in enclosed Makefile.

## Installing

To install files already built just type:

  $ sudo gmake install

It installs downloader executable, and firmwares.

## Running

To download firmware to Intel Wireless 8260/8265 Controllers run:

  $ sudo iwmbtfw

You can add -h command line option to see other supported options.

## Caveats

Currently FreeBSD bluetooth stack initialization script (/etc/rc.d/bluetooth)
causes lock up of firmware bootloader at one of early stages of execution.
That means that you **must** always run firmware downloader before init
script to get bootloader replaced with fully functional firmware otherwise
full power off/on or suspend/resume cycle is required to restore device.
That creates problem with devd which tries to run both tasks in parallel.
To workaround it **iwmbtfw** performs kernel driver detachment on start
that allows firmware downloader to win the race against init script.

## License

Due to way it written, different parts of **iwmbt-firmware** is redistributed
under the terms of different licenses. See LICENSE file for details.

## TODO

* Manpage
* Improve Make-fu
