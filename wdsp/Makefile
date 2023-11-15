#
# Makefile for WDSP library
# This one creates libwdsp.a intended for statis linking
#

# get the OS Name
UNAME_S := $(shell uname -s)

CFLAGS?= -g -O3 -D_GNU_SOURCE
LINK?=   $(CC)
# 
# The "official" way to compile+link with pthreads is now to use the -pthread option
# *both* for the compile and the link step.
# 
CFLAGS+=-pthread
LINK+=-pthread


FFTWLIB=`pkg-config --libs fftw3`
FFTWINCLUDE=`pkg-config --cflags fftw3`

COMPILE=$(CC) $(CFLAGS) $(FFTWINCLUDE)

SOURCES= amd.c\
ammod.c\
amsq.c\
analyzer.c\
anf.c\
anr.c\
bandpass.c\
calcc.c\
calculus.c\
cblock.c\
cfcomp.c\
cfir.c\
channel.c\
comm.c\
compress.c\
delay.c\
dexp.c\
div.c\
eer.c\
emnr.c\
emph.c\
eq.c\
fcurve.c\
fir.c\
firmin.c\
fmd.c\
fmmod.c\
fmsq.c\
gain.c\
gen.c\
icfir.c\
iir.c\
iobuffs.c\
iqc.c\
linux_port.c\
lmath.c\
main.c\
meter.c\
meterlog10.c\
nbp.c\
nob.c\
nobII.c\
osctrl.c\
patchpanel.c\
resample.c\
rmatch.c\
RXA.c\
sender.c\
shift.c\
siphon.c\
slew.c\
snb.c\
ssql.c \
syncbuffs.c\
TXA.c\
utilities.c\
varsamp.c\
version.c\
wcpAGC.c\
wisdom.c

HEADERS=amd.h\
ammod.h\
amsq.h\
analyzer.h\
anf.h\
anr.h\
bandpass.h\
calcc.h\
calculus.h\
cblock.h\
cfcomp.h\
cfir.h\
channel.h\
comm.h\
compress.h\
delay.h\
dexp.h\
div.h\
eer.h\
emnr.h\
emph.h\
eq.h\
fastmath.h\
fcurve.h\
fir.h\
firmin.h \
fmd.h\
fmmod.h\
fmsq.h\
gain.h\
gen.h\
icfir.h\
iir.h\
iobuffs.h\
iqc.h\
linux_port.h\
lmath.h\
main.h\
meter.h\
meterlog10.h\
nbp.h\
nob.h\
nobII.h\
osctrl.h\
patchpanel.h\
resample.h\
resource.h\
rmatch.h\
RXA.h\
sender.h\
shift.h\
siphon.h\
slew.h\
snb.h\
ssql.h \
syncbuffs.h\
TXA.h\
utilities.h\
wcpAGC.h

OBJS=linux_port.o\
amd.o\
ammod.o\
amsq.o\
analyzer.o\
anf.o\
anr.o\
bandpass.o\
calcc.o\
calculus.o\
cblock.o\
cfcomp.o\
cfir.o\
channel.o\
comm.o\
compress.o\
delay.o\
dexp.o\
div.o\
eer.o\
emnr.o\
emph.o\
eq.o\
fcurve.o\
fir.o\
firmin.o\
fmd.o\
fmmod.o\
fmsq.o\
gain.o\
gen.o\
icfir.o\
iir.o\
iobuffs.o\
iqc.o\
lmath.o\
main.o\
meter.o\
meterlog10.o\
nbp.o\
nob.o\
nobII.o\
osctrl.o\
patchpanel.o\
resample.o\
rmatch.o\
RXA.o\
sender.o\
shift.o\
siphon.o\
slew.o\
snb.o\
ssql.o \
syncbuffs.o\
TXA.o\
utilities.o\
version.o\
varsamp.o\
wcpAGC.o\
wisdom.o

libwdsp.a:	$(OBJS)
	ar rv libwdsp.a $(OBJS)
	ranlib libwdsp.a

.c.o:
	$(COMPILE) -c -o $@ $<


clean:
	-rm -f libwdsp.a *.o

