CC = gcc
MPI = mpicc
MPIFLAGS = -g -Wall
SRC_PATH = ./fdtd-2d
C_INCLUDE = -I utilities -I fdtd-2d/fdtd-2d utilities/polybench.c
C_FLAGS = -O3 -DPOLYBENCH_TIME -Wall

all: fdtd_seq fdtd_par fdtd_mpi
fdtd_seq: ${SRC_PATH}/fdtd_seq.c
	echo "Compiling fdtd_seq"
	${CC} ${C_FLAGS} ${C_INCLUDE} ${SRC_PATH}/fdtd_seq.c -o fdtd_seq

fdtd_par: ${SRC_PATH}/fdtd_par.c
	echo "Compiling fdtd_par"
	${CC} ${C_FLAGS} ${C_INCLUDE} ${SRC_PATH}/fdtd_par.c -o fdtd_par

fdtd_mpi: ${SRC_PATH}/fdtd_mpi.c
	echo "Compiling fdtd_mpi"
	$(MPI) -O3 ${SRC_PATH}/fdtd_mpi.c -o fdtd_mpi

clear:
	rm -f *.o
