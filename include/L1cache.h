/*
 *  Cache simulation project
 *  Class UCR IE-521
 *  Semester: I-2019
*/

#ifndef L1CACHE_H
#define L1CACHE_H

#include <netinet/in.h> 
/* 
 * ENUMERATIONS 
 */

/* Return Values */
enum returns_types {
 OK,
 PARAM,
 ERROR
};

/* Type of optimization */
enum replacement_policy{
  VC,
  L2,
  NONE
};


enum miss_hit_status {
 MISS_LOAD,
 MISS_STORE,
 HIT_LOAD,
 HIT_STORE
};

enum coherence {            // Estados de coherencia
  INVALID,
  SHARED,
  MODIFIED,
  EXCLUSIVE,
  NONE_
};

/*
 * STRUCTS
 */

/* Cache tag array fields */
struct entry {
 enum coherence state;          // Estado de coherencia en el bloque 
 bool valid ;
 bool dirty ;
 int tag ;
 uint8_t rp_value ;
};

/* Cache replacement policy results */
struct operation_result {
 enum miss_hit_status miss_hit;
 bool dirty_eviction;
 int  evicted_address;
 bool evicted_block;    //Para saber si salio un bloque
};


/* Cache L2 replacement policy results */
struct operation_result_L2 {
 int MissL1;
 int HitL1;
 int MissL2;
 int HitL2;
 int dirty_eviction;
 int  evicted_addressL1;
 int  evicted_addressL2;
};
//////////////////////////////////////////////////


//////////////////////////////////////////////////

/* 
 *  Functions
 * /

/*
 * Get tag, index and offset length
 * 
 * [in] cache_size: total size of the cache in Kbytes
 * [in] associativity: number of ways of the cache
 * [in] blocksize_bytes: size of each cache block in bytes
 *
 * [out] tag_size: size in bits of the tag field
 * [out] idx_size: size in bits of the index field
 * [out] offset_size: size in bits of the offset size
 */
int field_size_get(int cachesize_kb,
                   int associativity,
                   int blocksize_bytes,
                   int *tag_size,
                   int *idx_size,
                   int *offset_size);

/* 
 * Get tag and index from address
 * 
 * [in] address: memory address
 * [in] tag_size: number of bits of the tag field
 * [in] idx_size: number of bits of the index field
 *
 * [out] idx: cache line idx
 * [out] tag: cache line tag
 */

void address_tag_idx_get(long address,
                         int tag_size,
                         int idx_size,
                         int offset_size,
                         int *idx,
                         int *tag);


/* 
 * Search for an address in a cache set and
 * replaces blocks using LRU policy
 * 
 * [in] idx: index field of the block
 * [in] tag: tag field of the block
 * [in] associativity: number of ways of the entry
 * [in] opt: tipo de optimizacion con la que debe trabajar la politica
 * [in] loadstore: type of operation true if store false if load
 * [in] debug: if set to one debug information is printed
 *
 * [in/out] cache_block: return the cache operation return (miss_hit_status)
 * [out] result: result of the operation (returns_types)
 */
int lru_replacement_policy (int idx,
                           int tag,
                           int associativity,
                           bool loadstore,
                           entry* cache_blocks,
                           operation_result* operation_result,
                           bool debug=false);



/* 
 * Search for an address in a cache set and
 * replaces blocks using LRU policy
 * 
 * [in] idx: index field of the block
 * [in] tag: tag field of the block
 * [in] associativity: number of ways of the entry
 * [in] opt: tipo de optimizacion con la que debe trabajar la politica
 * [in] loadstore: type of operation true if store false if load
 * [in] debug: if set to one debug information is printed
 *
 * [in/out] cache_block: return the cache operation return (miss_hit_status)
 * [out] result: result of the operation (returns_types)
 */
int lru_L1_L2_replacement_policy (int idx,
                           int tag,
                           int idxL2,
                           int tagL2,
                           int associativity,
                           bool loadstore,
                           entry* cache_blocks,
                           entry* cache_blocksL2,                           
                           operation_result_L2* operation_result_L2,
                           bool debug=false);


/* 
 * Crea una matriz que representa la memoria cache
 * donde las filas son los sets, y las columnas son
 * las vias.
 * 
 * [in] idx_size: Numero de bits del indice
 * [in] associativity: Numero de vias de la cache
 *
 * [out] cantidad_sets: Cantidad de sets en la cache
 */
entry** creando_matriz_cache  (int idx_size,
                            int associativity,
                            int *cantidad_sets);





void simulation_outL2( int cache_size_kb, 
                       int associativity,  
                       int block_size,   
                       operation_result_L2* L2
                       );


/* 
 * Search for the coherence_state of a specific
 * block in a cache set
 * 
 * [in] tag: tag field of the block
 * [in] associativity: number of ways of the entry
 * [in] cache_block: cache block
 * [out] return: return the coherence state of the data (INVALID,SHARED,MODIFIED,EXCLUSIVE),
 *               if the tag is not in the block, return NONE_
 */
coherence get_coherence_state ( int tag,
                                int associativity,
                                entry* cache_blocks);


/* 
 * Set the coherence_state of a specific
 * block
 * 
 * [in] tag: tag field of the block
 * [in] associativity: number of ways of the entry
 * [in] cache_block: cache block
 * [in] coherence_state: new coherence_state of the data (INVALID,SHARED,MODIFIED,EXCLUSIVE)
 */
void set_coherence_state (int tag,
                          int associativity,
                          entry* cache_blocks,
                          coherence coherence_state);


#endif


/*

PROTOCOLO MESI para un bloque llamado  bloque1 (que esta por ejemplo en el set 3 way 2) en el L1 del CORE 1

if(HAY HIT DEL bloque1 EN L1)
{ 
  if(bloque1 tiene estado I) 
  {

    PREGUNTAS. 
	Si estaba invalido el bloque y pasa a exclusivo, significa que
    	se tuvo que traer el dato de MM. En este caso seria un hit de todas formas?

	Si hay miss en L1 y hit en L2, se copia el dato junto con el estado en ambos?

	Que significa lo de coherency de los resultados a imprimir?

    if(Era un read)
    {
       se debe buscar en la L1 del otro core y en L2, si esta en alguna de estas se debe pasar
       al estado S todos los bloques (excepto si estan en I). (funciones get y set? u otra)

       En caso contrario pasa a E
    }
    
    if(Era un write)
    {
       Se debe pasar al estado modificado, y verificar si esta en el L1 del otro core o L2 para ponerlo I
    }
  }

  if(bloque1 tiene estado E)
  {
    if(Era un read)
    {
        Se queda en E. Porque tiene el mismo dato que MM y ninguna otra memoria de la jerarquia tiene el dato
    }
    
    if(Era un write)
    {
        Se pasa al estado M. 
    } 
  }
  if(bloque1 tiene estado S)
  {
      if(Era un read)
      {
          Se queda en S
      }
      
      if(Era un write)
      {
          Se pasa al estado M. SE debe mandar mensaje a L2 y al otro core para que invalide el dato
      }  
  }
  if(bloque1 tiene estado M)
  {
    if(Era un read)
    {
        se queda en M
    }
    
    if(Era un write)
    {
        se queda en M
    }    
  }
}

if(HAY miss DEL bloque1 EN L1 y hit en L2)
{ 
se hace lo mismo que en el anterior solo que ahora solo se debe verificar el otro core para ver si 
tiene el dato y acomodarlo segun corresponda.

El dato de L2 y el core 1 es el mismo despues del miss del core 1 y hit de L2
}

    

 */
