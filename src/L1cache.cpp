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
  cachesize_kb_double = log2((double)cachesize_kb);
  associativity_double = log2((double)associativity);
  blocksize_bytes_double = log2((double)blocksize_bytes);   
  if(cachesize_kb_double - (int)cachesize_kb_double != 0 || associativity_double - (int)associativity_double != 0 || blocksize_bytes_double - (int)blocksize_bytes_double != 0)
  { return ERROR;   }

  // Calculando offset: 
  *offset_size = log2((double)blocksize_bytes);

  // Calculando idx_size: 
  *idx_size = log2((double) ((cachesize_kb / blocksize_bytes ) / associativity) );

  // Calculando idx_size:
  *tag_size = ADDRSIZE - *idx_size - *offset_size;

  // si el index size es menor que 0, error
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
                             entry* cache_blocks,
                             operation_result* result,
                             bool debug)
{
   return ERROR;
}
