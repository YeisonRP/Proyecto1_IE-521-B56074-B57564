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

using namespace std;

int main(int argc, char * argv []) {
  printf("Do something :), don't forget to keep track of execution time");
  
  /* Parse argruments */
  int sizeCacheKB;
  int sizeBloqBytes;
  int associativity;
  int politica;       // 0 lru, 1 srrip

  string comandos[4] = {"-t", "-a", "-l", "-rp"};
  for(int i = 1; i <= 8; i+=2)
  { 
    if(argv[i] == comandos[0]){ sizeCacheKB = atoi(argv[i + 1]);  }
    if(argv[i] == comandos[1]){ sizeBloqBytes = atoi(argv[i + 1]);  }
    if(argv[i] == comandos[2]){ associativity = atoi(argv[i + 1]);  }
    if(argv[i] == comandos[3])
    {   
      string politicas[2] = {"lru","srrip"};
      if(argv[i + 1] == politicas[0]){ politica = 0; }
      else if(argv[i + 1] == politicas[1]){ politica = 1; }
           else{ politica = -1; } //Error    
    }
  }

  int status = 0;   // Para saber si una funcion retono error
  int * tag_size = new int;       //tamano del tag
  int * index_size = new int;     //tamano del index
  int * offset_size = new int;    //tamano del offset

  // Verificando los tamanos de tag index y offset
  status = field_size_get(sizeCacheKB,associativity,sizeBloqBytes,tag_size,index_size,offset_size);
  if(status == ERROR){ cout << "\nSe presento un error en la funcion field_size_get()\n" << endl;}

  int * cantidad_sets = new int;    //tamano del offset
  

  // Creando la matriz de la cache, donde las filas son los set y las columnas las vias
  entry ** cache = creando_matriz_cache(*index_size,associativity,cantidad_sets);

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
  

  //--------------------------------------------Liberando memoria dinamica-------------------------------------

  // Liberando memoria del arreglo de la cache
  entry **cache_matrix = new entry*[*cantidad_sets];
  for(int i = 0; i < associativity; i++)
  {
    delete [] cache_matrix[i];
  }

  delete [] cache_matrix;

  // Liberando memoria de las demas variables
  delete tag_size, index_size, offset_size, cantidad_sets;

   
return 0;
}
