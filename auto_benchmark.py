import subprocess


def main():
    ptread_arq_names = ["fdtd-par", "fdtd-seq"]
    number_of_exec = 10
    number_of_threads = [1, 2, 4, 8, 16]
    data_size = ["small", "medium", "large"]

    par_times = []
    seq_times = []

    par_sum = 0
    seq_sum = 0


    for size in data_size:
        line = []
        for num_threads in number_of_threads:
            for i in range(number_of_exec):
                #prints for debug
                #print("Executando " + str(i) + " do arquivo " + ptread_arq_names[0] + " com " + str(num_threads) + " threads e tamanho " + size)
                par_sum += float(subprocess.check_output(["./" + ptread_arq_names[0] + " -d " + size + " -t " + str(num_threads)], shell=True))
                #print("Execução " + str(i) + " do arquivo " + ptread_arq_names[0] + " com " + str(num_threads) + " threads e tamanho " + size + " finalizada")
            line.append(par_sum/number_of_exec)
        par_times.append(line)

    for size in data_size:
        for i in range(number_of_exec):
            #prints for debug
            #print("Executando " + str(i) + " do arquivo " + ptread_arq_names[0] + " com tamanho " + size)
            seq_sum += float(subprocess.check_output(["./" + ptread_arq_names[1] + " -d " + size], shell=True))
            #print("Execução " + str(i) + " do arquivo " + ptread_arq_names[0] + " com tamanho " + size + " finalizada")
        seq_times.append(seq_sum/number_of_exec)

    print("\n")
    print("Execução finalizada")
    print("Tempos paralelo")
    print(par_times)
    print("Tempos sequencial")
    print(seq_times)
    print("\n")
    for ti, t in enumerate(number_of_threads):
        for si, s in enumerate(data_size):
            print("Tempo paralelo para -d " + s + "\t-t " + str(t) + "\t= " + str(par_times[si][ti]))

    for si, s in enumerate(data_size):
        print("Tempo sequencial para -d " + s + "\t= " + str(seq_times[si]))

main()