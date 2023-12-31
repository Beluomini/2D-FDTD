import subprocess
import sys


# ---------COMO USAR---------:
# 
# python3 auto_benchmark.py -p -s -m -h -e 1
# os parametros -p, -s e -m indicam quais testes serao executados
# -p para paralelo
# -s para sequencial
# -m para mpi

# -h significa que voce ira utilizar o tempo sequencial hardcoded
# na variavel 'hard_coded_seq', na sequencia: Small, Medium e Large.
# Atualmente com valores simbolicos, para exemplificar, caso nao 
# utilize -h, o tempo sequencial sera calculado para fazer o speedup apos

# -e indica quantas vezes cada teste sera executado, para tirar a media
# sempre que usar -e, o numero apos ele sera o numero de execucoes.
# Caso nao utilize -e, o numero de execucoes sera 1(default).
# Caso utilize -e e nao passe nenhum valor inteiro como proximo parametro
# o programa ira dar erro.

# A ordem ou sequencia de parametros passados nao importa, tirando
# a excecao do -e, que deve ser seguido de um numero inteiro.

# Para a execução com perf, basta adicionar o parametro -perf
# Com isso o perf irá printar muita coisa na tela, entao é recomendado
# redirecionar a saida para um arquivo, como por exemplo:
# python3 auto_benchmark.py -p -s -m -perf &> benchmark.txt
# o comando que redireciona é p '&>' , para o arquivo 'benchmark.txt'
# ou apos o termino, copie tudo que foi printado na tela para um arquivo
# recomendado roda com 5 execucoes, acredito que demora cerca de 4hrs


# default values
exec_names = ["fdtd_par", "fdtd_seq", "fdtd_mpi"]
default_exec = 1
number_of_threads = [2, 4, 8, 16]
number_of_clusters = [2, 4]
data_size = ["small", "medium", "large"]

# TODO: add -perf arg, to run with perf, in this case we dont capture the output
# just redirect it to a file
# prob perf cmd
# perf stat -e cpu-clock,task-clock,context-switches,cpu-migrations,page-faults,branches,branch-misses,cache-references,cache-misses,cycles,instructions ->COMMAND<-             
available_args = ["-p", "-s", "-m", "-h", "-e", "-perf"]

perf_prefix = "perf stat -e cpu-clock,task-clock,context-switches,cpu-migrations,page-faults,branches,branch-misses,cache-references,cache-misses,cycles,instructions "


par_times = []
seq_times = []
mpi_times = []

par_sum = 0
seq_sum = 0
mpi_sum = 0

# use this if you dont want to always run sequential benchmark
# small, medium, large
hard_coded_seq = [7.04, 14.10, 21.30]

def show_speedup(argv):
    if "-h" in argv:
        if hard_coded_seq == []:
            print("Faltando tempo sequencial")
            return 1
        print("Usando tempo sequencial guardado")
        seq = hard_coded_seq
    else:
        seq = seq_times
    
    if "-p" in argv:
        for i, s in enumerate(seq):
            for j, t in enumerate(number_of_threads):
                print(f"Speedup Paralelo para -d {data_size[i]}\t -t {t}\t = {s:.2f}/{par_times[i][j]:.2f} = {s/par_times[i][j]:.2f}")

    if "-m" in argv:
        for i, s in enumerate(seq):
            for j, t in enumerate(number_of_clusters):
                print(f"Speedup MPI para -d {data_size[i]}\t -t {t}\t = {s}/{mpi_times[i][j]} = {s/mpi_times[i][j]:.2f}")

    print("\n")
    print("Tempos sequencial")
    for i, s in enumerate(seq):
        print(f"Tempo sequencial usado para -d {data_size[i]} = {s:.2f}")
    
    return 0


def benchmark(argv):

    if "-e" in argv:
        pos = argv.index("-e")
        if len(argv) <= pos:
            print("Faltando numero de execucoes")
            return 1
        number_of_exec = int(argv[pos+1])
    else:
        number_of_exec = default_exec

    if "-p" in argv:
        for size in data_size:
            line = []
            for num_threads in number_of_threads:
                par_sum = 0
                for i in range(number_of_exec):
                    print(f"Executando {i} do arquivo {exec_names[0]} com {num_threads} threads e tamanho {size}")
                    
                    cmd = "./fdtd_par -d " + size + " -t " + str(num_threads)
                    if "-perf" in argv:
                        cmd = perf_prefix + cmd
                    tmp = float(subprocess.check_output(cmd, shell=True))
                    
                    par_sum += tmp
                    print(f"Tempo de execução: {tmp:.5f}")
                line.append(par_sum/number_of_exec)
            par_times.append(line)

    if "-s" in argv:
        for size in data_size:
            seq_sum = 0
            for i in range(number_of_exec):
                print(f"Executando {i} do arquivo {exec_names[1]} com tamanho {size}")

                cmd = "./fdtd_seq -d " + size
                if "-perf" in argv:
                        cmd = perf_prefix + cmd
                tmp = float(subprocess.check_output(cmd, shell=True))

                seq_sum += tmp
                print(f"Tempo de execução: {tmp:.5f}")
            seq_times.append(seq_sum/number_of_exec)
    
    if "-m" in argv:
        for size in data_size:
            line = []
            for prcss in number_of_clusters:
                mpi_sum = 0
                for i in range(number_of_exec):
                    print(f"Executando {i} do arquivo {exec_names[2]} com {prcss} threads e tamanho {size}")
                    
                    cmd = "mpirun -np " + str(prcss) + " ./fdtd_mpi -d " + size
                    if "-perf" in argv:
                        cmd = perf_prefix + cmd
                    tmp = float(subprocess.check_output(cmd, shell=True))

                    mpi_sum += tmp
                    print(f"Tempo de execução: {tmp:.5f}")
                line.append(mpi_sum/number_of_exec)
            mpi_times.append(line)

    return seq_times, par_times, mpi_times

def main():

    seq_times, par_times, mpi_times = benchmark(sys.argv)

    print("\n")
    print("Execução finalizada")
    print("\n")

    if "-p" in sys.argv:
        for si, s in enumerate(data_size):
            for ti, t in enumerate(number_of_threads):
                print(f"Tempo de paralelo para -d {s}\t -t {t}\t= {par_times[si][ti]:.2f}")
    if "-s" in sys.argv:
        for si, s in enumerate(data_size):
            print(f"Tempo de sequencial para -d {s}\t= {seq_times[si]:.2f}")

    if "-m" in sys.argv:
        for si, s in enumerate(data_size):
            for ti, t in enumerate(number_of_clusters):
                print(f"Tempo de mpi para -d {s}\t-t {t}\t= {mpi_times[si][ti]:.2f}")


    print("\n")
    print("Speedup")

    show_speedup(sys.argv)

main()