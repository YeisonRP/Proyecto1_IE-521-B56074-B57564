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

   return ERROR;
}


int lru_replacement_policy (int idx,
                             int tag,
                             int associativity,
                             bool loadstore,
                             entry* cache_blocks,  // set de la cache
                             operation_result* result,
                             bool debug)
{
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
         result->evicted_address = NULL;
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
            result->evicted_address = (cache_blocks[i].valid)? cache_blocks[i].tag: NULL ; //FIX: lo que se debe sacar es el adress, yo estoy manndando solo el tag porque no se como mandar toda la adress
            //FIX------

            cache_blocks[i].valid = 1;                      // Es valido ya que se va a escribir sobre el     
            cache_blocks[i].tag = tag;                      // tag nuevo guardado en el set
            result->dirty_eviction = (cache_blocks[i].dirty)? true: false;   //Si hubo dirty eviction
            if(loadstore)
            {
               cache_blocks[i].dirty = true;   
               result->miss_hit = MISS_STORE;    
            }
            else
            {
               cache_blocks[i].dirty = false;   
               result->miss_hit = MISS_LOAD;    
            }
            for(int j = 0; j < associativity; j++)
            {
               if(cache_blocks[j].rp_value < (associativity - 1)){  cache_blocks[j].rp_value += 1;   }  //suma 1 a la politica de remplazo
            }
            cache_blocks[i].rp_value = 0; // el dato mas recientemente usado en 0
         } 
      }
   }


   return OK;
}
