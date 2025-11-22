RESULTFILE="../resultado"
TEMPFILE="../tempo"
echo "" > $RESULTFILE

echo "Compilando arquivos"
cd ./phases-parallels
make WORKLOAD=D
make WORKLOAD=E
make WORKLOAD=F
make WORKLOAD=G
make WORKLOAD=H

echo "Executando data_generator"
./data_generator.D.exe
./data_generator.E.exe
./data_generator.F.exe
./data_generator.G.exe
./data_generator.H.exe

echo "Obtendo resultados"
echo "NUMERO PROCESSOS: 8 WORKLOAD: D " >> $TEMPFILE
{ time mpirun -np 8 --oversubscribe ./k_means.D.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 16 WORKLOAD: D " >> $TEMPFILE
{ time mpirun -np 16 --oversubscribe ./k_means.D.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 32 WORKLOAD: D " >> $TEMPFILE
{ time mpirun -np 32 --oversubscribe ./k_means.D.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 64 WORKLOAD: D " >> $TEMPFILE
{ time mpirun -np 64 --oversubscribe ./k_means.D.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 72 WORKLOAD: D " >> $TEMPFILE
{ time mpirun -np 72 --oversubscribe ./k_means.D.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 96 WORKLOAD: D " >> $TEMPFILE
{ time mpirun -np 96 --oversubscribe ./k_means.D.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE


echo "NUMERO PROCESSOS: 8 WORKLOAD: E " >> $TEMPFILE
{ time mpirun -np 8 --oversubscribe ./k_means.E.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 16 WORKLOAD: E " >> $TEMPFILE
{ time mpirun -np 16 --oversubscribe ./k_means.E.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 32 WORKLOAD: E " >> $TEMPFILE
{ time mpirun -np 32 --oversubscribe ./k_means.E.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 64 WORKLOAD: E " >> $TEMPFILE
{ time mpirun -np 64 --oversubscribe ./k_means.E.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 72 WORKLOAD: E " >> $TEMPFILE
{ time mpirun -np 72 --oversubscribe ./k_means.E.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 96 WORKLOAD: E " >> $TEMPFILE
{ time mpirun -np 96 --oversubscribe ./k_means.E.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE


echo "NUMERO PROCESSOS: 8 WORKLOAD: F " >> $TEMPFILE
{ time mpirun -np 8 --oversubscribe ./k_means.F.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 16 WORKLOAD: F " >> $TEMPFILE
{ time mpirun -np 16 --oversubscribe ./k_means.F.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 32 WORKLOAD: F " >> $TEMPFILE
{ time mpirun -np 32 --oversubscribe ./k_means.F.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 64 WORKLOAD: F " >> $TEMPFILE
{ time mpirun -np 64 --oversubscribe ./k_means.F.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 72 WORKLOAD: F " >> $TEMPFILE
{ time mpirun -np 72 --oversubscribe ./k_means.F.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 96 WORKLOAD: F " >> $TEMPFILE
{ time mpirun -np 96 --oversubscribe ./k_means.F.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE


echo "NUMERO PROCESSOS: 8 WORKLOAD: G " >> $TEMPFILE
{ time mpirun -np 8 --oversubscribe ./k_means.G.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 16 WORKLOAD: G " >> $TEMPFILE
{ time mpirun -np 16 --oversubscribe ./k_means.G.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 32 WORKLOAD: G " >> $TEMPFILE
{ time mpirun -np 32 --oversubscribe ./k_means.G.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 64 WORKLOAD: G " >> $TEMPFILE
{ time mpirun -np 64 --oversubscribe ./k_means.G.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 72 WORKLOAD: G " >> $TEMPFILE
{ time mpirun -np 72 --oversubscribe ./k_means.G.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 96 WORKLOAD: G " >> $TEMPFILE
{ time mpirun -np 96 --oversubscribe ./k_means.G.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE

echo "NUMERO PROCESSOS: 8 WORKLOAD: H " >> $TEMPFILE
{ time mpirun -np 8 --oversubscribe ./k_means.H.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 16 WORKLOAD: H " >> $TEMPFILE
{ time mpirun -np 16 --oversubscribe ./k_means.H.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 32 WORKLOAD: H " >> $TEMPFILE
{ time mpirun -np 32 --oversubscribe ./k_means.H.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 64 WORKLOAD: H " >> $TEMPFILE
{ time mpirun -np 64 --oversubscribe ./k_means.H.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 72 WORKLOAD: H " >> $TEMPFILE
{ time mpirun -np 72 --oversubscribe ./k_means.H.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE
echo "NUMERO PROCESSOS: 96 WORKLOAD: H " >> $TEMPFILE
{ time mpirun -np 96 --oversubscribe ./k_means.H.exe file 200; } >> $RESULTFILE 2>> $TEMPFILE