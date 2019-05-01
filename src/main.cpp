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

  //-----------------Se leen los Parse argruments que ingresa el usuario-----------------

  int sizeCacheKB;
  int sizeBloqBytes;
  int associativity;
  int politica;       // 0 lru, 1 srrip

  string comandos[4] = {"-t", "-a", "-l", "-rp"};
  for(int i = 1; i <= 8; i+=2)
  { 
    if(argv[i] == comandos[0]){ sizeCacheKB = atoi(argv[i + 1]);  }
    if(argv[i] == comandos[1]){ associativity = atoi(argv[i + 1]);  }
    if(argv[i] == comandos[2]){ sizeBloqBytes = atoi(argv[i + 1]);  }
    if(argv[i] == comandos[3])
    {   
      string politicas[2] = {"lru","srrip"};
      if(argv[i + 1] == politicas[0]){ politica = 0; }
      else if(argv[i + 1] == politicas[1]){ politica = 1; }
           else{ politica = -1; } //Error    
    }
  }


  //-----------------Se calculan los tamanos del tag, index y offset-----------------

  int status = 0;   // Para saber si una funcion retono error
  int * tag_size = new int;       //tamano del tag
  int * index_size = new int;     //tamano del index
  int * offset_size = new int;    //tamano del offset

  // Verificando los tamanos de tag index y offset
  status = field_size_get(sizeCacheKB,associativity,sizeBloqBytes,tag_size,index_size,offset_size);
  if(status == ERROR){ cout << "\nSe presento un error en la funcion field_size_get()\n" << endl;}



  //-----------------Se crea la matriz que es la memoria cache------------------------

  int * cantidad_sets = new int;    //Cantidad de sets de la cache

  // Creando la matriz de la cache, donde las filas son los set y las columnas las vias
  entry ** cache = creando_matriz_cache(*index_size,associativity,cantidad_sets);



 //-----------------Se comienza con la lectura de los datos de entrada------------------------

  bool LS; 
  long address;
  int IC; 
  char data [8];
  int *tag = new int;
  int *index = new int;
  struct operation_result result = {};

  // miss_hit_counter[0]  = MISS_LOAD
  // miss_hit_counter[1]  = MISS_STORE
  // miss_hit_counter[2]  = HIT_LOAD
  // miss_hit_counter[3]  = HIT_STORE
  int miss_hit_counter[4] = {0,0,0,0}; //Contador de hits y miss 
  int dirty_eviction_counter = 0;
  bool valido = true;

  while (valido){
  //  -----------------Se leen los datos de una linea----------------------
    // Lee el numeral
    cin >> data;
    if(data[0] != 35){ valido = false; }
    else
    {
      // Lee si si es load o store 
      cin >> data;
      LS = atoi(data);
      // Lee la direccion
      cin >> data;
      address = strtol(data, NULL, 16);      
      // Lee los IC
      cin >> data;
      IC = atoi(data);

      // Cuenta las ciclos de las instrucciones
      IC_counter += IC;

    // -----------------Se procesan los datos de la linea----------------------

          // -----------------Se obtiene el tag y el index----------------------
      address_tag_idx_get(address, *tag_size, *index_size, *offset_size, index, tag); // REVISAR
    
          // -----------------Se ingresa en la cache segun la politica----------------------
          if(politica == 0) 
          {   
            status = lru_replacement_policy(*index, *tag, associativity, LS, cache[*index],&result, 0);
            if(status == ERROR){cout << "Se presento un error en la funcion lru_replacement_policy\n" << endl;}
          }
          else  // politica == 1
          {   
            status = srrip_replacement_policy(*index, *tag, associativity, LS, cache[*index],&result,0);
            if(status == ERROR){cout << "Se presento un error en la funcion srrip_replacement_policy\n" << endl;}
          }

    // -----------------Se procesan los resultados de result ----------------------      
      
      miss_hit_counter[result.miss_hit] += 1; // contador de si hubo hit o miss de load o store
      if(result.dirty_eviction){  dirty_eviction_counter += 1;  } // Contador de si hubo dirty eviction
    }

  }


  // -----------------Se analizan los resultados finales  ---------------------- 

      double total_miss = (double)miss_hit_counter[0]+ (double)miss_hit_counter[1];
      double total_data = total_miss + (double)miss_hit_counter[2] + (double)miss_hit_counter[3];
      double miss_rate = total_miss/total_data;
      double read_miss_rate = (double)miss_hit_counter[0]/total_data;

      //AMAT = hit_time + miss_rate*miss_penalty (hit_time = 1 ciclo / miss_penalty = 20 ciclos)

         int AMAT = 1 + miss_rate*20;

      // CPU_TIME = IC + (1+20)* Load_misses
         int CPU_time = IC_counter + (21)*miss_hit_counter[0];

  // ------------------------ Se imprimen los resultados  ---------------------- 

  simulation_out(sizeCacheKB,associativity,sizeBloqBytes,CPU_time,AMAT,miss_rate,read_miss_rate,dirty_eviction_counter,miss_hit_counter[0],miss_hit_counter[1],miss_hit_counter[2],miss_hit_counter[3]);
  /* Print cache configuration */

  /* Print Statistics */


  //--------------------------------------------Liberando memoria dinamica-------------------------------------

  // Liberando memoria del arreglo de la cache
  entry **cache_matrix = new entry*[*cantidad_sets];
  for(int i = 0; i < associativity; i++)
  {
    delete [] cache_matrix[i];
  }

  delete [] cache_matrix;

  // Liberando memoria de las demas variables
  delete tag_size, index_size, offset_size, cantidad_sets, tag, index;

   
return 0;
}
