
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
  //-----------------------Verificando que los parametros sean multiplos de dos----------------------:
  double cachesize_kb_double, associativity_double, blocksize_bytes_double;
  cachesize_kb_double = log2((double)cachesize_kb*KB);
  associativity_double = log2((double)associativity);
  blocksize_bytes_double = log2((double)blocksize_bytes);   
  if(cachesize_kb_double - (int)cachesize_kb_double != 0 || associativity_double - (int)associativity_double != 0 || blocksize_bytes_double - (int)blocksize_bytes_double != 0)
  { return ERROR;   }

  //-------------------- Verificando que ningun dato sea negativos------------------------------------
   if(cachesize_kb < 0 || associativity <= 0 || blocksize_bytes <= 0){  return ERROR;   }

  //-------------------------------- Calculando offset----------------------------------------------: 
  *offset_size = log2((double)blocksize_bytes);

  // -------------------------------Calculando idx_size---------------------------------------------: 
  *idx_size = log2((double) ((( cachesize_kb*KB ) / blocksize_bytes ) / associativity) );

  //--------------------------------- Calculando tag_size---------------------------------------------:
  *tag_size = ADDRSIZE - *idx_size - *offset_size;

  //------------------------------ Si el tag_size es menor que 0, error-----------------------------
  if(*tag_size < 0)
  {  return ERROR;  }

  //------------------------------ Si todo esta bien retorna OK -----------------------------
  return OK;  
}

void address_tag_idx_get(long address,
                        int tag_size,
                        int idx_size,
                        int offset_size,
                        int *idx,
                        int *tag)
{

   int mascara1 = 0x7FFFFFFF;
   int dir = address;
//------------------------------ Calculando el Index -----------------------------
   *idx = address << tag_size;
   *idx = *idx >> 1;
   *idx = *idx & mascara1;
   *idx = *idx >> (tag_size + offset_size - 1);
//------------------------------ Calculando el Tag -----------------------------
   *tag = dir >> 1;
   *tag = *tag & mascara1;
   *tag = *tag >> (idx_size + offset_size - 1) ;
}


///////////////////////////////////////////// NO SE USA ///////////////////////////////////////////////////////////////

// TESTEADA
int lru_replacement_policy (int idx,
                             int tag,
                             int associativity,
                             bool loadstore,
                             entry* cache_blocks,  // set de la cache
                             operation_result* result,
                             bool debug)
{
//------------------------------ Verificar si tag e index son validos -----------------------------
   if (idx < 0 || tag < 0 ) {    return ERROR;   }

//------------------------------ Verificar si asociatividad es valido -----------------------------
   double associativity_double = log2((double)associativity); 
   if(associativity_double - (int)associativity_double != 0) {  return ERROR;   } 
     

   result->evicted_block = false;      //--- se asume que no salio ningun bloque
   bool hit_o_miss = false;
//------------------------------ Verificar Si es Hit -----------------------------
   for(int i = 0; i < associativity; i++)
   {
      if(cache_blocks[i].tag == tag && cache_blocks[i].valid)
      {  //---------------------ocurrio un hit----------------------
         for(int j = 0; j < associativity; j++)
         {
            if(cache_blocks[j].rp_value < (cache_blocks[i].rp_value)){  cache_blocks[j].rp_value += 1;   }  //suma 1 a la politica de remplazo
         }
         hit_o_miss = true;
         cache_blocks[i].rp_value = 0;
         result->dirty_eviction = false;
         result->evicted_address = 0; 
         if(loadstore)  
         {  //------- si es un hit store-----------
            cache_blocks[i].dirty = true; 
            result->miss_hit = HIT_STORE; 
         }
         else           
         {  //------- si es un hit load------------
            result->miss_hit = HIT_LOAD;   
         }
         //------------Terminando el for-----------
         i = associativity;
      }
   }
//------------------------------ Se encontro que es un miss -----------------------------
   if(!hit_o_miss){
      for(int i = 0; i < associativity; i++)
      {
         if(cache_blocks[i].rp_value == (associativity - 1))   
         {  //----------------si es el bloque del set con menos prioridad---------------------------
            if (cache_blocks[i].valid == 1)  //-- si el bloque con asosiatividad - 1 es valido, quiere decir
            {                                //-- que va a salir ese bloque
               result->evicted_block = true;
            }
            
            result->evicted_address = (cache_blocks[i].valid)? cache_blocks[i].tag: 0 ; 
            
            cache_blocks[i].valid = 1;                      //---- Es valido ya que se va a escribir sobre el----
            cache_blocks[i].tag = tag;                      //-----tag nuevo guardado en el set----------
            result->dirty_eviction = (cache_blocks[i].dirty)? true: false;   //----Si hubo dirty eviction-----
            if(loadstore)  
            {  // -----------si hubo miss store----------------
               cache_blocks[i].dirty = true;   
               result->miss_hit = MISS_STORE;    
            }
            else           
            {  // --------------si hubo miss load---------------
               cache_blocks[i].dirty = false;   
               result->miss_hit = MISS_LOAD;    
            }
            for(int j = 0; j < associativity; j++)
            {  //----------suma 1 a los valores de remplazo correspondientes ----------------
               if(cache_blocks[j].rp_value < (associativity - 1))
               {  
                  cache_blocks[j].rp_value += 1;   
               }  
            }
            cache_blocks[i].rp_value = 0; //---- el dato que se ingreso/guardo con remplazo en 0-----
            i = associativity;            // ---- Termina el for ----
         } 
      }
   }


   return OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int lru_L1_L2_replacement_policy (int idx,
                           int tag,
                           int idxL2,
                           int tagL2,
                           int associativity,
                           bool loadstore,
                           entry* cache_blocks,
                           entry* cache_blocksL2,                           
                           operation_result_L2* operation_result_L2,
                           bool debug)
{
//------------------------------ Verificar si tag e index son validos -----------------------------
   if (idx < 0 || tag < 0 || tagL2 < 0 || idxL2 < 0) {    return ERROR;   }

//------------------------------ Verificar si asociatividad es valido -----------------------------
   double associativity_double = log2((double)associativity); 
   if(associativity_double - (int)associativity_double != 0) {  return ERROR;   } 
     
// -------------------- Creando asociatividad para L2 y otras variables del sistema ----------------
   int associativityL2 = associativity * 2;

   bool hit_o_missL1 = false;
   bool hit_o_missL2 = false;
//------------------------------ Verificar Si es Hit en L1-----------------------------
   for(int i = 0; i < associativity; i++)
   {
      if(cache_blocks[i].tag == tag && cache_blocks[i].valid)
      {  //---------------------ocurrio un hit en L1 ----------------------
         for(int j = 0; j < associativity; j++)
         {
            if(cache_blocks[j].rp_value < (cache_blocks[i].rp_value)){  cache_blocks[j].rp_value += 1;   }  //suma 1 a la politica de remplazo
         }
         hit_o_missL1 = true;                   // Declara al sistema que hubo hit en L1
         cache_blocks[i].rp_value = 0;          // Le asigna valor de remplazo 0.
         operation_result_L2->HitL1 +=1; 
         operation_result_L2->evicted_addressL1 = 0; // no sale nada de L1 por que es un hit.
         
         if(loadstore){  
            //------- si es un hit store-----------
            //--------- Busca el tag en L2 para ponerlo sucio-------
            for(int a = 0; a < associativityL2; a++){
               if(cache_blocksL2[a].tag == tagL2){
                  cache_blocksL2[a].dirty = true;
                  a = associativityL2;
               }
            }
         }
         // -----------Actualizando el valor de reemplazo en L2
            for(int i = 0; i < associativityL2; i++){
               if(cache_blocksL2[i].tag == tagL2 && cache_blocksL2[i].valid){  
                  for(int j = 0; j < associativityL2; j++){
                     if(cache_blocksL2[j].rp_value < (cache_blocksL2[i].rp_value)){  cache_blocksL2[j].rp_value += 1;   }  //suma 1 a la politica de remplazo
                  }                           
                  cache_blocksL2[i].rp_value = 0;  
                  i = associativityL2;     
               }
            }
         //------------Terminando el for-----------
         i = associativity;
      }
   }
//------------------------------ Se encontro que es un miss en L1 -----------------------------
   if(!hit_o_missL1){
      operation_result_L2->MissL1 +=1;

      //------------------------Busca en L2----------------------------------------------
      for(int i = 0; i < associativityL2; i++){
         if(cache_blocksL2[i].tag == tagL2 && cache_blocksL2[i].valid){  
            
            //---------------------ocurrio un hit en L2 ----------------------
            for(int j = 0; j < associativityL2; j++){
               if(cache_blocksL2[j].rp_value < (cache_blocksL2[i].rp_value)){  cache_blocksL2[j].rp_value += 1;   }  //suma 1 a la politica de remplazo
            }
            hit_o_missL2 = true;                  // Indica al sistema que hubo un hit en L2
            cache_blocksL2[i].rp_value = 0;       // Asiga un cero al valor de reemplazo
            operation_result_L2->HitL2 += 1; 
            if(loadstore){  
               //------- si es un hit store pone sucio el dato en L2 -----------
               cache_blocksL2[i].dirty = true; 
            }

          //------------------------ Guarda el dato en L1-------------------------------
            for(int m = 0; m < associativity; m++){
               
               //----------------si es el bloque del set con menos prioridad---------------------------
               if(cache_blocks[m].rp_value == (associativity - 1)){  
                  operation_result_L2->evicted_addressL1 = (cache_blocks[m].valid)? cache_blocks[m].tag: 0 ; 
                  cache_blocks[m].valid = 1;                      //---- Es valido ya que se va a escribir sobre el----
                  cache_blocks[m].tag = tag;                      //-----tag nuevo guardado en el set----------

                //----------suma 1 a los valores de remplazo correspondientes ----------------
                  for(int j = 0; j < associativity; j++){  
                     if(cache_blocks[j].rp_value < (associativity - 1)){  cache_blocks[j].rp_value += 1;   }  
                  }
                  cache_blocks[m].rp_value = 0; //---- el dato que se ingreso/guardo con remplazo en 0-----
                  m = associativity;            // ---- Termina el for ----
               } 
            }

          //------------Terminando el for-----------   
            i = associativityL2;
         }
      }

      //------------------------------- Hubo un miss en L1 y en L2----------------------------------------------
      if(!hit_o_missL2){
         operation_result_L2->MissL2 +=1;

         //----------------Guarda el nuevo valor en L2 ---------------------------
         for(int i = 0; i < associativityL2; i++){

            //----------------si es el bloque del set con menos prioridad---------------------------
            if(cache_blocksL2[i].rp_value == (associativityL2 - 1)){  
           
               operation_result_L2->evicted_addressL2 = (cache_blocksL2[i].valid)? cache_blocksL2[i].tag: 0 ; 
            
               cache_blocksL2[i].valid = 1;                      //---- Es valido ya que se va a escribir sobre el----
               cache_blocksL2[i].tag = tagL2;                      //-----tag nuevo guardado en el set----------
               operation_result_L2->dirty_eviction += (cache_blocksL2[i].dirty)? 1: 0;   //----Si hubo dirty eviction-----
               
               if(loadstore){
               // -----------si hubo miss store----------------
                  cache_blocksL2[i].dirty = true;      
               }
         
               //----------suma 1 a los valores de remplazo correspondientes ----------------
               for(int j = 0; j < associativityL2; j++){  
                  if(cache_blocksL2[j].rp_value < (associativityL2 - 1)){  cache_blocksL2[j].rp_value += 1;   }  
               }
               cache_blocksL2[i].rp_value = 0; //---- el dato que se ingreso/guardo con remplazo en 0-----
               i = associativityL2;            // ---- Termina el for ----
            } 
         }

         //----------------Guarda el nuevo valor en L1 ---------------------------
         for(int i = 0; i < associativity; i++){

            //----------------si es el bloque del set con menos prioridad---------------------------
            if(cache_blocks[i].rp_value == (associativity - 1)){  
               operation_result_L2->evicted_addressL1 = (cache_blocks[i].valid)? cache_blocks[i].tag: 0 ; 
               cache_blocks[i].valid = 1;                      //---- Es valido ya que se va a escribir sobre el----
               cache_blocks[i].tag = tag;                      //-----tag nuevo guardado en el set----------
               
               //----------suma 1 a los valores de remplazo correspondientes ----------------
               for(int j = 0; j < associativity; j++){  
                  if(cache_blocks[j].rp_value < (associativity - 1)){  cache_blocks[j].rp_value += 1; }  
               }
               cache_blocks[i].rp_value = 0; //---- el dato que se ingreso/guardo con remplazo en 0-----
               i = associativity;            // ---- Termina el for ----
            } 
         }      
      }      
   }
   return OK;
}



entry** creando_matriz_cache  (int idx_size,
                            int associativity,
                            int *cantidad_sets)
{
   //----------------Cantidad de sets = filas matriz -----------------------
   *cantidad_sets = pow(2,idx_size);  

   //-----------Creando matriz memoria dinamica de datos tipo entry---------
   entry **cache_matrix = new entry*[*cantidad_sets];
   for(int i = 0; i < *cantidad_sets; i++)
   {
      cache_matrix[i] = new entry[associativity];
   }

   //---------------------Inicializando valores de la cache-----------------
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
   //-----------------Retorna puntero de la matriz----------------------
   return cache_matrix;
}
      
void simulation_outL2( int cache_size_kb, 
                     int associativity, 
                     int block_size,  
                     int cp, 
                     operation_result_L2* L2)
   {
      double L1MR = (double)L2->MissL1/double(L2->MissL1+L2->HitL1);
      double L2MR = (double)L2->MissL2/double(L2->MissL2+L2->HitL2);
      double GMR = L1MR*L2MR;
      double OMR = double(L2->MissL1+L2->MissL2)/double(L2->MissL1+L2->HitL1);

      cout << "------------------------------------------\n";
            cout << "  Cache parameters:\n";
            cout << "------------------------------------------\n";
            cout << "  L1 Cache Size (KB): "<<"          " << cache_size_kb << "\n";
            cout << "  L2 Cache Size (KB): "<<"          " << cache_size_kb*4 << "\n";
            cout << "  Cache L1 Associativity: "<<"      " << associativity << "\n";
            cout << "  Cache L2 Associativity: "<<"      " << associativity*2 << "\n";
            cout << "  Cache Block Size (bytes):"<<"     " << block_size << "\n";
            cout << "  Coherence protocol                " << cp "\n";
            cout << "------------------------------------------\n";
            cout << "  Simulation results:\n";
            cout << "------------------------------------------\n";
            cout << "  Overall miss rate"<<"               " << OMR <<"\n";
            cout << "  CPU1 L1 miss rate:"<<"              " << L1MR <<"\n";
            cout << "  CPU2 L2 miss rate:"<<"              " << L2MR<<"\n";
            cout << "  Coherence Invalidation CPU1"<<"     " << GMR <<"\n";
            cout << "  Coherence Invalidation CPU2"<<"     " << GMR <<"\n";
            cout << "------------------------------------------\n";
   }       

 //////////////////////////////////////////// NO SE USA /////////////////////////////////////////////////////////////////               
void simulation_out( int cache_size_kb, 
                     int associativity, 
                     int block_size,  
                     int misses, 
                     int hits,
                     int dirty_evictions,
                     int victim_cache_hits,
                     int opt)
  {
      double miss_rate = (double)misses/double(misses + hits);

      switch (opt)
      {
               // Estadisticas para la Cache L1 simple
      case NONE:
            cout << "------------------------------------------\n";
            cout << "  Cache parameters:\n";
            cout << "------------------------------------------\n";
            cout << "  L1 Cache Size (KB): "<<"          " << cache_size_kb << "\n";
            cout << "  Cache L1 Associativity: "<<"      " << associativity << "\n";
            cout << "  Cache Block Size (bytes):"<<"     " << block_size << "\n";
            cout << "------------------------------------------\n";
            cout << "  Simulation results:\n";
            cout << "------------------------------------------\n";
            cout << "  Miss rate (L1):"<<"            " << miss_rate <<"\n";
            cout << "  Misses (L1):"<<"               " << misses << "\n";
            cout << "  Hits (L1):"<<"                 " << hits << "\n";
            cout << "  Dirty evictions:"<<"           " << dirty_evictions << "\n";
            cout << "------------------------------------------\n";
         break;
      
               // Estadisticas para la optimizacion de Victim Cache
      case VC:
            cout << "------------------------------------------\n";
            cout << "  Cache parameters:\n";
            cout << "------------------------------------------\n";
            cout << "  L1 Cache Size (KB): "<<"          " << cache_size_kb << "\n";
            cout << "  Cache L1 Associativity: "<<"      " << associativity << "\n";
            cout << "  Cache Block Size (bytes):"<<"     " << block_size << "\n";
            cout << "------------------------------------------\n";
            cout << "  Simulation results:\n";
            cout << "------------------------------------------\n";
            cout << "  Miss rate (L1+VC):"<<"            " << miss_rate <<"\n";
            cout << "  Misses (L1+VC):"<<"               " << misses << "\n";
            cout << "  Hits (L1+VC):"<<"                 " << hits << "\n";
            cout << "  Victim cache hits:"<<"                " << victim_cache_hits << "\n";
            cout << "  Dirty evictions:"<<"           " << dirty_evictions << "\n";
            cout << "------------------------------------------\n";
         break;
      }   
    
  }

/*
 * TESTEADA 
 */
entry* creando_victim_cache()
{
   //-----------Creando victim cache---------
   entry *victim_cache = new entry[16];
   for (int i = 0; i < 16; i++)
   {
      victim_cache[i].dirty = 0;
      victim_cache[i].rp_value = 16;
      victim_cache[i].tag = 0;
      victim_cache[i].valid = 0;
   }
   
   return victim_cache;
}


/*
 * TESTEADA
 */
int joining_tag_index(   int idx_size,
                         int idx,
                         int tag)
{
   int tag_vc = tag << idx_size;
   tag_vc = tag_vc + idx;  
   return tag_vc;
}

// TESTEADA
int vc_searching ( int tag,
                   int idx,
                   int idx_size,
                   entry* victim_cache,
                   operation_result_vc* operation_result)
{
   tag = joining_tag_index(idx_size,idx,tag);                        //------ Uniendo tag e index
   for (int i = 0; i < 16; i++)                                      //------ Recorriendo todo el VC
   {  //------ HIT
      if (victim_cache[i].tag == tag && victim_cache[i].valid == 1)  //------ Si se encontro el tag en la VC
      {                                                              //------ y es valido
         operation_result->miss_hit = HIT;
         operation_result->dirty_eviction = (victim_cache[i].dirty == 1)? true:false;  //--- Si el sucio es 1, dirty eviction
         operation_result->evicted_tag = victim_cache[i].tag;        //----- tag expulsado
         for (int j = i; j > 0; j--)            // -- moviendo los elementos del cache una posicion adelante
         {                                      //--- con el fin de ingresar luego el primer elemento en la pos 0
            victim_cache[j] = victim_cache[j-1];   
         }
         return OK;                             //------ Termina la funcion
      }
   }
   //------ MISS
   operation_result->miss_hit = MISS;        //------ Retorna miss
   operation_result->dirty_eviction = false; //------ Este valor podria cambiar
   for(int i = 15; i >= 0; i--)              //------ Recorriendo el VC hacia atras
   {
      if (victim_cache[i].valid == 1)        //------ Buscando el primer dato valido
      {
         if (i == 15)                        //------ Si el primer dato valido es el ultimo del VC
         {
            operation_result->dirty_eviction = (victim_cache[i].dirty == 1)? true:false;  //--- Si el sucio es 1, dirty eviction
            operation_result->evicted_tag = victim_cache[i].tag;     //----- tag expulsado 
            for (int j = i; j > 0; j--)         // -- moviendo los datos hacia adelante
            {                                   // -- desde el primer dato valido
               victim_cache[j] = victim_cache[j-1];
            } 
         }
         else
         {
            for (int j = i + 1; j > 0; j--)         // -- moviendo los datos hacia adelante
            {                                      // -- desde el primer dato valido
               victim_cache[j] = victim_cache[j-1];
            } 
         }
         i = -1;  // saliendo del for principal  
      } 
   }
   return OK;
}


// TESTEADA
int vc_insertion ( int tag,
                   int idx,
                   int idx_size,
                   bool dirty,
                   entry* victim_cache)
{
   victim_cache[0].tag = joining_tag_index(idx_size,idx,tag); //------ Uniendo tag e index
   victim_cache[0].dirty = dirty;                             //------ si hay bit de sucio
   victim_cache[0].valid = 1;                                 //------ Dato valido
   return OK;
}



// TESTEADA y poco bonita
int comun_vc_L1(   int tag,
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
                   int * dirty_evictions)
{

   lru_replacement_policy(idx,tag,associativity,loadstore,cache_blocks,operation_result_l1);

   // si fue miss L1
   if (operation_result_l1->miss_hit == MISS_LOAD || operation_result_l1->miss_hit == MISS_STORE )
   {
      if (!operation_result_l1->evicted_block)  // si no salio ningun bloque
      {
        *misses += 2;   // el miss de L1 y VC
      }
      else
      {
         *misses += 1;   // el miss de L1
      }
   }
   // si fue hit L1  
   if (operation_result_l1->miss_hit == HIT_LOAD || operation_result_l1->miss_hit == HIT_STORE)
   {
     *hits += 1;   // hit de L1
   }
   // si se saco un bloque de L1 porque el set estaba lleno y hubo miss en L1:
   if (operation_result_l1->evicted_block)   // si se saco algo del cache es la unica forma que pueda existir 
   {                                          // un dato en VC 
      //Buscando el dato en VC:
      vc_searching(tag,idx,idx_size,victim_cache,operation_result_vc);  

      // ingresando el dato que salio de L1:
      vc_insertion(operation_result_l1->evicted_address,idx,idx_size,operation_result_l1->dirty_eviction,victim_cache);

      // si fue miss en VC
      if (operation_result_vc->miss_hit == MISS)
      {
         *misses += 1;
      }
      // si fue hit en VC
      if (operation_result_vc->miss_hit == HIT)
      {
         *VC_hits += 1;
         *hits += 1;
      }
      // si hubo dirty eviction en VC
      if (operation_result_vc->dirty_eviction)
      {
         if (operation_result_vc->miss_hit == MISS)   // si fue miss en el VC
         {
            *dirty_evictions += 1;
         }
         if (operation_result_vc->miss_hit == HIT & operation_result_l1->miss_hit == MISS_LOAD) // si fue hit en el VC
         {
            for (int i = 0; i < associativity; i++)
            {
               // le agrega al bloque ingresado del VC el bit de sucio
               if(cache_blocks[i].rp_value == 0 & cache_blocks[i].valid == 1) { cache_blocks[i].dirty = true; } 
            }              
         }         
      }
      else // si no hubo dirty eviction en VC
      {
         if (operation_result_vc->miss_hit == HIT & operation_result_l1->miss_hit == MISS_LOAD) // si fue hit en el VC
         {
            for (int i = 0; i < associativity; i++)
            {
               // le agrega al bloque ingresado del VC que no es sucio
               if(cache_blocks[i].rp_value == 0 & cache_blocks[i].valid == 1) { cache_blocks[i].dirty = false; } 
            } 
         }  
      }
   }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

