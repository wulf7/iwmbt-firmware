PROG=	iwmbtfw

SRCS+=	iwmbtfw.c
SRCS+=	${SOURCES:S|^|src/|}
FILES!=	test ! -d firmware || ls firmware/*
FILES+=	iwmbt-firmware.conf

CFLAGS+=-I. -I./include -Wall -Wextra
CFLAGS+=-DFIRMWARE_PATH=\"${FILESDIR}\"
LDADD=	-lusb
MK_MAN=	no
MK_DEBUG_FILES=	no

PREFIX?=	/usr/local
BINDIR?=	${PREFIX}/sbin
FILESDIR?=	${PREFIX}/share/iwmbt-firmware
DEVDDIR?=	${PREFIX}/etc/devd
FILESDIR_iwmbt-firmware.conf=	${DEVDDIR}

BTSTACK=	btstack
BTSTACK_COMMIT=	e034024d16933df0720cab3582d625294e26c667
BTSTACK_SUBDIRS=src platform/libusb platform/posix chipset/intel
BTSTACK_REGEX!=	echo "(${BTSTACK_SUBDIRS})" | sed -e 's/ /|/g'

SOURCES=hci_cmd.c hci_dump.c hci_transport_h2_libusb.c \
	btstack_util.c btstack_linked_list.c btstack_run_loop.c \
	btstack_run_loop_posix.c btstack_chipset_intel_firmware.c
INCLUDE=bluetooth.h btstack_chipset.h btstack_control.h \
	btstack_debug.h btstack_defines.h btstack_em9304_spi.h \
	btstack_event.h btstack_linked_list.h btstack_run_loop.h \
	btstack_uart_block.h btstack_util.h gap.h \
	hci_cmd.h hci_dump.h hci_transport.h hci.h \
	btstack_run_loop_posix.h btstack_chipset_intel_firmware.h \
	classic/btstack_link_key_db.h classic/sdp_util.h

GNUMAKE=gmake
FIND=	find -E	# find with extended regular expressions support enabled
GIT=	git

.PHONY:	clean-sources sources
clean-sources:
	rm -rf ${BTSTACK} include src firmware

sources:include src firmware

${BTSTACK}:
	${GIT} clone https://github.com/bluekitchen/${BTSTACK}.git
	${GIT} -C ${BTSTACK} checkout ${BTSTACK_COMMIT}

include: ${BTSTACK} ${INCLUDE:S|^|include/|}
${INCLUDE:S|^|include/|}:
	@test -d include/classic || mkdir -p include/classic
	@${FIND} ${BTSTACK} \
		-regex '${BTSTACK}/${BTSTACK_REGEX}/${@:S|^include/||}' \
		-print -exec cp {} $@ \;

firmware: ${BTSTACK}
	@test -d firmware || mkdir -p firmware
	@cd firmware; ${GNUMAKE} -f ../${BTSTACK}/chipset/intel/Makefile.inc

src: ${BTSTACK} ${SOURCES:S|^|src/|}
${SOURCES:S|^|src/|}:
	@test -d src || mkdir -p src
	@${FIND} ${BTSTACK} \
		-regex '${BTSTACK}/${BTSTACK_REGEX}/${@:S|^src/||}' \
		-print -exec cp {} $@ \;

.include <bsd.prog.mk>
