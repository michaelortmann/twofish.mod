# Makefile for src/mod/${MYNAME}.mod/
# $Id: Makefile,v 1.11 2000/09/12 15:26:51 fabian Exp $

MYNAME=twofish
srcdir = .


doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../ && make

static: ../${MYNAME}.o

modules: ../../../${MYNAME}.$(MOD_EXT)

./twofish_cipher.o: twofish_cipher.c
	$(CC) $(CFLAGS) $(CPPFLAGS) twofish_cipher.c -c -o twofish_cipher.o

../${MYNAME}.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c $(srcdir)/${MYNAME}.c
	@rm -f ../${MYNAME}.o
	mv ${MYNAME}.o ../

../../../${MYNAME}.$(MOD_EXT): ../${MYNAME}.o ./twofish_cipher.o
	$(LD) -o ../../../${MYNAME}.$(MOD_EXT) ../${MYNAME}.o ./twofish_cipher.o $(XLIBS)
	$(STRIP) ../../../${MYNAME}.$(MOD_EXT)

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $(srcdir)/${MYNAME}.c > .depend

clean:
	@rm -f .depend *.o *.$(MOD_EXT) *~
distclean: clean

#safety hash
../${MYNAME}.o: .././${MYNAME}.mod/${MYNAME}.c ../../../src/mod/module.h \
 ../../../src/main.h ../../../config.h ../../../src/lang.h \
 ../../../src/eggdrop.h ../../../src/flags.h ../../../src/proto.h \
 ../../../lush.h ../../../src/misc_file.h ../../../src/cmdt.h \
 ../../../src/tclegg.h ../../../src/tclhash.h ../../../src/chan.h \
 ../../../src/users.h ../../../src/compat/compat.h \
 ../../../src/compat/inet_aton.h ../../../src/compat/snprintf.h \
 ../../../src/compat/memset.h ../../../src/compat/memcpy.h \
 ../../../src/compat/strcasecmp.h ../../../src/compat/strftime.h \
 ../../../src/mod/modvals.h ../../../src/tandem.h \
 ../${MYNAME}.mod/${MYNAME}.h
