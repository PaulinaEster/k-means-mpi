#!/bin/bash

# Declaração associativa: cada chave é o diretório, o valor é um array de comandos make (alvo + parâmetros)
# !!ATENÇÃO!!
# Apenas execute esse cara se você já estar em maquinas alocadas, caso não tenha alocado recursos, segue comandos uteis:
# ladinfo -- para listar as maquinas que podem ser alocadas, visaulizar quantas maquinas estão livres
# PARA ALOCAR MAQUINAS: 
# -n numero de maquinas
# -t tempo que será alocado
# -e exclusivo ou -s shared
# ladalloc -n 4 -t 60 -e

declare -A mpiruns

# BENCHMARKS QUE SERÃO EXECUTADOS EM CADA PASTA

# quantidade de nodos que será utilizada na execução
numnodes=("8" "16" "32" "64" "96" "1")
#numnodes=("4" "2" "8" "16")
# classes que serão executadas
classes=("D" "E" "F" "G" "H") 
# classes=("A" "B" "C") 

# qual o diretorio dos benchmarks que serão executados e quais os benchmarks.
mpirun="k_means" 

HOME=$(pwd)

# Arquivo de log temporario
mpi=$HOME"/resultado/tempo_points.log"

# Limpa/cria arquivos de log
> "$mpi"

cd ./mpi 

logfile="./logfile"
for workload in "${classes[@]}"
do
    make WORKLOAD=$workload
    ./data_generator.$workload.exe
    for nodes in "${numnodes[@]}"
    do 
        echo -n -e "WORKLOAD: $workload NODES: $nodes" >> "$mpi" 
        (mpirun -np $nodes --oversubscribe $mpirun.$workload.exe) >> "$logfile" 2>&1
        if [ $? -ne 0 ]; then
            echo "Erro na execução do mpirun -np $nodes '$workload'. Abortando." | tee -a "$mpi"
            exit 1
        fi 
        echo -n -e " \t TEMPO: " | tee -a >> "$mpi" 
        
        sed -n 's/.*Execution time in seconds *= *\([0-9.]*\).*/\1/p' $logfile | tr -d '\n' >> "$mpi"
        grep -m1 -E "Correctness verification" "$logfile" | tr -d '\n' >> "$mpi" 

        > "$logfile" 
        echo " " >> $mpi 
    done
done
rm $logfile