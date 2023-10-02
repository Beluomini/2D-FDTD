import subprocess
import sys

ptread_arq_names = ["fdtd_par", "fdtd_seq"]
number_of_exec = 1
number_of_threads = [2, 4, 8, 16]
data_size = ["small", "medium", "large"]

par_times = []
seq_times = []

par_sum = 0
seq_sum = 0


def show_speedup(seq, par):
    for i, s in enumerate(seq):
        for j, t in enumerate(number_of_threads):
            print(f"Speedup para -d {data_size[i]}\t -t {t}\t = {s/par[i][j]:.2f}")

    print("\n")
    print("Tempos sequencial")
    for i, s in enumerate(seq):
        print(f"Tempo sequencial para -d {data_size[i]} = {s:.2f}")
    

    #print(seq)
    #print(par)


def benchmark():
    for size in data_size:
        line = []
        for num_threads in number_of_threads:
            par_sum = 0
            for i in range(number_of_exec):
                #prints for debug
                print(f"Executando {i} do arquivo {ptread_arq_names[0]} com {num_threads} threads e tamanho {size}")
                tmp= float(subprocess.check_output(["./" + ptread_arq_names[0] + " -d " + size + " -t " + str(num_threads)], shell=True))
                par_sum += tmp
                print(f"Tempo de execução: {tmp}")
            line.append(par_sum/number_of_exec)
        par_times.append(line)

    for size in data_size:
        seq_sum = 0
        for i in range(number_of_exec):
            #prints for debug
            print(f"Executando {i} do arquivo {ptread_arq_names[1]} com tamanho {size}")
            tmp= float(subprocess.check_output(["./" + ptread_arq_names[1] + " -d " + size], shell=True))
            seq_sum += tmp
            print(f"Tempo de execução: {tmp}")
        seq_times.append(seq_sum/number_of_exec)
    return seq_times, par_times

def main():

    #argc = len(sys.argv)
    #argv = sys.argv

    seq_times, par_times = benchmark()

    print("\n")
    print("Execução finalizada")
    print("\n")
    for si, s in enumerate(data_size):
        for ti, t in enumerate(number_of_threads):
            print("Tempo paralelo para -d " + s + "\t-t " + str(t) + "\t= " + str(par_times[si][ti]))

    for si, s in enumerate(data_size):
        print("Tempo sequencial para -d " + s + "\t= " + str(seq_times[si]))

    print("\n")
    print("Speedup")

    show_speedup(seq_times, par_times)

main()