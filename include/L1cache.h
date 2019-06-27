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
 int Miss_L1_C1;
 int Hit_L1_C1;
 int Coherence_Inv_C1;

 int Miss_L1_C2;
 int Hit_L1_C2;
 int Coherence_Inv_C2;

 int Miss_L2;
 int Hit_L2;
 int dirty_eviction;
 int  evicted_addressL1;
 int  evicted_addressL2;
};


struct datos_de_funcion {          
int idx;
int tag;
int idxL2;
int tagL2;
int associativity;
bool loadstore;
entry* cache_blocks;
entry* Other_L1_Core;
int cp;
entry* cache_blocksL2;                           
operation_result_L2* operation_result_L2_;
bool debug;
bool core;
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
 * [in] cp: Define el protocolo de coherencia
 * [in/out] cache_block: return the cache operation return (miss_hit_status)
 * [out] result: result of the operation (returns_types)
 */
void* lru_L1_L2_replacement_policy (void* datos_funcion);



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





void simulation_outL2(  int cache_size_kb, 
                        int associativity, 
                        int block_size,  
                        int cp, 
                        operation_result_L2* L2);


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



void insert_LRU_data_in_cache_set(int idx,
                                  int tag,
                                  int associativity,
                                  bool loadstore,
                                  entry* cache_blocks,
                                  operation_result* operation_result
);


void MESI(  int idx,
            int tag,
            int idxL2,
            int tagL2,
            int associativity, 
            bool loadstore,
            bool core,
            entry* cache_blocks_L1_C1,
            entry* cache_blocks_L1_C2,
            entry* cache_blocksL2,                           
            operation_result_L2* operation_result_L2);




#endif


