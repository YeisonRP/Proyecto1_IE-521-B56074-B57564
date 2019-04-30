/*
 *  Cache simulation project
 *  Class UCR IE-521
 *  Semester: I-2019
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <math.h>
#include <debug_utilities.h>
#include <L1cache.h>

#define KB 1024
#define ADDRSIZE 32
using namespace std;
int field_size_get( int cachesize_kb,
                    int associativity,
                    int blocksize_bytes,
                    int *tag_size,
                    int *idx_size,
                    int *offset_size)
{
  //Verificando que los parametros sean multiplos de dos:
  double cachesize_kb_double, associativity_double, blocksize_bytes_double;
  cachesize_kb_double = log2((double)cachesize_kb*KB);
  associativity_double = log2((double)associativity);
  blocksize_bytes_double = log2((double)blocksize_bytes);   
  if(cachesize_kb_double - (int)cachesize_kb_double != 0 || associativity_double - (int)associativity_double != 0 || blocksize_bytes_double - (int)blocksize_bytes_double != 0)
  { return ERROR;   }

  // Calculando offset: 
  *offset_size = log2((double)blocksize_bytes);

  // Calculando idx_size: 
  *idx_size = log2((double) ((( cachesize_kb*KB ) / blocksize_bytes ) / associativity) );

  // Calculando tag_size:
  *tag_size = ADDRSIZE - *idx_size - *offset_size;

  // Si el tag_size es menor que 0, error
  if(*tag_size < 0)
  {  return ERROR;  }

  return OK;  
}

void address_tag_idx_get(long address,
                        int tag_size,
                        int idx_size,
                        int offset_size,
                        int *idx,
                        int *tag)
{
      //INDEX 
   *idx = address << tag_size;
   *idx = *idx >> (tag_size+offset_size);
      //TAG 
   *tag = address >> (idx_size+offset_size);

}

int srrip_replacement_policy (int idx,
                             int tag,
                             int associativity,
                             bool loadstore,
                             entry* cache_blocks,
                             operation_result* result,
                             bool debug)
{

   bool replace,hit_o_miss = false;
        
    // Verificar si tag e index son validos
   if (idx < 0 || tag < 0 ) {    return ERROR;   }

   // Verificar si associativity es valido
   double associativity_double = log2((double)associativity); 
   if(associativity_double - (int)associativity_double != 0) {  return ERROR;   } 

   for(int j=0; j < associativity; j++)
   {
      if(cache_blocks[j].tag == tag && cache_blocks[j].valid == true)
      {
          // HAY HIT
               
         result->dirty_eviction = false;
         cache_blocks[j].rp_value = 0;  
         hit_o_miss = true;

         if(loadstore)  // si es un hit store
         {
            cache_blocks[j].dirty = true; 
            result->miss_hit = HIT_STORE; 
         }
         else           // si es un hit load
         {
            result->miss_hit = HIT_LOAD;  
         }
               
         j = associativity; // Sale del bucle.
      }
   }
       // HAY MISS
      if(!hit_o_miss)
         {        
            replace = false;
            while(!replace)
            {
               for(int m = 0; m < associativity; m++)
               {
                  if(cache_blocks[m].rp_value == (associativity-1))
                  {
                     result->evicted_address = (cache_blocks[m].valid)? cache_blocks[m].tag: 0 ; // Linea que me pidió
                     
                     cache_blocks[m].valid = true; // Lo hace valido.
                     cache_blocks[m].tag = tag;   // Guarda el nuevo tag.

                     result->dirty_eviction = (cache_blocks[m].dirty)? true: false;   //Si hubo dirty eviction
                  
                     if(loadstore) // hay un miss store 
                     {
                        cache_blocks[m].dirty = true;   
                        result->miss_hit = MISS_STORE;    
                     }
                     else        // hay un miss load
                     {
                        cache_blocks[m].dirty = false;   
                        result->miss_hit = MISS_LOAD;    
                     }
                        
                     cache_blocks[m].rp_value = (associativity <= 2) ? 0:2; // Si la asociatividad es <= 2, el valor de remplazo es 0, de lo contrario será 2.   
                     m = associativity; // Para salir del bucle.
                     replace = true;
                  }    
               }
                  // Si ninguno tiene valor de reemplazo = (asociatividad-1) se procede a aumentar los valores.
               if(!replace)
               {
                  for(int g = 0; g < associativity; g++)
                     {
                        if(cache_blocks[g].rp_value < (associativity - 1)){  cache_blocks[g].rp_value += 1;   }  //suma 1 a la politica de remplazo
                     }
               }  
            }
         }
   return OK;
}
        




int lru_replacement_policy (int idx,
                             int tag,
                             int associativity,
                             bool loadstore,
                             entry* cache_blocks,  // set de la cache
                             operation_result* result,
                             bool debug)
{
   // Verificar si tag e index son validos
   if (idx < 0 || tag < 0 ) {    return ERROR;   }

   // Verificar si associativity es valido
   double associativity_double = log2((double)associativity); 
   if(associativity_double - (int)associativity_double != 0) {  return ERROR;   } 
     

   //int adress = (tag << associativity) + idx;   // Formato: Tag + Index
   bool hit_o_miss = false;
   // si es hit ESTADO: TERMINADO NO PROBADO
   for(int i = 0; i < associativity; i++)
   {
      if(cache_blocks[i].tag == tag && cache_blocks[i].valid)
      {  //ocurrio un hit de store
         for(int j = 0; j < associativity; j++)
         {
            if(cache_blocks[j].rp_value < (associativity - 1)){  cache_blocks[j].rp_value += 1;   }  //suma 1 a la politica de remplazo
         }
         hit_o_miss = true;
         cache_blocks[i].rp_value = 0;
         result->dirty_eviction = false;
         result->evicted_address = 0; //OJO FIX
         if(loadstore)  // si es un hit store
         {
            cache_blocks[i].dirty = true; 
            result->miss_hit = HIT_STORE; 
         }
         else           // si es un hit load
         {
            result->miss_hit = HIT_LOAD;   //
         }
         i = associativity;
      }
   }
   
   // si es miss. ESTADO: TERMINADO NO PROBADO
   if(!hit_o_miss){
      for(int i = 0; i < associativity; i++)
      {
         if(cache_blocks[i].rp_value == (associativity - 1))   //si es el bloque del set con menos prioridad
         {  
            //FIX------
            result->evicted_address = (cache_blocks[i].valid)? cache_blocks[i].tag: 0 ; //FIX: lo que se debe sacar es el adress, yo estoy manndando solo el tag porque no se como mandar toda la adress y  cuidado con el null
            //FIX------

            cache_blocks[i].valid = 1;                      // Es valido ya que se va a escribir sobre el     
            cache_blocks[i].tag = tag;                      // tag nuevo guardado en el set
            result->dirty_eviction = (cache_blocks[i].dirty)? true: false;   //Si hubo dirty eviction
            if(loadstore)  // si hubo miss store
            {
               cache_blocks[i].dirty = true;   
               result->miss_hit = MISS_STORE;    
            }
            else           // si hubo miss load
            {
               cache_blocks[i].dirty = false;   
               result->miss_hit = MISS_LOAD;    
            }
            for(int j = 0; j < associativity; j++)
            {
               if(cache_blocks[j].rp_value < (associativity - 1)){  cache_blocks[j].rp_value += 1;   }  //suma 1 a la politica de remplazo
            }
            cache_blocks[i].rp_value = 0; // el dato mas recientemente usado en 0
            i = associativity;
         } 
      }
   }


   return OK;
}

entry** creando_matriz_cache  (int idx_size,
                            int associativity,
                            int *cantidad_sets)
{

   *cantidad_sets = ((int)log2(idx_size)) * associativity;  //Filas//

   //Creando matriz memoria dinamica de datos tipo entry
   entry **cache_matrix = new entry*[*cantidad_sets];
   for(int i = 0; i < associativity; i++)
   {
      cache_matrix[i] = new entry[associativity];
   }

   // Inicializando valores de la cache   
   for(int i = 0; i < *cantidad_sets; i++)
   {
      for(int j = 0; j < associativity; j++)
      {
         cache_matrix[i][j].valid = 0;
         cache_matrix[i][j].dirty = 0;
         cache_matrix[i][j].rp_value = associativity - 1;
         cache_matrix[i][j].tag = 0;
      } 
   }
   return cache_matrix;
}

void simulation_out( int cache_size_kb, 
                     int associativity, 
                     int block_size, 
                     int CPU_time, 
                     int AMAT, 
                     double miss_rate, 
                     double Read_miss_rate, 
                     int Dirty, 
                     int Load_miss,
                     int Store_miss,
                     int Load_hit,
                     int Store_hit
                       )
  {
      
    cout << "------------------------------------------\n";
    cout << "  Cache parameters:\n";
    cout << "------------------------------------------\n";
    cout << "  Cache Size (KB): "<<"          " << cache_size_kb << "\n";
    cout << "  Cache Associativity: "<<"      " << associativity << "\n";
    cout << "  Cache Block Size (bytes):"<<"  " << block_size << "\n";
    cout << "------------------------------------------\n";
    cout << "  Simulation results:\n";
    cout << "------------------------------------------\n";
    cout << "  CPU time (cycles):"<<"         "<< CPU_time <<"\n";
    cout << "  AMAT(cycles):"<<"              "<< AMAT << "\n";
    cout << "  Overall: miss rate"<<"         " << miss_rate <<"\n";
    cout << "  Read miss rate:"<<"            " << Read_miss_rate <<"\n";
    cout << "  Dirty evictions:"<<"           " << Dirty << "\n";
    cout << "  Load misses:"<<"               " << Load_miss << "\n";
    cout << "  Store misses:"<<"              " << Store_miss << "\n";
    cout << "  Load hits:"<<"                 " << Load_hit << "\n";
    cout << "  Store hits:"<<"                " << Store_hit << "\n";
    cout << "  Total hits:"<<"                " << (Load_hit + Store_hit) << "\n";
    cout << "------------------------------------------\n";
  }