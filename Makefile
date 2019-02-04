PROG=	iwmbtfw

SRCS+=	iwmbtfw.c
SRCS+=	${SOURCES:C|^.*/|src/|}
FILES!=	ls firmware/*

CFLAGS+=-I. -I./include -Wall -Wextra
CFLAGS+=-DDEFAULT_FIRMWARE_PATH=\"${FILESDIR}\"
LDADD=	-lusb
MK_MAN=	no

PREFIX=	/usr/local
BINDIR=	${PREFIX}/bin
FILESDIR=${PREFIX}/share/${PROG}

BTSTACK=btstack
SOURCES=src/hci_cmd.c src/hci_dump.c src/btstack_util.c \
	src/btstack_linked_list.c src/btstack_run_loop.c \
	platform/libusb/hci_transport_h2_libusb.c \
	platform/posix/btstack_run_loop_posix.c \
	chipset/intel/btstack_chipset_intel_firmware.c
INCLUDE=src/bluetooth.h src/btstack_chipset.h src/btstack_control.h \
	src/btstack_debug.h src/btstack_defines.h src/btstack_em9304_spi.h \
	src/btstack_event.h src/btstack_linked_list.h src/btstack_run_loop.h \
	src/btstack_uart_block.h src/btstack_util.h src/gap.h src/hci_cmd.h \
	src/hci_dump.h src/hci_transport.h src/hci.h \
	platform/posix/btstack_run_loop_posix.h \
	chipset/intel/btstack_chipset_intel_firmware.h
CLASSIC=btstack_link_key_db.h sdp_util.h

sources: include src firmware

${BTSTACK}:
	git clone https://github.com/bluekitchen/${BTSTACK}.git
	git -C ${BTSTACK} checkout e034024d16933df0720cab3582d625294e26c667

include: ${BTSTACK}
	mkdir -p include/classic
.for FILE in ${INCLUDE}
	cp ${BTSTACK}/${FILE} include
.endfor
.for FILE in ${CLASSIC}
	cp ${BTSTACK}/src/classic/${FILE} include/classic
.endfor

firmware: ${BTSTACK}
	mkdir -p firmware
	cd firmware; gmake -f ../${BTSTACK}/chipset/intel/Makefile.inc

src: ${BTSTACK}
	mkdir -p src
.for FILE in ${SOURCES}
	cp ${BTSTACK}/${FILE} src
.endfor

.include <bsd.prog.mk>
