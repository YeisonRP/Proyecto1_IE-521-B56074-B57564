
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
#include "L1cache.h"
#include "debug_utilities.h"
#include <pthread.h>

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




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Verificar hit hit que se cambie el dato en L2 si corresponde
void* lru_L1_L2_replacement_policy (void* datos_funcion)
{
   // Arreglando funcion para que sea compatible con pthreads
   datos_de_funcion *datos = (datos_de_funcion*)datos_funcion;
   int associativityL2 = datos->associativity * 2;
     
// -------------------- Creando asociatividad para L2 y otras variables del sistema ----------------


   bool hit_o_missL1 = false;
   bool hit_o_missL2 = false;

// ------------------- Verificar si el otro core tiene el dato ------------------------
   bool otro_core_tiene_el_dato;
   if (get_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core) != NONE_)
   {
      otro_core_tiene_el_dato = true;
   }
   else
   {
      otro_core_tiene_el_dato = false;
   }
   
   
//------------------------------ Verificar Si es Hit en L1-----------------------------
   for(int i = 0; i < datos->associativity; i++)
   {
      if(datos->cache_blocks[i].tag == datos->tag && datos->cache_blocks[i].valid)
      {  //---------------------ocurrio un hit en L1 ----------------------
         for(int j = 0; j < datos->associativity; j++)
         {
            if(datos->cache_blocks[j].rp_value < (datos->cache_blocks[i].rp_value)){  datos->cache_blocks[j].rp_value += 1;   }  //suma 1 a la politica de remplazo
         }
         hit_o_missL1 = true;                   // Declara al sistema que hubo hit en L1
         datos->cache_blocks[i].rp_value = 0;          // Le asigna valor de remplazo 0.
         datos->operation_result_L2_->evicted_addressL1 = 0; // no sale nada de L1 por que es un hit.
         
         if(datos->core){datos->operation_result_L2_->Hit_L1_C2 +=1;}
         else{datos->operation_result_L2_->Hit_L1_C1 +=1;}

         if(datos->loadstore){  
            //------- si es un hit store-----------
            //--------- Busca el datos->tag en L2 para ponerlo sucio-------

            datos->cache_blocks[i].state =  MODIFIED;  // Cambia el estado a Modified **********

            set_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core,INVALID);  // Invalida el dato si está en el otro core ******** // set_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core[datos->idx],INVALID);

            if(datos->core){ datos->operation_result_L2_->Coherence_Inv_C1 +=1; } // Aumenta el contador de invalidos en el core correspondiente**
            else{ datos->operation_result_L2_->Coherence_Inv_C2 +=1; }   

            for(int a = 0; a < associativityL2; a++){
               if(datos->cache_blocksL2[a].tag == datos->tagL2){        // SI el dato esta en L2
                  datos->cache_blocksL2[a].state =  MODIFIED;    // Cambia el estado del dato en L2 como Modified *******
                  datos->cache_blocksL2[a].dirty = true;
                  a = associativityL2;
               }
            }
         }

         

 
         //------------Terminando el for----------- 
         i = datos->associativity;
      } 
   }
//------------------------------ Se encontro que es un miss en L1 -----------------------------
   if(!hit_o_missL1){
      if(datos->core){datos->operation_result_L2_->Miss_L1_C2 +=1;}
      else{datos->operation_result_L2_->Miss_L1_C1 +=1;}
      

      //------------------------Busca en L2----------------------------------------------
      for(int i = 0; i < associativityL2; i++){
         if(datos->cache_blocksL2[i].tag == datos->tagL2 && datos->cache_blocksL2[i].valid){  
            
            //---------------------ocurrio un hit en L2 ----------------------
            for(int j = 0; j < associativityL2; j++){
               if(datos->cache_blocksL2[j].rp_value < (datos->cache_blocksL2[i].rp_value)){  datos->cache_blocksL2[j].rp_value += 1;   }  //suma 1 a la politica de remplazo
            }
            hit_o_missL2 = true;                  // Indica al sistema que hubo un hit en L2
            datos->cache_blocksL2[i].rp_value = 0;       // Asiga un cero al valor de reemplazo
            datos->operation_result_L2_->Hit_L2 += 1;     // suma un hit a L2
            
            if(datos->loadstore){  
               //------- si es un hit store en L2 pone sucio el dato en L2 -----------

               datos->cache_blocksL2[i].state = MODIFIED;       // Pone el estado en Modified *********

               datos->cache_blocksL2[i].dirty = true; 

               // Revisar si el otro core lo tiene y ponerlo invalido
            }
             else{ //-------------Si es un hit load en L2 -----------------------
                  
                  if(otro_core_tiene_el_dato){ // Si el otro core lo tiene pasa a shared******** // if(get_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core[datos->idx]) != NONE_){ 
                     datos->cache_blocksL2[i].state = SHARED;       // Pone el estado del dato en L2 en SHARED *********   
                  }
                  else{
                     datos->cache_blocksL2[i].state = SHARED;       // Pone el estado del dato en L2 en SHARED si es MSI y el otro core no lo tiene *********   
                     if(datos->cp == 1){
                        datos->cache_blocksL2[i].state = EXCLUSIVE; // Pone el estado del dato en L2 en EXCLUSIVE si es MESI y el otro core no tiene el dato ********* 
                     }         
                  }
             }

          //------------------------ Guarda el dato en L1-------------------------------
            for(int m = 0; m < datos->associativity; m++){
               
               //----------------si es el bloque del set con menos prioridad---------------------------
               if(datos->cache_blocks[m].rp_value == (datos->associativity - 1)){  
                  datos->operation_result_L2_->evicted_addressL1 = (datos->cache_blocks[m].valid)? datos->cache_blocks[m].tag: 0 ; 
                  datos->cache_blocks[m].valid = 1;                      //---- Es valido ya que se va a escribir sobre el----
                  datos->cache_blocks[m].tag = datos->tag;                      //-----tag nuevo guardado en el set----------
               
                  if (datos->loadstore){   //STORE
                     datos->cache_blocks[m].state = MODIFIED;  // Cambia el estado a MODIFIED en L1 ***********
                     
                     if (otro_core_tiene_el_dato){ // Si el otro procesador tiene el dato *******
                        set_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core,INVALID);  // Invalida el dato si está en el otro core ******** 
                        //cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << endl;
                        if(datos->core){ datos->operation_result_L2_->Coherence_Inv_C1 +=1; } // Aumenta el contador de invalidos en el core correspondiente**
                        else{ datos->operation_result_L2_->Coherence_Inv_C2 +=1; }  
                     
                     } 
                  }
                     
                  else { // LOAD
                     if(otro_core_tiene_el_dato){ // Si estoy leyendo y el otro core tiene el dato...*****
                        set_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core,SHARED);   // Pongo el dato en el otro core como shared *********
                       // cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << endl;
                        datos->cache_blocks[m].state = SHARED;                                    // Cambia el estado a SHARED en L1*****
                     }
                     else{ 
                        datos->cache_blocks[m].state = SHARED;       // Pone el estado del dato en L1 en SHARED si es MSI y el otro core no lo tiene *********   
                        if(datos->cp == 1){
                           //YEISON~~~~~~~~~~~~~~ 
                           datos->cache_blocks[m].state = EXCLUSIVE;}      // Pone el estado del dato en L1 en EXCLUSIVE si es MESI y el otro core no tiene el dato *********   
                     }
                  }
                


                //----------suma 1 a los valores de remplazo correspondientes ----------------
                  for(int j = 0; j < datos->associativity; j++){  
                     if(datos->cache_blocks[j].rp_value < (datos->associativity - 1)){  datos->cache_blocks[j].rp_value += 1;   }  
                  }
                  datos->cache_blocks[m].rp_value = 0; //---- el dato que se ingreso/guardo con remplazo en 0-----
                  m = datos->associativity;            // ---- Termina el for ----
               } 
            }

          //------------Terminando el for-----------   
            i = associativityL2;
         }
      }

      //------------------------------- Hubo un miss en L1 y en L2----------------------------------------------
      if(!hit_o_missL2){
         datos->operation_result_L2_->Miss_L2 +=1;

         //----------------Guarda el nuevo valor en L2 ---------------------------
         for(int i = 0; i < associativityL2; i++){

            //----------------si es el bloque del set con menos prioridad---------------------------
            if(datos->cache_blocksL2[i].rp_value == (associativityL2 - 1)){  
           
               datos->operation_result_L2_->evicted_addressL2 = (datos->cache_blocksL2[i].valid)? datos->cache_blocksL2[i].tag: 0 ; 
            
               datos->cache_blocksL2[i].valid = 1;                      //---- Es valido ya que se va a escribir sobre el----
               datos->cache_blocksL2[i].tag = datos->tagL2;                      //-----datos->tag nuevo guardado en el set----------
               datos->operation_result_L2_->dirty_eviction += (datos->cache_blocksL2[i].dirty)? 1: 0;   //----Si hubo dirty eviction-----
               
               if(datos->loadstore){
               // -----------si hubo miss store----------------
                  datos->cache_blocksL2[i].dirty = true;    
                  datos->cache_blocksL2[i].state = MODIFIED; // Pone el estado en Modified ********** 
               }
               else { // -----------si hubo miss load----------------
                     if(datos->cp == 1){   // si es MESI
                         datos->cache_blocksL2[i].state = EXCLUSIVE; // Pone el estado en EXCLUSIVE si es MESI **********     
                     }
                     else{ 
                        datos->cache_blocksL2[i].state = SHARED; // Pone el estado en SHARED si es MSI ********** 
                     }
               } 
         
               //----------suma 1 a los valores de remplazo correspondientes ----------------
               for(int j = 0; j < associativityL2; j++){  
                  if(datos->cache_blocksL2[j].rp_value < (associativityL2 - 1)){  datos->cache_blocksL2[j].rp_value += 1; }  
               }
               datos->cache_blocksL2[i].rp_value = 0; //---- el dato que se ingreso/guardo con remplazo en 0-----
               i = associativityL2;            // ---- Termina el for ----
            } 
         }

         //----------------Guarda el nuevo valor en L1 ---------------------------
         for(int i = 0; i < datos->associativity; i++){

            //----------------si es el bloque del set con menos prioridad---------------------------
            if(datos->cache_blocks[i].rp_value == (datos->associativity - 1)){  
               datos->operation_result_L2_->evicted_addressL1 = (datos->cache_blocks[i].valid)? datos->cache_blocks[i].tag: 0 ; 
               datos->cache_blocks[i].valid = 1;                      //---- Es valido ya que se va a escribir sobre el----
               datos->cache_blocks[i].tag = datos->tag;                      //-----datos->tag nuevo guardado en el set----------
               
               // ------------------------Cambia el estado en L1 -----------------------------------

               if(datos->loadstore){
               // -----------si hubo miss store----------------
                  datos->cache_blocks[i].dirty = true;    
                  datos->cache_blocks[i].state = MODIFIED; // Pone el estado en Modified ********** 

                  if (otro_core_tiene_el_dato){ // Si el otro procesador tiene el dato *******
                    // cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << endl;
                     set_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core,INVALID);  // shared el dato si está en el otro core (ESTA CONDICION NO CREO QUE PASE)******** 
                  } 
               }
               else {
                  // -----------si hubo miss load----------------
                     if(datos->cp == 1){
                        datos->cache_blocks[i].state = EXCLUSIVE; // Pone el estado en EXCLUSIVE si es MESI********** 
                        if (otro_core_tiene_el_dato){ // Si el otro procesador tiene el dato *******
                        //   cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << endl;
                           set_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core,INVALID);  // Invalida el dato si está en el otro core (ESTA CONDICION NO CREO QUE PASE)******** 
                        } 
                     }
                     else{ 
                        datos->cache_blocks[i].state = SHARED; // Pone el estado en SHARED si es MSI ********** 

                        if (otro_core_tiene_el_dato){ // Si el otro procesador tiene el dato *******
                           set_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core,INVALID);   // invalida el dato si está en el otro core (ESTA CONDICION NO CREO QUE PASE)******** 
                        } 
                     }
               } 


               //----------suma 1 a los valores de remplazo correspondientes ----------------
               for(int j = 0; j < datos->associativity; j++){  
                  if(datos->cache_blocks[j].rp_value < (datos->associativity - 1)){  datos->cache_blocks[j].rp_value += 1; }  
               }
               datos->cache_blocks[i].rp_value = 0; //---- el dato que se ingreso/guardo con remplazo en 0-----
               i = datos->associativity;            // ---- Termina el for ----
            } 
         }      
      }      
   }
     pthread_exit(NULL);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Verificar hit hit que se cambie el dato en L2 si corresponde
void* lru_L1_L2_replacement_policy_no_threads (void* datos_funcion)
{
   // Arreglando funcion para que sea compatible con pthreads
   datos_de_funcion *datos = (datos_de_funcion*)datos_funcion;
   int associativityL2 = datos->associativity * 2;
     
// -------------------- Creando asociatividad para L2 y otras variables del sistema ----------------


   bool hit_o_missL1 = false;
   bool hit_o_missL2 = false;

// ------------------- Verificar si el otro core tiene el dato ------------------------
   bool otro_core_tiene_el_dato;
   if (get_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core) != NONE_)
   {
      otro_core_tiene_el_dato = true;
   }
   else
   {
      otro_core_tiene_el_dato = false;
   }
   
   
//------------------------------ Verificar Si es Hit en L1-----------------------------
   for(int i = 0; i < datos->associativity; i++)
   {
      if(datos->cache_blocks[i].tag == datos->tag && datos->cache_blocks[i].valid)
      {  //---------------------ocurrio un hit en L1 ----------------------
         for(int j = 0; j < datos->associativity; j++)
         {
            if(datos->cache_blocks[j].rp_value < (datos->cache_blocks[i].rp_value)){  datos->cache_blocks[j].rp_value += 1;   }  //suma 1 a la politica de remplazo
         }
         hit_o_missL1 = true;                   // Declara al sistema que hubo hit en L1
         datos->cache_blocks[i].rp_value = 0;          // Le asigna valor de remplazo 0.
         datos->operation_result_L2_->evicted_addressL1 = 0; // no sale nada de L1 por que es un hit.
         
         if(datos->core){datos->operation_result_L2_->Hit_L1_C2 +=1;}
         else{datos->operation_result_L2_->Hit_L1_C1 +=1;}

         if(datos->loadstore){  
            //------- si es un hit store-----------
            //--------- Busca el datos->tag en L2 para ponerlo sucio-------

            datos->cache_blocks[i].state =  MODIFIED;  // Cambia el estado a Modified **********

            set_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core,INVALID);  // Invalida el dato si está en el otro core ******** // set_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core[datos->idx],INVALID);

            if(datos->core){ datos->operation_result_L2_->Coherence_Inv_C1 +=1; } // Aumenta el contador de invalidos en el core correspondiente**
            else{ datos->operation_result_L2_->Coherence_Inv_C2 +=1; }   

            for(int a = 0; a < associativityL2; a++){
               if(datos->cache_blocksL2[a].tag == datos->tagL2){        // SI el dato esta en L2
                  datos->cache_blocksL2[a].state =  MODIFIED;    // Cambia el estado del dato en L2 como Modified *******
                  datos->cache_blocksL2[a].dirty = true;
                  a = associativityL2;
               }
            }
         }

         

// YEISON~~~~~~~~~~~~~~~~~~~~~~~~
   /////////////////////////////////////////////////////////////SERA QUE ESTO HAY QUE HACERLO?
 /*     // -----------Actualizando el valor de reemplazo en L2                            
            for(int x = 0; x < associativityL2; x++){
               if(datos->cache_blocksL2[x].tag == datos->tagL2 && datos->cache_blocksL2[x].valid){  
                  for(int j = 0; j < associativityL2; j++){
                     if(datos->cache_blocksL2[j].rp_value < (datos->cache_blocksL2[x].rp_value)){  datos->cache_blocksL2[j].rp_value += 1;   }  //suma 1 a la politica de remplazo
                  }                           
                  datos->cache_blocksL2[x].rp_value = 0;  
                  x = associativityL2;     
               }
            }
             */  
         //------------Terminando el for----------- 
         i = datos->associativity;
      } 
   }
//------------------------------ Se encontro que es un miss en L1 -----------------------------
   if(!hit_o_missL1){
      if(datos->core){datos->operation_result_L2_->Miss_L1_C2 +=1;}
      else{datos->operation_result_L2_->Miss_L1_C1 +=1;}
      

      //------------------------Busca en L2----------------------------------------------
      for(int i = 0; i < associativityL2; i++){
         if(datos->cache_blocksL2[i].tag == datos->tagL2 && datos->cache_blocksL2[i].valid){  
            
            //---------------------ocurrio un hit en L2 ----------------------
            for(int j = 0; j < associativityL2; j++){
               if(datos->cache_blocksL2[j].rp_value < (datos->cache_blocksL2[i].rp_value)){  datos->cache_blocksL2[j].rp_value += 1;   }  //suma 1 a la politica de remplazo
            }
            hit_o_missL2 = true;                  // Indica al sistema que hubo un hit en L2
            datos->cache_blocksL2[i].rp_value = 0;       // Asiga un cero al valor de reemplazo
            datos->operation_result_L2_->Hit_L2 += 1;     // suma un hit a L2
            
            if(datos->loadstore){  
               //------- si es un hit store en L2 pone sucio el dato en L2 -----------

               datos->cache_blocksL2[i].state = MODIFIED;       // Pone el estado en Modified *********

               datos->cache_blocksL2[i].dirty = true; 

               // Revisar si el otro core lo tiene y ponerlo invalido
            }
             else{ //-------------Si es un hit load en L2 -----------------------
                  
                  if(otro_core_tiene_el_dato){ // Si el otro core lo tiene pasa a shared******** // if(get_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core[datos->idx]) != NONE_){ 
                     datos->cache_blocksL2[i].state = SHARED;       // Pone el estado del dato en L2 en SHARED *********   
                  }
                  else{
                     datos->cache_blocksL2[i].state = SHARED;       // Pone el estado del dato en L2 en SHARED si es MSI y el otro core no lo tiene *********   
                     if(datos->cp == 1){
                        datos->cache_blocksL2[i].state = EXCLUSIVE; // Pone el estado del dato en L2 en EXCLUSIVE si es MESI y el otro core no tiene el dato ********* 
                     }         
                  }
             }

          //------------------------ Guarda el dato en L1-------------------------------
            for(int m = 0; m < datos->associativity; m++){
               
               //----------------si es el bloque del set con menos prioridad---------------------------
               if(datos->cache_blocks[m].rp_value == (datos->associativity - 1)){  
                  datos->operation_result_L2_->evicted_addressL1 = (datos->cache_blocks[m].valid)? datos->cache_blocks[m].tag: 0 ; 
                  datos->cache_blocks[m].valid = 1;                      //---- Es valido ya que se va a escribir sobre el----
                  datos->cache_blocks[m].tag = datos->tag;                      //-----tag nuevo guardado en el set----------
               
                  if (datos->loadstore){   //STORE
                     datos->cache_blocks[m].state = MODIFIED;  // Cambia el estado a MODIFIED en L1 ***********
                     
                     if (otro_core_tiene_el_dato){ // Si el otro procesador tiene el dato *******
                        set_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core,INVALID);  // Invalida el dato si está en el otro core ******** 
                        //cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << endl;
                        if(datos->core){ datos->operation_result_L2_->Coherence_Inv_C1 +=1; } // Aumenta el contador de invalidos en el core correspondiente**
                        else{ datos->operation_result_L2_->Coherence_Inv_C2 +=1; }  
                     
                     } 
                  }
                     
                  else { // LOAD
                     if(otro_core_tiene_el_dato){ // Si estoy leyendo y el otro core tiene el dato...*****
                        set_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core,SHARED);   // Pongo el dato en el otro core como shared *********
                       // cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << endl;
                        datos->cache_blocks[m].state = SHARED;                                    // Cambia el estado a SHARED en L1*****
                     }
                     else{ 
                        datos->cache_blocks[m].state = SHARED;       // Pone el estado del dato en L1 en SHARED si es MSI y el otro core no lo tiene *********   
                        if(datos->cp == 1){
                           //YEISON~~~~~~~~~~~~~~ 
                           datos->cache_blocks[m].state = EXCLUSIVE;}      // Pone el estado del dato en L1 en EXCLUSIVE si es MESI y el otro core no tiene el dato *********   
                     }
                  }
                


                //----------suma 1 a los valores de remplazo correspondientes ----------------
                  for(int j = 0; j < datos->associativity; j++){  
                     if(datos->cache_blocks[j].rp_value < (datos->associativity - 1)){  datos->cache_blocks[j].rp_value += 1;   }  
                  }
                  datos->cache_blocks[m].rp_value = 0; //---- el dato que se ingreso/guardo con remplazo en 0-----
                  m = datos->associativity;            // ---- Termina el for ----
               } 
            }

          //------------Terminando el for-----------   
            i = associativityL2;
         }
      }

      //------------------------------- Hubo un miss en L1 y en L2----------------------------------------------
      if(!hit_o_missL2){
         datos->operation_result_L2_->Miss_L2 +=1;

         //----------------Guarda el nuevo valor en L2 ---------------------------
         for(int i = 0; i < associativityL2; i++){

            //----------------si es el bloque del set con menos prioridad---------------------------
            if(datos->cache_blocksL2[i].rp_value == (associativityL2 - 1)){  
           
               datos->operation_result_L2_->evicted_addressL2 = (datos->cache_blocksL2[i].valid)? datos->cache_blocksL2[i].tag: 0 ; 
            
               datos->cache_blocksL2[i].valid = 1;                      //---- Es valido ya que se va a escribir sobre el----
               datos->cache_blocksL2[i].tag = datos->tagL2;                      //-----datos->tag nuevo guardado en el set----------
               datos->operation_result_L2_->dirty_eviction += (datos->cache_blocksL2[i].dirty)? 1: 0;   //----Si hubo dirty eviction-----
               
               if(datos->loadstore){
               // -----------si hubo miss store----------------
                  datos->cache_blocksL2[i].dirty = true;    
                  datos->cache_blocksL2[i].state = MODIFIED; // Pone el estado en Modified ********** 
               }
               else { // -----------si hubo miss load----------------
                     if(datos->cp == 1){   // si es MESI
                         datos->cache_blocksL2[i].state = EXCLUSIVE; // Pone el estado en EXCLUSIVE si es MESI **********     
                     }
                     else{ 
                        datos->cache_blocksL2[i].state = SHARED; // Pone el estado en SHARED si es MSI ********** 
                     }
               } 
         
               //----------suma 1 a los valores de remplazo correspondientes ----------------
               for(int j = 0; j < associativityL2; j++){  
                  if(datos->cache_blocksL2[j].rp_value < (associativityL2 - 1)){  datos->cache_blocksL2[j].rp_value += 1; }  
               }
               datos->cache_blocksL2[i].rp_value = 0; //---- el dato que se ingreso/guardo con remplazo en 0-----
               i = associativityL2;            // ---- Termina el for ----
            } 
         }

         //----------------Guarda el nuevo valor en L1 ---------------------------
         for(int i = 0; i < datos->associativity; i++){

            //----------------si es el bloque del set con menos prioridad---------------------------
            if(datos->cache_blocks[i].rp_value == (datos->associativity - 1)){  
               datos->operation_result_L2_->evicted_addressL1 = (datos->cache_blocks[i].valid)? datos->cache_blocks[i].tag: 0 ; 
               datos->cache_blocks[i].valid = 1;                      //---- Es valido ya que se va a escribir sobre el----
               datos->cache_blocks[i].tag = datos->tag;                      //-----datos->tag nuevo guardado en el set----------
               
               // ------------------------Cambia el estado en L1 -----------------------------------

               if(datos->loadstore){
               // -----------si hubo miss store----------------
                  datos->cache_blocks[i].dirty = true;    
                  datos->cache_blocks[i].state = MODIFIED; // Pone el estado en Modified ********** 

                  if (otro_core_tiene_el_dato){ // Si el otro procesador tiene el dato *******
                    // cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << endl;
                     set_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core,INVALID);  // shared el dato si está en el otro core (ESTA CONDICION NO CREO QUE PASE)******** 
                  } 
               }
               else {
                  // -----------si hubo miss load----------------
                     if(datos->cp == 1){
                        datos->cache_blocks[i].state = EXCLUSIVE; // Pone el estado en EXCLUSIVE si es MESI********** 
                        if (otro_core_tiene_el_dato){ // Si el otro procesador tiene el dato *******
                        //   cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << endl;
                           set_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core,INVALID);  // Invalida el dato si está en el otro core (ESTA CONDICION NO CREO QUE PASE)******** 
                        } 
                     }
                     else{ 
                        datos->cache_blocks[i].state = SHARED; // Pone el estado en SHARED si es MSI ********** 

                        if (otro_core_tiene_el_dato){ // Si el otro procesador tiene el dato *******
                           set_coherence_state(datos->tag,datos->associativity,datos->Other_L1_Core,INVALID);   // invalida el dato si está en el otro core (ESTA CONDICION NO CREO QUE PASE)******** 
                        } 
                     }
               } 


               //----------suma 1 a los valores de remplazo correspondientes ----------------
               for(int j = 0; j < datos->associativity; j++){  
                  if(datos->cache_blocks[j].rp_value < (datos->associativity - 1)){  datos->cache_blocks[j].rp_value += 1; }  
               }
               datos->cache_blocks[i].rp_value = 0; //---- el dato que se ingreso/guardo con remplazo en 0-----
               i = datos->associativity;            // ---- Termina el for ----
            } 
         }      
      }      
   }

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
      
void simulation_outL2(  int cache_size_kb, 
                        int associativity, 
                        int block_size,  
                        int cp, 
                        operation_result_L2* L2)
   {
      double C1L1MR = (double)L2->Miss_L1_C1 / double(L2->Miss_L1_C1 + L2->Hit_L1_C1);
      double C2L1MR = (double)L2->Miss_L1_C2 / double(L2->Miss_L1_C2 + L2->Hit_L1_C2);
      double OMR = double(L2->Miss_L1_C1 + L2->Miss_L1_C2) / double(L2->Hit_L1_C2 + L2->Hit_L1_C1 + L2->Miss_L1_C1 + L2->Miss_L1_C2);
      string CoherenceProtocol;
      if(cp == 1){ 
         CoherenceProtocol = "MESI"; 
      }
      else {
         CoherenceProtocol = "MSI";
      }

      cout << "------------------------------------------\n";
            cout << "  Cache parameters:\n";
            cout << "------------------------------------------\n";
            cout << "  L1 Cache Size (KB): "<<"          " << cache_size_kb << "\n";
            cout << "  L2 Cache Size (KB): "<<"          " << cache_size_kb*4 << "\n";
            cout << "  Cache L1 Associativity: "<<"      " << associativity << "\n";
            cout << "  Cache L2 Associativity: "<<"      " << associativity*2 << "\n";
            cout << "  Cache Block Size (bytes):"<<"     " << block_size << "\n";
            cout << "  Coherence protocol                " << CoherenceProtocol <<"\n";
            cout << "------------------------------------------\n";
            cout << "  Simulation results:\n";
            cout << "------------------------------------------\n";
            cout << "  Overall miss rate"<<"               " << OMR <<"\n";
            cout << "  CPU1 L1 miss rate:"<<"              " << C1L1MR <<"\n";
            cout << "  CPU2 L1 miss rate:"<<"              " << C2L1MR<<"\n";
            cout << "  Coherence Invalidation CPU1"<<"     " << L2->Coherence_Inv_C1 <<"\n";
            cout << "  Coherence Invalidation CPU2"<<"     " << L2->Coherence_Inv_C2 <<"\n";
            cout << "------------------------------------------\n";
     }       


//-- PROBADA
coherence get_coherence_state (int tag,
                               int associativity,
                               entry* cache_blocks)
{

   for (int i = 0; i < associativity; i++)
   {
      if (cache_blocks[i].tag == tag && cache_blocks[i].valid)// && cache_blocks[i].valid == 0)   //-- Si se encuentra el dato y no esta valido
      {
         return cache_blocks[i].state; //-- Retorna el estado si encontro el dato
      }
   }
   return NONE_; //-- Si no encontro el dato en el set retorna un NONE_

}

//-- PROBADA
void set_coherence_state (int tag,
                          int associativity,
                          entry* cache_blocks,
                          coherence coherence_state)
{

   for (int i = 0; i < associativity; i++)
   {
      if (cache_blocks[i].tag == tag)
      {
         cache_blocks[i].state = coherence_state; // Escribe el dato

         if (coherence_state == INVALID)
         {
            cache_blocks[i].valid = false; // si el estado es invalido, se ivalida el dato
         } 
      }
   }
}