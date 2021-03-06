PROG=	iwmbtfw

CSRCS=	iwmbtfw.c ${DSRCS}
FILES=	${shell test ! -d firmware || ls firmware/* }

CFLAGS+=-I. -I./include -Wall -Wextra
CFLAGS+=-DFIRMWARE_PATH=\"${FILESDIR}\"

CFLAGS+=${shell pkg-config --cflags libusb-1.0}
LDFLAGS=${shell pkg-config --libs libusb-1.0}

PREFIX?=	/usr/local
BINDIR?=	${PREFIX}/sbin
FILESDIR?=	${PREFIX}/share/iwmbt-firmware
INSTALL?=	install -c
MKDIR?=		install -c -d

BTSTACK=	btstack
BTSTACK_COMMIT=	ad7cba718043f6df01760889ce52fa75d6280eb9
BTSTACK_SUBDIRS=src platform/libusb platform/posix chipset/intel src/classic
BTSTACK_REGEX=	${shell echo "(${BTSTACK_SUBDIRS})" | sed -e 's/ /|/g' }

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

DSRCS=	${addprefix src/,${SOURCES}}
INCS=	${addprefix include/,${INCLUDE}}

GNUMAKE=gmake
FIND=	find -E	# find with extended regular expressions support enabled
GIT=	git

.SUFFIX:.o
.PHONY:	all clean clean-sources install sources

all:	sources ${PROG}

OBJS=	${CSRCS:.c=.o}

.c.o:	${INCS}
	${CC} ${CFLAGS} -c $< -o $@

${PROG}:${OBJS}
	${CC} ${LDFLAGS} ${OBJS} -o $@

clean:
	rm -f *.o src/*.o ${PROG}

install:all
	${MKDIR} ${DESTDIR}${BINDIR}
	${INSTALL} -s -m 755 ${PROG} ${DESTDIR}${BINDIR}
	${MKDIR} ${DESTDIR}${FILESDIR}
	${INSTALL} -m 444 ${FILES} ${DESTDIR}${FILESDIR}

clean-sources:
	rm -rf ${BTSTACK} include src firmware

sources:include src firmware

${BTSTACK}:
	${GIT} clone https://github.com/bluekitchen/${BTSTACK}.git
	${GIT} -C ${BTSTACK} checkout ${BTSTACK_COMMIT}

include: ${BTSTACK} ${INCS}
${INCS}:
	@test -d include/classic || mkdir -p include/classic
	@${FIND} ${BTSTACK} \
		-regex '${BTSTACK}/${BTSTACK_REGEX}/${@F}' \
		-print -exec cp {} $@ \;

firmware: ${BTSTACK}
	@test -d firmware || mkdir -p firmware
	@cd firmware; ${GNUMAKE} -f ../${BTSTACK}/chipset/intel/Makefile.inc

src: ${BTSTACK} ${DSRCS}
${DSRCS}:
	@test -d src || mkdir -p src
	${FIND} ${BTSTACK} \
		-regex '${BTSTACK}/${BTSTACK_REGEX}/${@F}' \
		-print -exec cp {} $@ \;
