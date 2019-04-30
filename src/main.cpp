#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <math.h>
#include <L1cache.h>
#include <debug_utilities.h>


/* Helper funtions */
void print_usage ()
{
  printf ("Print print_usage\n");
  exit (0);
}

int main(int argc, char * argv []) {
  printf("Do something :), don't forget to keep track of execution time");
  /* Parse argruments */

  /* Get trace's lines and start your simulation */

  bool LS; 
  long address;
  int IC; 
  char data [8];

  while (data != NULL){
    // Lee el numeral
    cin >> data;
    // Lee si si es load o store 
    cin >> data;
    LS = atoi(data);
    // Lee la direccion
    cin >> data;
    address = strtol(data, NULL, 16);      
    // Lee los IC
    cin >> data;
    IC = atoi(data);
  }

  /* Print cache configuration */

  /* Print Statistics */

  /*
  int cache_size_kb = 1024;
  int bloq_size = 64;
  int associativity = 4;
  entry ** cache;
  int * sets;
  creando_matriz_cache(cache_size_kb,associativity,bloq_size,sets,cache);
  */

  //  cache[i] seria un puntero simple de tipo entry que lo que representa 
  //  es un set de la cache, cache[i] seria el set de entrada que se inyecta
  //  a las funciones lru_replacement_policy o srrip_replacement_policy

  // cache[i][j] seria un dato tipo entry que representa un bloque de la cache
  // Por ejemplo:

  // cache[2][0] Esto representaria el bloque que
  // se encuentra en el set numero 2, y la via numero 0

  // Si por ejemplo se quisiera insertar un dato con el metodo srrip en el set 2
  // La entrada para el dato cache_blocks seria cache[2], ya que es un puntero sim
  // ple de tipo entry que apunta al set 2.
  




   
return 0;
}
