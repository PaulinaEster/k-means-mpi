# Lab2
Códigos para o Laboratório 2:
- k-means
- png-gray-histogram
- riemann-zeta

# Compilar e executar
- Compile e execute o arquivo data_generator (caso exista):
```
make data_generator WORKLOAD=A
./data_generator.A.exe
```
- Compile e execute o código da aplicação:
```
make png_gray_histogram WORKLOAD=A TIMER=ON DEBUG=ON
./png_gray_histogram.A.exe
```
- A aplicação png-gray-histogram exige a instalação da biblioteca de png do Linux:
```
sudo apt install libpng-dev
```

## Report

Neste trabalho você deve implementar a versão paralela de um dos programas acima listados e depois fazer: descrever o ambiente de testes e discutir os resultados obtidos. 

### Descrição do Ambiente de Testes

- SO:
- Kernel Linux:
- CPU:
- GPU:
- Versões de Software:
  - Compilador:
  - CUDA/OpenCL:

 ### Descrição dos Resultados

_Obs: Aqui você deve apresentar um gráfico de barras para os tempos de execução, ilustrando o tempo de execução para a versão serial e as demais barras são suas versões paralelas. Utilize *error-bars* para plotar o desvio padrão de 5 repetições. Também deve ser apresentado um gráfico de speed-up. Você pode apresentar pelo menos dois gráficos de speed-up, um em relação ao tempo total da aplicação, e outro em relação ao TIMER_COMPUTATION (é exibido ao ativar o timer da aplicação). Discuta os resultados que estão nos gráficos._
