import matplotlib.pyplot as plt


threads = [2, 4, 8, 16]

hard_coded_seq = [7.04, 14.10, 21.30]


# exemplos d como estar os dados
small_par_lov = [4, 2.3, 1.6, 0.4]
medium_par_lov = [6.2, 3.3, 2.2, 1.6]
large_par_lov = [7.04, 3.6, 2.4, 2.0]

small_mpi_lov = [3.8, 2.1, 1.3, 0.3]
medium_mpi_lov = [5.8, 3.0, 2.0, 1.4]
large_mpi_lov = [7.0, 3.0, 2.2, 1.5]



ambery = "#ffbf00"

# para linas, small, medium, large
lred = "#e97452"
mred = "#c34632"
dred = "#8b0001"

# para chatas, small, medium, large
lblue = "9ec2ff"
mblue = "4259c3"
dblue = "03018c"

# para belu, small, medium, large
lgreen = "b7ffbf"
mgreen = "4ded30"
dgreen = "00ab08"

li = [small_par_lov, medium_par_lov, large_par_lov]
ch = [small_mpi_lov, medium_mpi_lov, large_mpi_lov]
be = [small_par_lov, medium_par_lov, large_par_lov]

def plot_speedup(linas, chatas, belus):
    plt.figure(figsize=(10, 10))
    plt.subplot(3, 1, 1)

    # linas -> vermelho
    # belu -> laranja
    # chata -> azul

    # talvez nao precise calcular, benchmark ja faz isso
    par_smalls_spd = [hard_coded_seq[0]/small_par_lov[i] for i in range(len(threads))]
    par_mediums_spd = [hard_coded_seq[1]/medium_par_lov[i] for i in range(len(threads))]
    par_large_spd = [hard_coded_seq[2]/large_par_lov[i] for i in range(len(threads))]

    mpi_smalls_spd = [hard_coded_seq[0]/small_mpi_lov[i] for i in range(len(threads))]
    mpi_mediums_spd = [hard_coded_seq[1]/medium_mpi_lov[i] for i in range(len(threads))]
    mpi_large_spd = [hard_coded_seq[2]/large_mpi_lov[i] for i in range(len(threads))]

    # caso nao precisa apenas usar parametro passado: smalld, mediumd, larged

    # plot linas, chatas, belu small
    plt.subplot(3, 1, 1)
    plt.title("Dataset Small")
    plt.plot(threads, threads,"--", label="SpeedUp Ideal", color=ambery)
    plt.plot(threads, par_smalls_spd, label="Molina", color=lred)
    
    
    plt.legend()
    plt.grid()

    # plot linas, chatas, belu medium
    plt.subplot(3, 1, 2)
    plt.title("Dataset Medium")
    plt.plot(threads, threads,"--", label="SpeedUp Ideal", color=ambery)
    plt.plot(threads, par_mediums_spd, label="Molina", color=mred)

    plt.ylabel("SpeedUp")
    plt.legend()
    plt.grid()
    
    # plot linas, chatas, belu big
    plt.subplot(3, 1, 3)
    plt.title("Dataset Large")
    plt.plot(threads, threads,"--", label="SpeedUp Ideal", color=ambery)
    plt.plot(threads, par_large_spd, label="Molina", color=dred)
    plt.xlabel("Threads")

    plt.legend()
    plt.grid()
    
    #plot chatas

    #plot belu
    
    plt.show()

# speedup x threads(process)
def plot_efficiency(linas, chatas, belus):
    plt.figure(figsize=(10, 10))

    plt.subplot(3, 1, 1)


    seff_l = [linas[0][i]/threads[i] for i in range(len(threads))]
    meff_l = [linas[1][i]/threads[i] for i in range(len(threads))]
    leff_l = [linas[2][i]/threads[i] for i in range(len(threads))]

    seff_c = [chatas[0][i]/threads[i] for i in range(len(threads))]
    meff_c = [chatas[1][i]/threads[i] for i in range(len(threads))]
    leff_c = [chatas[2][i]/threads[i] for i in range(len(threads))]

    seff_b = [belus[0][i]/threads[i] for i in range(len(threads))]
    meff_b = [belus[1][i]/threads[i] for i in range(len(threads))]
    leff_b = [belus[2][i]/threads[i] for i in range(len(threads))]


    print(seff_l)
    print(meff_l)
    print(leff_l)
    

    # plot linas, chatas, belu small
    plt.subplot(3, 1, 1)
    plt.title("Dataset Small")
    plt.plot(threads, threads,"--", label="SpeedUp Ideal", color=ambery)
    
    plt.plot(threads, seff_l, label="Molina", color=lred)
    
    
    plt.legend()
    plt.grid()

    # plot linas, chatas, belu medium
    plt.subplot(3, 1, 2)
    plt.title("Dataset Medium")
    plt.plot(threads, threads,"--", label="SpeedUp Ideal", color=ambery)
    
    plt.plot(threads, meff_l, label="Molina", color=mred) 
    

    plt.ylabel("SpeedUp")
    plt.legend()
    plt.grid()
    
    # plot linas, chatas, belu big
    plt.subplot(3, 1, 3)
    plt.title("Dataset Large")
    plt.plot(threads, threads,"--", label="SpeedUp Ideal", color=ambery)
    
    plt.plot(threads, leff_l, label="Molina", color=dred)
    
    
    plt.xlabel("Threads")
    plt.legend()
    plt.grid()
    
    
    plt.show()


plot_speedup(li, ch, be)
plot_efficiency(li, ch, be)