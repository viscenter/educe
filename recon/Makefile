files=rec.cc
FLAGS = -D TIMING -Wall
LIBS = -lm
debugfl=-g
OPTIMIZ = -funroll-loops -fomit-frame-pointer -O3
NRRD = -I/homes/thb/src/teem-1.9.0-src/include/ -L/homes/thb/src/teem-1.9.0-src/linux.32/lib/ -lteem 

BINS=t_rec recon_clean # t_rec_binary

all: ${BINS}

amd64: OPTIMIZ+=-march=athlon64
amd64: ${BINS}

pentium4: OPTIMIZ+=-march=pentium4
pentium4: ${BINS}

core: OPTIMIZ+=-march=prescott
core: ${BINS}

t_rec: t_rec.c
	mpicc t_rec.c ${FLAGS} ${OPTIMIZ} ${LIBS} -o t_rec

recon_clean: recon_clean.c
	mpicc recon_clean.c ${FLAGS} ${OPTIMIZ} ${LIBS} -o recon_clean

t_rec_binary: t_rec_binary.c
	mpicc t_rec_binary.c ${FLAGS} ${OPTIMIZ} ${LIBS} ${NRRD} -o t_rec_binary

clean:
	rm -f *.o
	rm -f *.mod
	rm -f ${BINS}
