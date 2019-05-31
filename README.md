# Proyecto I IE521 Simulador de Caché - Yeison Rodriguez B56074 - Pablo Vargas B57564

Este proyecto consiste en un simulador de memoria caché L1 con Victim caché y 
una L1 con una L2. Las direcciones son de 32 bits y todo se encuentra parametrizado
por lo que el usuario decide los valores de la caché

Esta caché solo utiliza política de remplazo LRU

Para las pruebas se usa el framework gtest para los test, más adelante se indicará la 
manera de ejecutarlos.

## Formato de los datos de entrada de la cache
Este es el formato de entrada para los datos que recibe la cache:
```
# LS Dir_Hex IC
```
LS: Es un booleano que es 0 en caso de un load y 1 para store

Dir_Hex: Dirección en Hexadecimal

IC: Es el número de instrucciones que se ejecutaron entre la referencia a memoria 
anterior y la actual

Ejemplo de entrada:
```
# 1 30003770 2
```
Lo cual indicaria que es un store, con la dirección hexadecimal de 8 bits 30003770 y
se ejecutaron 2 instrucciones entre la referencia anterior y la actual


## Guía para construir el proyecto
Se debe crear el directorio build, posicionarse en el mismo y ejecutar cmake y seguidamente make
como se presenta a continuación:
```
>> mkdir build
>> cd build
>> cmake ..
>> make <target> (l1cache or cachetest)
```

## Guía para ejecutar los Test
Diríjase a la carpeta build y ejecute make cachetest, existen muchas maneras de correr los test.

1. Correr todos los test:
```
  ./test/cachetest
```
2. Correr solo un test:
```
  ./test/cachetest  --gtest_filter=<test_name>
  Ex: ./test/cachetest  --gtest_filter=VC.miss_hit
```
3. Correr un test n veces:
```
./test/cachetest  --gtest_filter=<test_name> --gtest_repeat=<n>
Ex: ./test/cachetest  --gtest_filter=VC.miss_hit --gtest_repeat=2
```
4. Replicar resultados de un test en específco:
```
  Each test is run with base seed, to replicate the result the same seed must be used
  ./test/cachetest  --gtest_filter=<test_name> --gtest_random_seed=<test_seed>
  ./test/cachetest  --gtest_filter=VC.miss_hit --gtest_random_seed=2126
```  
Para habilitar el debug se debe poner esta variable en alto, es posible hacerlo desde la
terminal de la siguiente forma:
```
export TEST_DEBUG=1
```
Si se quiere deshabilitar el debug, ponga la variable en 0

## Guía para ejecutar las simulaciones de la caché
Para ejecutar el programa diríjase al directorio build/src,
```
gunzip -c <trace> | <l1cache executable> -t <cache size KB> -a <associativity> -l <block size in bytes> -opt <optimization>
```
Donde:
trace: Son los datos que entran a la caché, en el siguiente enlace se dejan trace válidos:
https://drive.google.com/drive/folders/15Z0Z0cpxmuLTrfKjuVrbsAw4fJ9tGI2s

l1cache executable: Es el ejecutable de la caché ya compilada

cache size KB: Tamaño en KB de la caché

associativity: Número de vías de la caché

block size in bytes: Tamaño del bloque en bytes

replacement policy: Políica de remplazo utilizada, únicos valores válidos son lru y srrip

A continuación se deja un ejemplo de una simulación válida:
```
gunzip -c mcf.trace.gz | ./l1cache -t 32 -a 1 -l 32 -opt L2
```

### Dependencias
Asegúrese de tener gtest instalado:
```
sudo apt-get install libgtest-dev

sudo apt-get install cmake # install cmake
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
```
