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
twofish.o: .././twofish.mod/twofish.c ../../../src/mod/module.h \
 ../../../src/main.h ../../../config.h ../../../eggint.h ../../../lush.h \
 ../../../src/lang.h ../../../src/eggdrop.h ../../../src/compat/in6.h \
 ../../../src/flags.h ../../../src/cmdt.h ../../../src/tclegg.h \
 ../../../src/tclhash.h ../../../src/chan.h ../../../src/users.h \
 ../../../src/compat/compat.h ../../../src/compat/snprintf.h \
 ../../../src/compat/strlcpy.h ../../../src/mod/modvals.h \
 ../../../src/tandem.h .././twofish.mod/twofish.h .././twofish.mod/aes.h \
 .././twofish.mod/platform.h
