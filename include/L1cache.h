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

/* Hit or miss in victim cache  */
enum miss_hit_status_vc {
 MISS,
 HIT
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
  EXCLUSIVE
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




/* Cache replacement policy results */
struct operation_result_vc {
 enum miss_hit_status_vc miss_hit;
 bool dirty_eviction;
 int  evicted_tag;
};


void simulation_outL2( int cache_size_kb, 
                       int associativity,  
                       int block_size,   
                       operation_result_L2* L2
                       );



#endif
