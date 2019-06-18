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

////////////////////////////////////// ESTO NO SE USA //////////////////////////////////////////////////////////////


/* 
 * Crea el victim cache y lo inicializa
 * Return: puntero al arreglo del victim cache
 */
entry* creando_victim_cache();


/*
 * Une tag e index para meter el dato en el VC  
 * [in] tag_size: Tamano del tag de L1
 * [in] idx_size: Tamano del index de L1
 * [in] idx: Index a unir
 * [in] tag: Tag a unir
 * RETURN: retorna el tag y el index unidos para
 * guardarlos en el victim cache
 */
int joining_tag_index(   int idx_size,
                         int idx,
                         int tag);


/*
 * Busca un tag junto con el index del que proviene
 * en el victim cache y retorna el resultado de la 
 * operacion (miss, hit, dirty eviction)
 * 
 * [in] tag: Etiqueta a buscar en el victim cache
 * [in] idx: Indice del que proviene el tag a buscar
 * [in] idx_size: Tamano en cantidad de bits del index
 * [in] victim_cache: victim cache
 * [out] operation_result: Indica si hubo miss o hit, 
 * ademas de si se dio un dirty eviction.
 */
int vc_searching ( int tag,
                   int idx,
                   int idx_size,
                   entry* victim_cache,
                   operation_result_vc* operation_result);


/*
 * Inserta un elemento en la primer posicion del VC
 * 
 * [in] tag: Tag a ingresar (viene de L1)
 * [in] idx: Index de donde viene el tag
 * [in] idx_size: Tamano en cantidad de bits del index
 * [in] dirty: Si el dato a ingresar esta sucio
 * [in/out] victim_cache: Victim cache a modificar
 */
int vc_insertion ( int tag,
                   int idx,
                   int idx_size,
                   bool dirty,
                   entry* victim_cache);

/*
 * Funcion que se encarga de comunicar el vc con L1 y
 * retornar lo ocurrido entre ambas caches, para registrar
 * los datos en el main.
 * 
 * [in] tag: Tag a buscar en L1 o VC
 * [in] idx: Index proveniente del tag
 * [in] idx_size: Tamano en bits del index
 * [in] associativity: Tamano de la associativity
 * [in] loadstore:  Si es un load o un store
 * [in] victim_cache: Puntero al victim cache
 * [in] cache_blocks: Puntero al set de la cache
 * [out] operation_result_vc: Resultados obtenidos del VC
 * [out] operation_result_l1: Resultados obtenidos de L1
 * [out] misses: cantidad de miss totales
 * [out] hits: cantidad de hits totales
 * [out] VC_hits: Vc hits
 * [out] dirty_evictions: dirty evictions
 */
int comun_vc_L1( int tag,
                   int idx,
                   int idx_size,
                   int associativity,
                   bool loadstore,
                   entry* victim_cache,
                   entry* cache_blocks,
                   operation_result_vc* operation_result_vc,
                   operation_result* operation_result_l1,
                   int * misses,
                   int * hits,
                   int * VC_hits,
                   int * dirty_evictions);




void simulation_out( int cache_size_kb, 
                     int associativity, 
                     int block_size,  
                     int misses, 
                     int hits,
                     int dirty_evictions,
                     int victim_cache_hits,
                     int opt);
                     
////////////////////////////////////////////////////////////////////////////////////////////////////////////////




#endif
