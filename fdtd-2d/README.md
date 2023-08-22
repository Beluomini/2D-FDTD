# Para compilar e rodar agora
gcc -O3 -I utilities -I fdtd-2d/fdtd-2d utilities/polybench.c fdtd-2d/fdtd-2d.c -DPOLYBENCH_TIME -o fdtd
./fdtd

# Como o professor quer que rode
gcc -O3 -I utilities -I fdtd-2d/fdtd-2d utilities/polybench.c fdtd-2d/fdtd-2d.c -DPOLYBENCH_TIME -o fdtd
./fdtd -d -t

-d > tamanho da matriz (-d small) [vamos alterar o num de iterações] 
-t > num de threads

-h > help
