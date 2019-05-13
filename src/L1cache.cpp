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
               if(cache_blocks[j].rp_value < (associativity - 1)){  cache_blocks[j].rp_value += 1;   }  
            }
            cache_blocks[i].rp_value = 0; //---- el dato que se ingreso/guardo con remplazo en 0-----
            i = associativity;            // ---- Termina el for ----
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
