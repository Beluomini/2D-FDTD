mpi: fdtd-2d/fdtd_mpi.c fdtd_par fdtd_seq
	mpicc -O3 fdtd-2d/fdtd_mpi.c -o fdtd_mpi

fdtd_par: fdtd-2d/fdtd_par.c utilities/polybench.c
	gcc -O3 -I utilities -I fdtd-2d/fdtd-2d utilities/polybench.c fdtd-2d/fdtd_par.c -DPOLYBENCH_TIME -o fdtd_par
fdtd_seq: fdtd-2d/fdtd_seq.c utilities/polybench.c
	gcc -O3 -I utilities -I fdtd-2d/fdtd-2d utilities/polybench.c fdtd-2d/fdtd_seq.c -DPOLYBENCH_TIME -o fdtd_seq
