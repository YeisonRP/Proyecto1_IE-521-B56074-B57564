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
  




   /*void simulation_out(int cache_size_kb, int associativity, int bloq_size)
  {
      
    cout << "---------------------------------------\n";
    cout << "  Cache parameters:\n";
    cout << "---------------------------------------\n";
    cout << "  Cache Size (KB): "<<"          " << cache_size_kb << "\n";
    cout << "  Cache Associativity: "<<"      " << associativity << "\n";
    cout << "  Cache Block Size (bytes):"<<"  " << bloq_size << "\n";
    cout << "---------------------------------------\n";
    cout << "  Simulation results:\n";
    cout << "---------------------------------------\n";
    cout << "  CPU time (cycles):"<<"         0000\n";
    cout << "  AMAT(cycles):"<<"              0000\n";
    cout << "  Overall: miss rate"<<"         0.00\n";
    cout << "  Read miss rate:"<<"            0.00\n";
    cout << "  Dirty evictions:"<<"           00000\n";
    cout << "  Load misses:"<<"               00000\n";
    cout << "  Store misses:"<<"              00000\n";
    cout << "  Load hits:"<<"                 00000\n";
    cout << "  Store hits:"<<"                00000\n";
    cout << "  Total hits:"<<"                00000\n";
    cout << "---------------------------------------\n";
  }*/
return 0;
}
