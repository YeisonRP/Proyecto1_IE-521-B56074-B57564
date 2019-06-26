#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <math.h>
#include <L1cache.h>
#include <debug_utilities.h>
#include <ctime>


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
  /*
  cout << "Tag L1 "<< *tag_size<< endl;
  cout << "Tag L2 "<< *tag_sizeL2<< endl;
  cout << "Index L1 "<< *index_size<< endl;
  cout << "Index L2 "<< *index_sizeL2<< endl;*/
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

  bool LS; 
  long address;
  int IC; 
  char data [8];
  int *tag = new int;
  int *index = new int;
  struct operation_result_L2 result = {0,0,0,0,0,0,0,0,0,0,0};

  int *tagL2 = new int;
  int *indexL2 = new int;

  bool valido = true;
  int IC_counter = 0;
  int access_counter = 0;
  int access_number = 0;

  // int INST_COUNTER = 0;  // Contador de instruccion

  while (valido){
  //  -----------------Se leen los datos de una linea----------------------
    // Lee el numeral
    cin >> data;
    if(data[0] != 35){ valido = false; }  // si no es un # se acaba la simulacion
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
      access_number = access_counter % 4;
      access_counter += 1;
      //INST_COUNTER +=1;  // Aumenta la cuenta 
  
    // -----------------Se procesan los datos de la linea----------------------

          // -----------------Se obtiene el tag y el index para L1----------------------
      address_tag_idx_get(address, *tag_size, *index_size, *offset_size, index, tag); // REVISAR
    
          // -----------------Se obtiene el tag y el index para L2----------------------
      address_tag_idx_get(address, *tag_sizeL2, *index_sizeL2, *offset_size, indexL2, tagL2); // REVISAR

      // Eligiendo el CORE
      if (access_number == 0) // C1
      {
        lru_L1_L2_replacement_policy(*index,*tag,*indexL2,*tagL2,associativity,LS,C1_L1[*index],C2_L1[*index],cp,cacheL2[*indexL2],&result,false,0);
      }
      else  // C2
      {
        lru_L1_L2_replacement_policy(*index,*tag,*indexL2,*tagL2,associativity,LS,C2_L1[*index],C1_L1[*index],cp,cacheL2[*indexL2],&result,false,1);
      }
    }
  }



  // ------------------------ Se imprimen los resultados  ---------------------- 

    simulation_outL2(sizeCacheKB,associativity,sizeBloqBytes,cp, &result);
    
cout << "CP" << cp << endl;
  //--------------------------------------------Liberando memoria dinamica-------------------------------------

  // Liberando memoria del arreglo de la cache

  delete[] C1_L1;
  delete[] C2_L1;
  delete[] cacheL2;

  // Liberando memoria de las demas variables
  delete tag_size, index_size, offset_size, cantidad_sets, tag, index;

   //--------------------- Termina el conteo de tiempo y se calcula el tiempo de ejecucion---------------------

    t1 = clock();
    double exe_time = (double(t1-t0)/CLOCKS_PER_SEC);
    cout << "Tiempo de ejecución : "<< exe_time << " segundos." << endl;

  return 0;
}
