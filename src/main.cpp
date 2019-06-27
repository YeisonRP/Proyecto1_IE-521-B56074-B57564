#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <math.h>
#include <L1cache.h>
#include <debug_utilities.h>
#include <ctime>
#include <pthread.h>

/* Helper funtions */
void print_usage ()
{
  printf ("Print print_usage\n");
  exit (0);
}

using namespace std;

int main(int argc, char * argv []) {

  //---------------- Se crean las variables que miden el tiempo de ejecución ------------

  unsigned t0,t1;

  //------------------------------- Inicia el conteo ------------------------------------

  t0 = clock();

  //-----------------Se leen los Parse argruments que ingresa el usuario-----------------
 
  int sizeCacheKB;
  int sizeBloqBytes;
  int associativity;
  int cp;           // 0 VC, 1 L2, 2 NONE
  int err = 0;            //Para saber si hubo error
  string comandos[4] = {"-t", "-a", "-l", "-cp"};
  for(int i = 1; i <= 8; i+=2)
  { 
    if(argv[i] == comandos[0]){ sizeCacheKB = atoi(argv[i + 1]);  err++;  }
    if(argv[i] == comandos[1]){ associativity = atoi(argv[i + 1]);  err++;  }
    if(argv[i] == comandos[2]){ sizeBloqBytes = atoi(argv[i + 1]);  err++;  }
    if(argv[i] == comandos[3])
    {   
      string coherence[2] = {"MSI","MESI"};
      if(argv[i + 1] == coherence[0]){ cp = 0; err++; }
      else if(argv[i + 1] == coherence[1]){ cp = 1; err++;  }
            else{ cp = -1; } //Error    
    }
  }
  //Verificando que se encontraran los datos necesarios
  if(err != 4)
  {
    cout << "\nSe presento un error encontrando los Parse argruments \n" << endl;
    return 0; 
  }



  //-----------------Se calculan los tamanos del tag, index y offset-----------------

  int status = 0;   // Para saber si una funcion retono error
  int statusL2 = 0;
  int * tag_size = new int;       //tamano del tag
  int * tag_sizeL2 = new int;       //tamano del tag
  int * index_size = new int;     //tamano del index
  int * index_sizeL2 = new int;     //tamano del index
  int * offset_size = new int;    //tamano del offset


  // Verificando los tamanos de tag index y offset
  status = field_size_get(sizeCacheKB,associativity,sizeBloqBytes,tag_size,index_size,offset_size);
  
  statusL2 = field_size_get(sizeCacheKB*4,associativity*2,sizeBloqBytes,tag_sizeL2,index_sizeL2,offset_size);
  
     
  if(status == ERROR || statusL2 == ERROR)
  { 
    cout << "\nSe presento un error en la funcion field_size_get()" << endl;
    cout << "\nEsto ocurre porque el tamano de la cache, la asociatividad o el tamano del bloque no son validos\n" << endl;
    return 0;
  }



  //-----------------Se crea la matriz que es la memoria cache------------------------

  int * cantidad_sets = new int;    //Cantidad de sets de la cache

  // Creando la matriz de la cache, donde las filas son los set y las columnas las vias
  entry ** C1_L1 = creando_matriz_cache(*index_size,associativity,cantidad_sets);
  entry ** C2_L1 = creando_matriz_cache(*index_size,associativity,cantidad_sets);
  //----------Creando la matriz de la cache L2, si se elige esta optimizacion-----------
  entry ** cacheL2 = creando_matriz_cache(*index_sizeL2,associativity*2,cantidad_sets); 
                                                    // Associativity *2 porque tiene el doble de vías

  ////----------------- creando el victim cache -----------------------------------------


 //-----------------Se comienza con la lectura de los datos de entrada------------------------
  int const NUM_THREADS = 4;
  bool LS[NUM_THREADS] = { 0 }; 
  long address[NUM_THREADS] = { 0 };
  int IC; 
  char data [8];
  int tag[NUM_THREADS] = { 0 };
  int index[NUM_THREADS] = { 0 };
  struct operation_result_L2 result = { 0 };
  int tagL2[NUM_THREADS] = { 0 };
  int indexL2[NUM_THREADS] = { 0 };

  datos_de_funcion **datos_funcion = new datos_de_funcion* [NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++)
  {
    datos_funcion[i] = new datos_de_funcion;
  }
  void **void_pointer = new void*[NUM_THREADS];
  
  pthread_t threads[NUM_THREADS];

  bool valido = true;
  int IC_counter = 0;
  int access_counter = 0;
  int access_number = 0;

  int stop = 0;
  bool multi_threading;



  // int INST_COUNTER = 0;  // Contador de instruccion


///////////////////////////////////////////////////////////////////////////////

int contador = 0;
  while (valido){

    
  //  -----------------Se leen los datos de una linea----------------------
    // Lee el numeral
    for (int i = 0 ; i < NUM_THREADS ; i++)
    {
      cin >> data;
 
      if(data[0] != 35)   // si no es un # se acaba la simulacion
      {  
        valido = false; 
        stop = i; 
        i = NUM_THREADS; 
      }  
      else 
      {
        // Lee si si es load o store 
        cin >> data;
        
        LS[i] = atoi(data);
        // Lee la direccion
        cin >> data;
        
        address[i] = strtol(data, NULL, 16);      
        // Lee los IC
        cin >> data;
        
        IC = atoi(data);

        // Cuenta las ciclos de las instrucciones
        IC_counter += IC;
        stop = i + 1;
      }
    }
    for (int i = 0; i < stop; i++)
    { 
     
     
      // -----------------Se procesan los datos de la linea----------------------

          // -----------------Se obtiene el tag y el index para L1----------------------
      address_tag_idx_get(address[i], *tag_size, *index_size, *offset_size, &(index[i]), &(tag[i])); 
    
          // -----------------Se obtiene el tag y el index para L2----------------------
      address_tag_idx_get(address[i], *tag_sizeL2, *index_sizeL2, *offset_size, &(indexL2[i]), &(tagL2[i])); 

      


      
      datos_funcion[i]->idx = index[i];
      datos_funcion[i]->tag = tag[i];
      datos_funcion[i]->associativity = associativity; 
      datos_funcion[i]->loadstore = LS[i];
      datos_funcion[i]->cache_blocksL2 = cacheL2[ indexL2[i]]; 
      datos_funcion[i]->operation_result_L2_ = &result;
      datos_funcion[i]->cache_blocks = C2_L1[index[i]]; //
      datos_funcion[i]->core = 1; //
      datos_funcion[i]->cp = cp;
      datos_funcion[i]->debug = false;
      datos_funcion[i]->idxL2 = indexL2[i];
      datos_funcion[i]->tagL2 = tagL2[i];
      datos_funcion[i]->Other_L1_Core = C1_L1[index[i]];//
      if (i == 0)
      {
        datos_funcion[i]->core = 0; //
        datos_funcion[i]->Other_L1_Core = C2_L1[index[i]];//
        datos_funcion[i]->cache_blocks = C1_L1[index[i]]; //
      }
      void_pointer[i] = (void*)(datos_funcion[i]); // casting
 
   /*   cout << index[i] << endl;
      cout << tag[i] << endl;
      cout << indexL2[i] << endl;
      cout << tagL2[i] << endl; */

    lru_L1_L2_replacement_policy(void_pointer[i]);

    }


    
    // Revisando que todos sean sets distintos
    multi_threading = true;
    for (int i = 0; i < stop; i++)
    {
      for (int j = 0; j < stop; j++)
      {
        if(j != i) {
          if (index[i] == index[j] || indexL2[i] == indexL2[j])
          {
            j = stop;
            i = stop;
            multi_threading = false;
          }
        }
      }
    }


    // Si todos los sets son independientes
    multi_threading = false;
    if (multi_threading)
    {
/*
      for (long int i = 0 ; i < NUM_THREADS ; ++i)
      {

        int t;
        // -----------------Se ingresa en la cache segun la optimizacion----------------------
        if(opt == 2){   
          t = pthread_create(&threads[i], NULL, lru_replacement_policy, (void*)dat_[i]);
        }


        // -----------------Se procesan los resultados de result ----------------------      
      
        miss_hit_counter[result.miss_hit] += 1; // contador de si hubo hit o miss de load o store
        if(result.dirty_eviction){  dirty_eviction_counter += 1;  } // Contador de si hubo dirty eviction

          
  
          if (t != 0)
          {
              cout << "Error in thread creation: " << t << endl;
          }
      }

        for(int i = 0 ; i < NUM_THREADS; ++i)
        {
            void* status;
            int t = pthread_join(threads[i], &status);
            if (t != 0)
            {
                cout << "Error in thread join: " << t << endl;
            }
        }  */
    }
  }





  simulation_outL2(sizeCacheKB,associativity,sizeBloqBytes,cp, &result);




//////////////////////////////////////////////////////////////////////////////

  // ------------------------ Se imprimen los resultados  ---------------------- 

    
    
  cout << "CP" << cp << endl;
  //--------------------------------------------Liberando memoria dinamica-------------------------------------

  // Liberando memoria del arreglo de la cache

  delete[] C1_L1;
  delete[] C2_L1;
  delete[] cacheL2;

   
  for (int i = 0; i < NUM_THREADS; i++)
  {
    delete datos_funcion[i];
  }
  delete[] datos_funcion;
  // Liberando memoria de las demas variables
  delete tag_size, index_size, offset_size, cantidad_sets, tag, index;

   //--------------------- Termina el conteo de tiempo y se calcula el tiempo de ejecucion---------------------

    t1 = clock();
    double exe_time = (double(t1-t0)/CLOCKS_PER_SEC);
    cout << "Tiempo de ejecución : "<< exe_time << " segundos." << endl;
 
  return 0;
}
