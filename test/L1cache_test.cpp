/*
 *  Cache simulation project
 *  Class UCR IE-521
 *  Semester: I-2019
*/

#include <gtest/gtest.h>
#include <time.h>
#include <stdlib.h>
#include <debug_utilities.h>
#include <L1cache.h>
#include <math.h>
#define YEL   "\x1B[33m"

/* Globals */
int debug_on = 0;
using namespace std;
/* Test Helpers */
#define DEBUG(x) if (debug_on) printf("%s\n",#x)



/*
 * VICTIM CACHE miss hit
 * 
 * Stimuli
 * 1. Choose a random associativity
 * 2. Choose a random address (AddressA)
 * 3. Fill a victim cache with random addresses and include AddressA.
 * 4. Fill a cache line with random addresses, making sure AddressA is not added.
 * 5. Read or Write (choose the operation randomly) AddressA. Check 1,2,3 and 4.
 * 
 * Checks
 * 1. Check operation result is a HIT
 * 2. Check LRU data in L1 line is swapped with AddressA data in the victim cache.
 * 3. Check replacement policy values in L1 were updated properly.
 * 4. Check dirty bit value of AddressA was changed accordingly with the operation performed
 */

TEST(VC, miss_hit){
  int associativity;
  int adress_A;

/////////////////////      1. Choose a random associativity      ///////////////////
  associativity = 1 << (rand()%4);  // associativity puede ser 1, 2 , 4, 8 
  
/////////////////////      2. Choose a random address (AddressA)      ///////////////////

  int idex_size = rand()%8 + 1; //idx_size
  int tag_random = rand()%4096; // tag random
  int indx_random = rand()%((int)pow(2,idex_size)); // 4 bits de index
  int tag_A = tag_random;
  int idx_A = indx_random;
  adress_A = joining_tag_index(idex_size,indx_random,tag_random);    //direccion random A

/////////////////////     3. Fill a victim cache with random addresses and include AddressA.      ///////////////////
  bool random_dirty = rand()%2;
   
  int adress_rand; 
  entry * vc;
  vc = creando_victim_cache();
  bool control = true;
  for (int i = 0; i < 16; i++)
  {
    // calculando que el tag que se ingrese no este en el VC
    control = true;
    while (control)
    {
      tag_random = rand()%4096; // tag random
      adress_rand = joining_tag_index(idex_size,indx_random,tag_random);
      while (adress_rand == adress_A)
      {
        tag_random = rand()%4096; // tag random
        adress_rand = joining_tag_index(idex_size,indx_random,tag_random);
      }
      control = false;
      for (int j = 0; j < 16; j++)
      {
        if (adress_rand == vc[j].tag & vc[j].valid == true)
        {
          control = true;
        } 
      }  
    }

    vc[i].valid = 1;
    if (i == 8)
    {
      vc[i].tag = adress_A;
      vc[i].dirty = random_dirty;
    }
    else
    {
      vc[i].tag = adress_rand;
      vc[i].dirty = 0;
    }
  }
  

/////////////////////     4. Fill a cache line with random addresses, making sure AddressA is not added.      ///////////////////
  int* cantidad_sets = new int;
  entry** cache = creando_matriz_cache(idex_size,associativity,cantidad_sets);
  int bloque_a_estar_en_VC;
  bool loadstore = rand()%2; // 0 load 1 store

  for (int i = 0; i < associativity; i++)
  {
    control = true;
    while (control)
    {
      tag_random = rand()%4096; // tag random
      adress_rand = joining_tag_index(idex_size,indx_random,tag_random);
      while (adress_rand == adress_A)
      {
        tag_random = rand()%4096; // tag random
        adress_rand = joining_tag_index(idex_size,indx_random,tag_random);
      }  
      control = false;
      for (int j = 0; j < associativity; j++)
      {
        if (adress_rand == cache[indx_random][j].tag & cache[indx_random][j].valid == true)
        {
          control = true;
        } 
      }  
    }

    cache[indx_random][i].tag = tag_random;
    cache[indx_random][i].dirty = 0;
    cache[indx_random][i].valid = 1;
    cache[indx_random][i].rp_value = i;
    bloque_a_estar_en_VC = joining_tag_index(idex_size,indx_random,tag_random);
  }

/////////////////////     5. Read or Write (choose the operation randomly) AddressA. Check 1,2,3 and 4.      ///////////////////

  
  operation_result_vc* resultado_vc = new operation_result_vc;
  operation_result* resultado_L1 = new operation_result;
  int* miss = new int;
  int* hits = new int;
  int* vc_hits = new int;
  int* dirty = new int;
  int* index = new int;
  int* tag_r = new int;
  // reading or writing

  comun_vc_L1(tag_A,idx_A,idex_size,associativity,loadstore,vc,cache[indx_random],resultado_vc,resultado_L1,miss,hits,vc_hits,dirty);

/*
 * 1. Check operation result is a HIT
 * 2. Check LRU data in L1 line is swapped with AddressA data in the victim cache.
 * 3. Check replacement policy values in L1 were updated properly.
 * 4. Check dirty bit value of AddressA was changed accordingly with the operation performed
 */

  
  // 1. Check operation result is a HIT
  if (loadstore) // store
  {
    EXPECT_EQ(resultado_L1->miss_hit, MISS_STORE);  //revisa que fuera un miss store en L1
  }
  else
  {
    EXPECT_EQ(resultado_L1->miss_hit, MISS_LOAD); //revisa que fuera un miss load en L1
  }
  
  EXPECT_EQ(resultado_vc->miss_hit, HIT); //revisa que fuera un hit en VC

  
  // 2. Check LRU data in L1 line is swapped with AddressA data in the victim cache.
  EXPECT_EQ(vc[0].tag, bloque_a_estar_en_VC); //Revisa que el bloque de la cache L1 ingresara al VC

  // 3. Check replacement policy values in L1 were updated properly.
  for (int i = 0; i < associativity; i++)
  {
    if (cache[indx_random][i].rp_value == 0)
    {
      EXPECT_EQ(cache[indx_random][i].tag,tag_A); //Revisa que el bloque que entro con remplazo 0 a L1                                  
    }                                             //fuera el tag_A del VC que salio y entro a L1
    else
    {
      EXPECT_EQ((int)cache[indx_random][i].rp_value, i+1);  // Revisa que los demas tengan el valor de remplazo esperado
    }
  }
  // 4. Check dirty bit value of AddressA was changed accordingly with the operation performed
  if (loadstore) //store
  {
    EXPECT_EQ(cache[indx_random][associativity-1].dirty, true); // Revisa que si fue un store, bit da valido 1 en la cache
  }
  else        // load
  {
    EXPECT_EQ(cache[indx_random][associativity-1].dirty, random_dirty); // Revisa que si fue un load, bit da valido random_dirty en la cache
  }

  // eliminando memoria dinamica

  delete miss, hits, vc_hits, dirty, resultado_L1, resultado_vc,index,tag_r;
  for(int i = 0; i < ((int)pow(2,idex_size)); i++)
  {
    delete[] cache[i];
  }
  delete[] cache; 
}

/*
 * VICTIM CACHE : miss miss
 * 
 * Stimuli
 * 1. Choose a random associativity
 * 2. Choose a random address (AddressA)
 * 3. Fill a victim cache with random addresses and include, make sure AddressA is not added.
 * 4. Fill a cache line with random addresses, make sure AddressA is not added.
 * 5. Read or Write (choose the operation randomly) AddressA. Check 1,2,3 and 4.
 * 
 * Checks
 * 1. Check operation result is a MISS
 * 2. Check LRU data in L1 line is now in the victim cache.
 * 3. Check LRU data in VC is discarded
 * 4. Check replacement policy values in L1 were updated properly.
 * 5. Check dirty bit value of AddressA was changed accordingly with the operation performed
 */
TEST(VC, miss_miss){

  int associativity;
  int adress_A;

/////////////////////      1. Choose a random associativity      ///////////////////
  associativity = 1 << (rand()%4);  // associativity puede ser 1, 2 , 4, 8 
  
/////////////////////      2. Choose a random address (AddressA)      ///////////////////

  int idex_size = rand()%8 + 1; //idx_size
  int tag_random = rand()%4096; // tag random
  int indx_random = rand()%((int)pow(2,idex_size)); // 4 bits de index
  int tag_A = tag_random;
  int idx_A = indx_random;
  adress_A = joining_tag_index(idex_size,indx_random,tag_random);    //direccion random A

/////////////////////     3. Fill a victim cache with random addresses and include, make sure AddressA is not added.      ///////////////////
  bool random_dirty = rand()%2;
   
  int adress_rand; 
  entry * vc;
  vc = creando_victim_cache();
  bool control = true;
  for (int i = 0; i < 16; i++)
  {
    // calculando que el tag que se ingrese no este en el VC
    bool control = true;
    while (control)
    {
      tag_random = rand()%4096; // tag random
      adress_rand = joining_tag_index(idex_size,indx_random,tag_random);
      while (adress_rand == adress_A)
      {
        tag_random = rand()%4096; // tag random
        adress_rand = joining_tag_index(idex_size,indx_random,tag_random);
      }
      control = false;
      for (int j = 0; j < 16; j++)
      {
        if (adress_rand == vc[j].tag & vc[j].valid == true)
        {
          control = true;
        } 
      }  
    }
    vc[i].valid = 1;
    vc[i].tag = adress_rand;
    vc[i].dirty = 0;
    
  }
  

/////////////////////     4. Fill a cache line with random addresses, make sure AddressA is not added.      ///////////////////
  int* cantidad_sets = new int;
  entry** cache = creando_matriz_cache(idex_size,associativity,cantidad_sets);
  int Bloque_a_salir_de_L1;
  int Bloque_a_salir_de_VC = vc[15].tag;

  for (int i = 0; i < associativity; i++)
  {
    control = true;
    while (control)
    {
      tag_random = rand()%4096; // tag random
      adress_rand = joining_tag_index(idex_size,indx_random,tag_random);
      while (adress_rand == adress_A)
      {
        tag_random = rand()%4096; // tag random
        adress_rand = joining_tag_index(idex_size,indx_random,tag_random);
      }  
      control = false;
      for (int j = 0; j < associativity; j++)
      {
        if (adress_rand == cache[indx_random][j].tag & cache[indx_random][j].valid == true)
        {
          control = true;
        } 
      }  
    }

    cache[indx_random][i].tag = tag_random;
    cache[indx_random][i].dirty = 0;
    cache[indx_random][i].valid = 1;
    cache[indx_random][i].rp_value = i;
    Bloque_a_salir_de_L1 = joining_tag_index(idex_size,indx_random,tag_random);
  }

/////////////////////     5. Read or Write (choose the operation randomly) AddressA. Check 1,2,3 and 4.      ///////////////////
  bool loadstore = rand()%2; // 0 load 1 store
  
  operation_result_vc* resultado_vc = new operation_result_vc;
  operation_result* resultado_L1 = new operation_result;
  int* miss = new int;
  int* hits = new int;
  int* vc_hits = new int;
  int* dirty = new int;
  int* index = new int;
  int* tag_r = new int;
  // reading or writing

  comun_vc_L1(tag_A,idx_A,idex_size,associativity,loadstore,vc,cache[indx_random],resultado_vc,resultado_L1,miss,hits,vc_hits,dirty);
/*
 * 1. Check operation result is a MISS
 * 2. Check LRU data in L1 line is now in the victim cache.
 * 3. Check LRU data in VC is discarded
 * 4. Check replacement policy values in L1 were updated properly.
 * 5. Check dirty bit value of AddressA was changed accordingly with the operation performed
 */

  
  // 1. Check operation result is a HIT
  if (loadstore) // store
  {
    EXPECT_EQ(resultado_L1->miss_hit, MISS_STORE);  //revisa que fuera un miss store en L1
  }
  else
  {
    EXPECT_EQ(resultado_L1->miss_hit, MISS_LOAD); //revisa que fuera un miss load en L1
  }
  
  EXPECT_EQ(resultado_vc->miss_hit, MISS); //revisa que fuera un miss en VC

  
  // 2. Check LRU data in L1 line is now in the victim cache. 

  // AQUIIIIIIIIIIII
  EXPECT_EQ(vc[0].tag, Bloque_a_salir_de_L1); //Revisa que el bloque de la cache L1 ingresara al VC
  
 // * 3. Check LRU data in VC is discarded
  for (int i = 0; i < 16; i++)
  {
    EXPECT_NE(vc[i].tag,Bloque_a_salir_de_VC); 
  }
  
 // * 4. Check replacement policy values in L1 were updated properly.
  for (int i = 0; i < associativity; i++)
  {
    if (i == associativity - 1)
    {
      EXPECT_EQ(cache[indx_random][i].rp_value,0);
      EXPECT_EQ(cache[indx_random][i].tag,tag_A);
    }
    else
    {
      EXPECT_EQ(cache[indx_random][i].rp_value,i+1);
    }
  }
  

 // * 5. Check dirty bit value of AddressA was changed accordingly with the operation performed
  for (int i = 0; i < associativity -1 ; i++)
  {
    if (cache[indx_random][i].tag == tag_A)
    {
      if (loadstore) //store
      {
        EXPECT_EQ(cache[indx_random][i].dirty, true); // Revisa que si fue un store, bit da valido 1 en la cache
      }
      else          // load
      {
        EXPECT_EQ(cache[indx_random][i].dirty, false); // Revisa que si fue un load, bit da valido random_dirty en la cache
      }
    }
  }
  
  // eliminando memoria dinamica
  delete miss, hits, vc_hits, dirty, resultado_L1, resultado_vc,index,tag_r;
  for(int i = 0; i < ((int)pow(2,idex_size)); i++)
  {
    delete[] cache[i];
  }
  delete[] cache; 

}


//-----CACHE_MULTI-NIVEL: L1 miss / L2 miss 
/*
* 1. Choose a random associativity
* 2. Choose a random address (AddressA)
* 3. Fill a l1 cache line with random addresses, making sure AddressA is not added.
* 4. Fill a l2 cache line with random addresses, making sure AddressA is not added.
* 5. Read or Write (choose the operation randomly) AddressA. Check 1,2,3 and 4.
*/


TEST(L2, miss_miss){

  int associativityL1,associativityL2;
  int adress_AL1, adress_AL2;

/////////////////////      1. Choose a random associativity      ///////////////////
  associativityL1 = 1 << (rand()%4);  // associativity puede ser 1, 2 , 4, 8 
  associativityL2 = associativityL1 << 1; //Es el doble de la asociatividad de L1
  
/////////////////////      2. Choose a random address (AddressA)      ///////////////////
  adress_AL1 = rand()%4096; // tag random
  adress_AL2 >> 1;
  
  struct entry cacheL1[associativityL1];  // Set de L1
  struct entry cacheL2[associativityL2];  //Set de L2
  struct operation_result_L2 resultL1L2 = {};

  int randomTagL1, randomTagL2;
  int LessUsedTagL1, LessUsedTagL2;          // Almacenan el tag que tiene el valor de remplazo proximo a salir de la cache

/////////////////////     3. Fill a l1 cache line with random addresses, making sure AddressA is not added.     ///////////////////
  
  for (int j =  0; j < associativityL1; j++) {

      do{randomTagL1 = rand()%4096; }
      while(randomTagL1 == adress_AL1);

      if(j==0){LessUsedTagL1=randomTagL1;}         // Guarda el tag que va a tener valor de reemplazo asociatividad-1

      cacheL1[j].valid = false;
      cacheL1[j].tag = randomTagL1;                               // Llenando L1
      cacheL1[j].dirty = 0;
      cacheL1[j].rp_value = associativityL1-1-j;
    }  

/////////////////////     4. Fill a l2 cache line with random addresses, making sure AddressA is not added.     ///////////////////
  

  for (int i =  0; i < associativityL2; i++) {

      do{randomTagL2 = rand()%4096; }
      while(randomTagL2 == adress_AL2);

      if(i==0){LessUsedTagL1=randomTagL2;}           // Guarda el tag que va a tener valor de reemplazo asociatividad-1

      cacheL2[i].valid = false;
      cacheL2[i].tag = randomTagL2;                               // Llenando L2
      cacheL2[i].dirty = 0;
      cacheL2[i].rp_value = associativityL2-1-i;
    } 


/////////////////////     5. Read or Write (choose the operation randomly) AddressA.     ///////////////////
   
    bool LS = rand()%2;                               // Escritura o lectura Random

    lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL2,associativityL1,LS,cacheL1,cacheL2,&resultL1L2,0);  
       

    /*
  1. Check operation result in L1 is a miss
  2. Check operation result in L2 is a miss
  3. Check LRU data in L1 line is evicted**.
  4. Check LRU data in L2 line is evicted***.
  5. Check replacement policy values in L1/L2 were updated properly.
  6. Check dirty bit value of AddressA was changed accordingly with the operation performed
 */

  
  // 1. Check operation result in L1 is a miss
    EXPECT_EQ(resultL1L2.MissL1, 1);  //revisa que fuera un miss en L1
  
  // 2. Check operation result in L2 is a miss 
 
    EXPECT_EQ(resultL1L2.MissL2,1); //revisa que fuera un miss en L2
  
 // 3. Check LRU data in L1 line is evicted**.

    EXPECT_EQ(resultL1L2.evicted_addressL1,LessUsedTagL1); //revisa que sacara al tag con mayor  valor de reemplazo en L1
  
  
 // 4. Check LRU data in L2 line is evicted***.
  
    EXPECT_EQ(resultL1L2.evicted_addressL2,LessUsedTagL2); //revisa que sacara al tag con mayor  valor de reemplazo en L2

 // 5. Check replacement policy values in L1/L2 were updated properly.
  for (int i = 0; i < associativityL1 -1 ; i++)
  {
    if (cacheL1[i].tag == adress_AL1)
    {
      EXPECT_EQ(cacheL1[i].rp_value, 0); // Revisa que ponga correctamente el valor de reemplazo en L1
    }
  }   

    for (int i = 0; i < associativityL2 -1 ; i++)
  {
    if (cacheL2[i].tag == adress_AL2)
    {
      EXPECT_EQ(cacheL2[i].rp_value, 0); // Revisa que ponga correctamente el valor de reemplazo en L2
    }
  }   

  // 6. Check dirty bit value of AddressA was changed accordingly with the operation performed
 
    for (int i = 0; i < associativityL2 -1 ; i++)
  {
    if (cacheL2[i].tag == adress_AL2 && LS == true)
    {
      EXPECT_EQ(cacheL2[i].dirty, 1); // Revisa que si fue un store, el dirty este en 1
    }
  }  
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//-----CACHE_MULTI-NIVEL: L1 miss / L2 hit 
/*
1. Choose a random associativity
2. Choose a random address (AddressA)
3. Fill a l1 cache line with random addresses, making sure AddressA is not added.
4. Fill a l2 cache line with random addresses, making sure AddressA is added.
5. Read or Write (choose the operation randomly) AddressA.
*/

TEST(L2, miss_hit){

  int associativityL1,associativityL2;
  int adress_AL1, adress_AL2;

/////////////////////      1. Choose a random associativity      ///////////////////
  associativityL1 = 1 << (rand()%4);  // associativity puede ser 1, 2 , 4, 8 
  associativityL2 = associativityL1 << 1; //Es el doble de la asociatividad de L1
  
/////////////////////      2. Choose a random address (AddressA)      ///////////////////
  adress_AL1 = rand()%4096; // tag random
  adress_AL2 >> 1;
  
  struct entry cacheL1[associativityL1];  // Set de L1
  struct entry cacheL2[associativityL2];  //Set de L2
  struct operation_result_L2 resultL1L2 = {};

  int randomTagL1, randomTagL2;
  int LessUsedTagL1;         // Almacenan el tag que tiene el valor de remplazo proximo a salir de la cache

/////////////////////     3. Fill a l1 cache line with random addresses, making sure AddressA is not added.     ///////////////////
  
  for (int j =  0; j < associativityL1; j++) {

      do{randomTagL1 = rand()%4096; }
      while(randomTagL1 == adress_AL1);

      if(j==0){LessUsedTagL1=randomTagL1;}         // Guarda el tag que va a tener valor de reemplazo asociatividad-1

      cacheL1[j].valid = false;
      cacheL1[j].tag = randomTagL1;                               // Llenando L1
      cacheL1[j].dirty = 0;
      cacheL1[j].rp_value = associativityL1-1-j;
    }  

/////////////////////     4. Fill a l2 cache line with random addresses, making sure AddressA is added.     ///////////////////
  

  for (int i =  0; i < associativityL2; i++) {

      cacheL2[i].valid = false;
      cacheL2[i].tag = randomTagL2;                               // Llenando L2
      cacheL2[i].dirty = 0;
      cacheL2[i].rp_value = associativityL2-1-i;

      if(i == 0){cacheL2[i].tag = adress_AL2;}   // Guarda el valor del tag A en L2 para forzar un hit
    } 


/////////////////////     5. Read or Write (choose the operation randomly) AddressA.     ///////////////////
   
    bool LS = rand()%2;                               // Escritura o lectura Random

    lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL2,associativityL1,LS,cacheL1,cacheL2,&resultL1L2,0);  
       

    /*
  1. Check operation result in L1 is a miss.
  2. Check operation result in L2 is a hit.
  3. Check LRU data in L1 line is evicted**.
  4. Check replacement policy values in L1/L2 were updated properly.
  5. Check dirty bit value of AddressA was changed accordingly with the operation performed
 */

  
  // 1. Check operation result in L1 is a miss
    EXPECT_EQ(resultL1L2.MissL1, 1);  //revisa que fuera un miss en L1
  
  // 2. Check operation result in L2 is a miss 
 
    EXPECT_EQ(resultL1L2.HitL2,1); //revisa que fuera un hit en L2
  
 // 3. Check LRU data in L1 line is evicted**.

    EXPECT_EQ(resultL1L2.evicted_addressL1,LessUsedTagL1); //revisa que sacara al tag con mayor  valor de reemplazo en L1
  
  
 // 4. Check replacement policy values in L1/L2 were updated properly.

   for (int i = 0; i < associativityL1 -1 ; i++)
  {
    if (cacheL1[i].tag == adress_AL1)
    {
      EXPECT_EQ(cacheL1[i].rp_value, 0); // Revisa que ponga correctamente el valor de reemplazo en L1
    }
  }   

    for (int i = 0; i < associativityL2 -1 ; i++)
  {
    if (cacheL2[i].tag == adress_AL2)
    {
      EXPECT_EQ(cacheL2[i].rp_value, 0); // Revisa que ponga correctamente el valor de reemplazo en L2
    }
  }  

  // 5. Check dirty bit value of AddressA was changed accordingly with the operation performed
 
    for (int i = 0; i < associativityL2 -1 ; i++)
  {
    if (cacheL2[i].tag == adress_AL2 && LS)
    {
      EXPECT_EQ(cacheL2[i].dirty, 1); // Revisa que el valor del dirty este sucio si se escribio
    }
  }  
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//-----CACHE_MULTI-NIVEL: L1 hit 
/*
1. Choose a random associativity
2. Choose a random address (AddressA)
3. Fill a l1 cache line with random addresses, making sure AddressA is added.
4. Fill a l2 cache line with random addresses, making sure AddressA is added.
5. Read or Write (choose the operation randomly) AddressA. Check
*/

TEST(L2,hit){

  int associativityL1,associativityL2;
  int adress_AL1, adress_AL2;

/////////////////////      1. Choose a random associativity      ///////////////////
  associativityL1 = 1 << (rand()%4);  // associativity puede ser 1, 2 , 4, 8 
  associativityL2 = associativityL1 << 1; //Es el doble de la asociatividad de L1
  
/////////////////////      2. Choose a random address (AddressA)      ///////////////////
  adress_AL1 = rand()%4096; // tag random
  adress_AL2 >> 1;
  
  struct entry cacheL1[associativityL1];  // Set de L1
  struct entry cacheL2[associativityL2];  //Set de L2
  struct operation_result_L2 resultL1L2 = {};

  int randomTagL1, randomTagL2;
  
/////////////////////     3. Fill a l1 cache line with random addresses, making sure AddressA is added.     ///////////////////
  
  for (int j =  0; j < associativityL1; j++) {

      do{randomTagL1 = rand()%4096; }
      while(randomTagL1 == adress_AL1);

      cacheL1[j].valid = false;
      cacheL1[j].tag = randomTagL1;                               // Llenando L1
      cacheL1[j].dirty = 0;
      cacheL1[j].rp_value = associativityL1-1-j;

      if(j == 0){cacheL1[j].tag = adress_AL1;}   // Guarda el valor del tag A en L1 para forzar un hit

    }  

/////////////////////     4. Fill a l2 cache line with random addresses, making sure AddressA is added.     ///////////////////
  

  for (int i =  0; i < associativityL2; i++) {

      cacheL2[i].valid = false;
      cacheL2[i].tag = randomTagL2;                               // Llenando L2
      cacheL2[i].dirty = 0;
      cacheL2[i].rp_value = associativityL2-1-i;

      if(i == 0){cacheL2[i].tag = adress_AL2;}   // Guarda el valor del tag A en L2 para forzar un hit
    } 


/////////////////////     5. Read or Write (choose the operation randomly) AddressA.     ///////////////////
   
    bool LS = rand()%2;                               // Escritura o lectura Random

    lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL2,associativityL1,LS,cacheL1,cacheL2,&resultL1L2,0);  
       

    /*
  1. Check operation result in L1 is a hit
  2. Check LRU data in L1 line is evicted**.
  3. Check replacement policy values in L1/L2 were updated properly.
  4. Check dirty bit value of AddressA was changed accordingly with the operation performed
 */

  
  // 1. Check operation result in L1 is a miss
    EXPECT_EQ(resultL1L2.HitL1, 1);  //revisa que fuera un Hit en L1
  
  // 2. Check LRU data in L1 line is evicted**.
 
    EXPECT_EQ(resultL1L2.evicted_addressL1,0); //revisa lo que sale de L1, en este caso como es un  hit, no sale nada
  
 // 3. Check replacement policy values in L1/L2 were updated properly.

   for (int i = 0; i < associativityL1 -1 ; i++)
  {
    if (cacheL1[i].tag == adress_AL1)
    {
      EXPECT_EQ(cacheL1[i].rp_value, 0); // Revisa que ponga correctamente el valor de reemplazo en L1
    }
  }   

    for (int i = 0; i < associativityL2 -1 ; i++)
  {
    if (cacheL2[i].tag == adress_AL2)
    {
      EXPECT_EQ(cacheL2[i].rp_value, 0); // Revisa que ponga correctamente el valor de reemplazo en L2
    }
  }  

  // 5. Check dirty bit value of AddressA was changed accordingly with the operation performed
 
    for (int i = 0; i < associativityL2 -1 ; i++)
  {
    if (cacheL2[i].tag == adress_AL2 && LS)
    {
      EXPECT_EQ(cacheL2[i].dirty, 1); // Revisa que el valor de reemplazo del dato que hizo hit sea 0
    }
  }  



   // eliminando memoria dinamica
  delete &resultL1L2;
  for(int i = 0; i < associativityL1; i++)
  {
    delete[] &cacheL1[i];
  }
  delete[] cacheL1; 
  
}
  
  
 






/* 
 * Gtest main function: Generates random seed, if not provided,
 * parses DEBUG flag, and execute the test suite
 */
int main(int argc, char **argv) {
  int argc_to_pass = 0;
  char **argv_to_pass = NULL; 
  int seed = 0;

  /* Generate seed */
  seed = time(NULL) & 0xffff;

  /* Parse arguments looking if random seed was provided */
  argv_to_pass = (char **)calloc(argc + 1, sizeof(char *));
  
  for (int i = 0; i < argc; i++){
    std::string arg = std::string(argv[i]);

    if (!arg.compare(0, 20, "--gtest_random_seed=")){
      seed = atoi(arg.substr(20).c_str());
      continue;
    }
    argv_to_pass[argc_to_pass] = strdup(arg.c_str());
    argc_to_pass++;
  }

  /* Init Gtest */
  ::testing::GTEST_FLAG(random_seed) = seed;
  testing::InitGoogleTest(&argc, argv_to_pass);

  /* Print seed for debug */
  printf(YEL "Random seed %d \n",seed);
  srand(seed);

  /* Parse for debug env variable */
  get_env_var("TEST_DEBUG", &debug_on);

  /* Execute test */
  return RUN_ALL_TESTS();
  
  /* Free memory */
  free(argv_to_pass);

  return 0;
}
