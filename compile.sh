#!/usr/bin/bash
echo "Compilig and linkink 'fdtd_par.c' and 'fdtd_seq.c' "
gcc -O3 -I utilities -I fdtd-2d/fdtd-2d utilities/polybench.c fdtd-2d/fdtd_par.c -DPOLYBENCH_TIME -o fdtd_par
gcc -O3 -I utilities -I fdtd-2d/fdtd-2d utilities/polybench.c fdtd-2d/fdtd_seq.c -DPOLYBENCH_TIME -o fdtd_seq

