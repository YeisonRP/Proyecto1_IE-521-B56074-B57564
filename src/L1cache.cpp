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

void address_tag_vc_get(long address,
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

int srrip_replacement_policy (int idx,
                             int tag,
                             int associativity,
                             bool loadstore,
                             entry* cache_blocks,
                             operation_result* result,
                             bool debug)
{

   bool replace,hit_o_miss = false;
        
//------------------------------ Verificar Si es Hit -----------------------------
   if (idx < 0 || tag < 0 ) {    return ERROR;   }

//------------------- Verificar si associativity es valido------------------------
   double associativity_double = log2((double)associativity); 
   if(associativity_double - (int)associativity_double != 0) {  return ERROR;   } 

//-----------------------------Verificar si hay un HIT----------------------------
   for(int j=0; j < associativity; j++)
   {
	//---------------------ocurrio un hit----------------------
      if(cache_blocks[j].tag == tag && cache_blocks[j].valid == true)
      {
       //------------- Configura los valores para un hit-----------   
         result->dirty_eviction = false;
         cache_blocks[j].rp_value = 0;  
         hit_o_miss = true;
	
	//---------------------- si es un hit store----------------
         if(loadstore)  
         {
            cache_blocks[j].dirty = true; 
            result->miss_hit = HIT_STORE; 
         }
         //------------------- si es un hit load ------------------
	else           
         {
            result->miss_hit = HIT_LOAD;  
         }
               
         j = associativity; // -------Sale del bucle----------------
      }
   }
       // ----------------------------- ocurrio un miss ------------------------
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
                     
                     cache_blocks[m].valid = true; // ------Lo hace valido-------
                     cache_blocks[m].tag = tag;   // ----Guarda el nuevo tag-----

                     result->dirty_eviction = (cache_blocks[m].dirty)? true: false;   //Si hubo dirty eviction
               	
		     //------------- si hay un miss store ------------------------
                     if(loadstore) 
                     {
                        cache_blocks[m].dirty = true;   
                        result->miss_hit = MISS_STORE;    
                     }
		     //------------- si hay un miss load ------------------------
                     else        
                     {
                        cache_blocks[m].dirty = false;   
                        result->miss_hit = MISS_LOAD;    
                     }
                     //-------------- Valor para la politica de reemplazo ---------------- 
                     cache_blocks[m].rp_value = (associativity <= 2) ? 0:2; // Si la asociatividad es <= 2, el valor de remplazo es 0, de lo contrario será 2.   
                     m = associativity; //---------- Para salir del bucle------------------
                     replace = true;
                  }    
               }
                //------ Si ninguno tiene valor de reemplazo = (asociatividad-1) se procede a aumentar los valores-----
               if(!replace)
               {
		//----------------------- suma 1 a la politica de remplazo---------------------------------
                  for(int g = 0; g < associativity; g++)
                     {
                        if(cache_blocks[g].rp_value < (associativity - 1)){  cache_blocks[g].rp_value += 1;   }  
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
         
         if(loadstore){  
            //------- si es un hit store-----------
            //--------- Busca el tag en L2 para ponerlo sucio-------
            for(int a = 0; a < associativityL2; a++){
               if(cache_blocksL2[a].tag==tagL2){
                  cache_blocksL2[a].dirty = true;
                  a = associativityL2;
               }
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
                  operation_result_L2->evicted_address = (cache_blocks[m].valid)? cache_blocks[m].tag: 0 ; 
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
           
               operation_result_L2->evicted_address = (cache_blocksL2[i].valid)? cache_blocksL2[i].tag: 0 ; 
            
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
               operation_result_L2->evicted_address = (cache_blocks[i].valid)? cache_blocks[i].tag: 0 ; 
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
                     operation_result_L2* L2)
   {
      double L1MR = (double)L2->MissL1/double(L2->MissL1+L2->HitL1);
      double L2MR = (double)L2->MissL2/double(L2->MissL2+L2->HitL2);

      cout << "------------------------------------------\n";
            cout << "  Cache parameters:\n";
            cout << "------------------------------------------\n";
            cout << "  L1 Cache Size (KB): "<<"          " << cache_size_kb << "\n";
            cout << "  L2 Cache Size (KB): "<<"          " << cache_size_kb*4 << "\n";
            cout << "  Cache L1 Associativity: "<<"      " << associativity << "\n";
            cout << "  Cache L2 Associativity: "<<"      " << associativity*2 << "\n";
            cout << "  Cache Block Size (bytes):"<<"     " << block_size << "\n";
            cout << "------------------------------------------\n";
            cout << "  Simulation results:\n";
            cout << "------------------------------------------\n";
            cout << "  Overall miss rate"<<"         " << "000" <<"\n";
            cout << "  L1 miss rate:"<<"            " << L1MR <<"\n";
            cout << "  L2 miss rate:"<<"            " << L2MR<<"\n";
            cout << "  Global miss rate:"<<"            " << "000" <<"\n";
            cout << "  Misses (L1):"<<"               " << L2->MissL1 << "\n";
            cout << "  Hits (L1):"<<"                 " << L2->HitL1  << "\n";
            cout << "  Misses (L2):"<<"               " << L2->MissL2  << "\n";
            cout << "  Hits (L2):"<<"                 " << L2->HitL2  << "\n";
            cout << "  Dirty evictions (L2):"<<"           " << L2->dirty_eviction  << "\n";
            cout << "------------------------------------------\n";
   }                    


/*void simulation_out( int cache_size_kb, 
                     int associativity, 
                     int block_size, 
                     int CPU_time, 
                     int AMAT, 
                     int opt,
                     double miss_rate, 
                     double Read_miss_rate, 
                     int Dirty, 
                     int Load_miss,
                     int Store_miss,
                     int Load_hit,
                     int Store_hit
                       )
  {
      switch (opt)
      {
               // Estadisticas para la Cache L1 simple
      case NONE:
            cout << "------------------------------------------\n";
            cout << "  Cache parameters:\n";
            cout << "------------------------------------------\n";
            cout << "  L1 Cache Size (KB): "<<"          " << cache_size_kb << "\n";
            cout << "  Cache L1 Associativity: "<<"      " << associativity << "\n";
            cout << "  Cache Block Size (bytes):"<<"  " << block_size << "\n";
            cout << "------------------------------------------\n";
            cout << "  Simulation results:\n";
            cout << "------------------------------------------\n";
            cout << "  Miss rate (L1):"<<"            " << Read_miss_rate <<"\n";
            cout << "  Misses (L1):"<<"               " << Load_miss << "\n";
            cout << "  Hits (L1):"<<"                 " << Load_hit << "\n";
            cout << "  Dirty evictions:"<<"           " << Dirty << "\n";
            cout << "------------------------------------------\n";
         break;
      
               // Estadisticas para la optimizacion de Victim Cache
      case VC:
            cout << "------------------------------------------\n";
            cout << "  Cache parameters:\n";
            cout << "------------------------------------------\n";
            cout << "  L1 Cache Size (KB): "<<"          " << cache_size_kb << "\n";
            cout << "  Cache L1 Associativity: "<<"      " << associativity << "\n";
            cout << "  Cache Block Size (bytes):"<<"  " << block_size << "\n";
            cout << "------------------------------------------\n";
            cout << "  Simulation results:\n";
            cout << "------------------------------------------\n";
            cout << "  Miss rate (L1+VC):"<<"            " << Read_miss_rate <<"\n";
            cout << "  Misses (L1+VC):"<<"               " << Load_miss << "\n";
            cout << "  Hits (L1+VC):"<<"                 " << Load_hit << "\n";
            cout << "  Victim cache hits:"<<"                " << Store_hit << "\n";
            cout << "  Dirty evictions:"<<"           " << Dirty << "\n";
            cout << "------------------------------------------\n";
         break;

               // Estadisticas para la optimizacion de L2 Cache
      case L2:
            cout << "------------------------------------------\n";
            cout << "  Cache parameters:\n";
            cout << "------------------------------------------\n";
            cout << "  L1 Cache Size (KB): "<<"          " << cache_size_kb << "\n";
            cout << "  L2 Cache Size (KB): "<<"          " << cache_size_kb << "\n";
            cout << "  Cache L1 Associativity: "<<"      " << associativity << "\n";
            cout << "  Cache L2 Associativity: "<<"      " << associativity << "\n";
            cout << "  Cache Block Size (bytes):"<<"  " << block_size << "\n";
            cout << "------------------------------------------\n";
            cout << "  Simulation results:\n";
            cout << "------------------------------------------\n";
            cout << "  Overall miss rate"<<"         " << miss_rate <<"\n";
            cout << "  L1 miss rate:"<<"            " << Read_miss_rate <<"\n";
            cout << "  L2 miss rate:"<<"            " << Read_miss_rate <<"\n";
            cout << "  Global miss rate:"<<"            " << Read_miss_rate <<"\n";
            cout << "  Misses (L1):"<<"               " << Load_miss << "\n";
            cout << "  Hits (L1):"<<"                 " << Load_hit << "\n";
            cout << "  Misses (L2):"<<"               " << Load_miss << "\n";
            cout << "  Hits (L2):"<<"                 " << Load_hit << "\n";
            cout << "  Dirty evictions (L2):"<<"           " << Dirty << "\n";
            cout << "------------------------------------------\n";
         break;
      }   
    
  }*/




/*
 * TESTEADA PERO TIENE ALGO EXTRANO YA QUE NO ME 
 * DA SEG FAULT SI ME METO EN DIR DE MEMORIA NO
 * PERMITIDAS, POR LO DEMAS TODO BIEN
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
// si hay un miss en cache y no salio bloque, ni si quiera busque en el victim xq no va a estar, 
// por lo que es victim miss y no usar ninguna funcion
// si no sale bloque de la cache, directo es miss de VC sin hacer nada al mismo
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

// NECESITO SABER SI HUBO MISS EN L1 Y SI SALIO UN DATO DEL CACHE
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