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
 */

TEST(MESI_MSI, ESTADO_S){

  int associativityL1, associativityL2;
  int adress_AL1;

//--    1. Choose a random associativity    
  associativityL1 = 1 << (rand()%4);  // associativity puede ser 1, 2 , 4, 8 
  associativityL2 = associativityL1 << 1;

//--    2. Choose a random opt  
  int opt = rand()%2;
//-- Choose a random address (AddressA)    
  adress_AL1 = rand()%4096; // tag random

  struct entry C1_L1[associativityL1];  // Set de L1
  struct entry C2_L1[associativityL1];  // Set de L2
  struct entry cacheL2[associativityL2];  //Set de L2
  struct operation_result_L2 resultL1L2 = {};

  int randomTagL1;
  bool C2_tiene_A = rand()%2; // 0 no lo tiene, 1 si

//-- LLenar el set de C1 y C2 si correspone, asegurando que el dato A este en C1 y en C2 tal vez, ambos en estado S
  for (int j =  0; j < associativityL1; j++) {

      do{randomTagL1 = rand()%4096; }
      while(randomTagL1 == adress_AL1);

      C1_L1[j].valid = true;
      C1_L1[j].tag = randomTagL1;                               // Llenando L1
      C1_L1[j].dirty = 0;
      C1_L1[j].rp_value = associativityL1-1-j;

      C2_L1[j].valid = true;
      C2_L1[j].tag = randomTagL1;                               // Llenando L1
      C2_L1[j].dirty = 0;
      C2_L1[j].rp_value = associativityL1-1-j;
      if(j == 0) // Guarda el valor del tag A en L1 para forzar un hit
      {
        C1_L1[j].tag = adress_AL1;
        C1_L1[j].state = SHARED;
        if(C2_tiene_A)
        {
          C2_L1[j].tag = adress_AL1; 
          C2_L1[j].state = SHARED;
        } 
      }   
    }  

  // Forzar un read o write
  bool LS = rand()%2;                               // Escritura o lectura Random

  int hit_C1 = resultL1L2.Hit_L1_C1;
  int hit_C2 = resultL1L2.Hit_L1_C2;
  opt
  lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL2,associativityL1,LS,C1_L1,cacheL2,&resultL1L2,0);  
       

////-- CHECKS
  if (!LS) // read
  {
    EXPECT_EQ(C1_L1[0].state, SHARED);
    EXPECT_EQ(hit_C1  + 1, resultL1L2.Hit_L1_C1);
    if (C2_tiene_A) // Si el dato esta en C2
    {
      EXPECT_EQ(C2_L1[associativityL1-1].state, SHARED);
      EXPECT_EQ(hit_C2, resultL1L2.Hit_L1_C2);
    }
  }

  if (LS) // store
  {
    EXPECT_EQ(C1_L1[0].state, MODIFIED); 
    EXPECT_EQ(hit_C1 + 1, resultL1L2.Hit_L1_C1 );
    if (C2_tiene_A) // Si el dato esta en C2
    {
      EXPECT_EQ(C2_L1[associativityL1-1].state, INVALID);
      EXPECT_EQ(hit_C2, resultL1L2.Hit_L1_C2);
    }
  } 
}

TEST(MESI_MSI, ESTADO_M){

  int associativityL1, associativityL2;
  int adress_AL1;

//--    1. Choose a random associativity    
  associativityL1 = 1 << (rand()%4);  // associativity puede ser 1, 2 , 4, 8 
  associativityL2 = associativityL1 << 1;

//--    2. Choose a random opt  
  int opt = rand()%2;

//-- Choose a random address (AddressA)    
  adress_AL1 = rand()%4096; // tag random

  struct entry C1_L1[associativityL1];  // Set de L1
  struct entry C2_L1[associativityL1];  // Set de L2
  struct entry cacheL2[associativityL2];  //Set de L2
  struct operation_result_L2 resultL1L2 = {};

  int randomTagL1;
  bool C2_tiene_A = rand()%2; // 0 no lo tiene, 1 si

//-- LLenar el set de C1 y C2 si correspone, asegurando que el dato A este en C1 y en C2 tal vez, ambos en estado S
  for (int j =  0; j < associativityL1; j++) {

      do{randomTagL1 = rand()%4096; }
      while(randomTagL1 == adress_AL1);

      C1_L1[j].valid = true;
      C1_L1[j].tag = randomTagL1;                               // Llenando L1
      C1_L1[j].dirty = 0;
      C1_L1[j].rp_value = associativityL1-1-j;

      C2_L1[j].valid = true;
      C2_L1[j].tag = randomTagL1;                               // Llenando L1
      C2_L1[j].dirty = 0;
      C2_L1[j].rp_value = associativityL1-1-j;
      if(j == 0) // Guarda el valor del tag A en L1 para forzar un hit
      {
        C1_L1[j].tag = adress_AL1;
        C1_L1[j].state = MODIFIED;
        if(C2_tiene_A)
        {
          C2_L1[j].tag = adress_AL1; 
          C2_L1[j].state = INVALID;
        } 
      }   
    }  

  // Forzar un read o write
  bool LS = rand()%2;                               // Escritura o lectura Random

  int hit_C1 = resultL1L2.Hit_L1_C1;
  int CI_C2 = resultL1L2.Coherence_Inv_C2;
  opt
  lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL2,associativityL1,LS,C1_L1,cacheL2,&resultL1L2,0);  
       

////-- CHECKS
  if (!LS) // read
  {
    EXPECT_EQ(C1_L1[0].state, MODIFIED);
    EXPECT_EQ(hit_C1  + 1, resultL1L2.Hit_L1_C1);
    if (C2_tiene_A) // Si el dato esta en C2
    {
      EXPECT_EQ(C2_L1[associativityL1-1].state, INVALID);
      EXPECT_EQ(CI_C2 + 1, resultL1L2.Coherence_Inv_C2);
    }
  }

  if (LS) // store
  {
    EXPECT_EQ(C1_L1[0].state, MODIFIED); 
    EXPECT_EQ(hit_C1  + 1, resultL1L2.Hit_L1_C1);
    if (C2_tiene_A) // Si el dato esta en C2
    {
      EXPECT_EQ(C2_L1[associativityL1-1].state, INVALID);
      EXPECT_EQ(hit_C2, resultL1L2.Hit_L1_C2);
    }
  } 
}




TEST(MESI, ESTADO_E){

  int associativityL1, associativityL2;
  int adress_AL1;

//--    1. Choose a random associativity    
  associativityL1 = 1 << (rand()%4);  // associativity puede ser 1, 2 , 4, 8 
  associativityL2 = associativityL1 << 1;

//-- Choose a random address (AddressA)    
  adress_AL1 = rand()%4096; // tag random

  struct entry C1_L1[associativityL1];  // Set de L1
  struct entry C2_L1[associativityL1];  // Set de L2
  struct entry cacheL2[associativityL2];  //Set de L2
  struct operation_result_L2 resultL1L2 = {};

  int randomTagL1;
  bool C2_tiene_A = 0; // 0 no lo tiene, 1 si

//-- LLenar el set de C1 y C2 si correspone, asegurando que el dato A este en C1 y en C2 tal vez, ambos en estado S
  for (int j =  0; j < associativityL1; j++) {

      do{randomTagL1 = rand()%4096; }
      while(randomTagL1 == adress_AL1);

      C1_L1[j].valid = true;
      C1_L1[j].tag = randomTagL1;                               // Llenando L1
      C1_L1[j].dirty = 0;
      C1_L1[j].rp_value = associativityL1-1-j;

      C2_L1[j].valid = true;
      C2_L1[j].tag = randomTagL1;                               // Llenando L1
      C2_L1[j].dirty = 0;
      C2_L1[j].rp_value = associativityL1-1-j;
      if(j == 0) // Guarda el valor del tag A en L1 para forzar un hit
      {
        C1_L1[j].tag = adress_AL1;
        C1_L1[j].state = EXCLUSIVE;
      }   
    }  

  // Forzar un read o write
  bool LS = rand()%2;                               // Escritura o lectura Random

  int hit_C1 = resultL1L2.Hit_L1_C1;
  int hit_C2 = resultL1L2.Hit_L1_C2;
  lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL2,associativityL1,LS,C1_L1,cacheL2,&resultL1L2,0);  
       

////-- CHECKS
  if (!LS) // read
  {
    EXPECT_EQ(C1_L1[0].state, EXCLUSIVE); 
    EXPECT_EQ(hit_C1 + 1, resultL1L2.Hit_L1_C1);
  }

  if (LS) // store
  {
    EXPECT_EQ(C1_L1[0].state, MODIFIED); 
    EXPECT_EQ(hit_C1 + 1, resultL1L2.Hit_L1_C1);
  } 
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
  
  //--    3. Choose a random opt  
  int opt = rand()%2;

  struct entry C1_L1[associativityL1];  // Set de L1
  struct entry cacheL2[associativityL2];  //Set de L2
  struct operation_result_L2 resultL1L2 = {};

  int randomTagL1, randomTagL2;
  int LessUsedTagL1, LessUsedTagL2;          // Almacenan el tag que tiene el valor de remplazo proximo a salir de la cache

/////////////////////     3. Fill a l1 cache line with random addresses, making sure AddressA is not added.     ///////////////////
  
  for (int j =  0; j < associativityL1; j++) {

      do{randomTagL1 = rand()%4096; }
      while(randomTagL1 == adress_AL1);

      C1_L1[j].valid = true;
      C1_L1[j].tag = randomTagL1;                               // Llenando L1
      C1_L1[j].dirty = 0;
      C1_L1[j].rp_value = associativityL1-1-j;
      
      if(j==0){LessUsedTagL1=randomTagL1;}         // Guarda el tag que va a tener valor de reemplazo asociatividad-1
    }  

/////////////////////     4. Fill a l2 cache line with random addresses, making sure AddressA is not added.     ///////////////////
  

  for (int i =  0; i < associativityL2; i++) {

      do{randomTagL2 = rand()%4096; }
      while(randomTagL2 == adress_AL2);

      cacheL2[i].valid = true;
      cacheL2[i].tag = randomTagL2;                               // Llenando L2
      cacheL2[i].dirty = 0;
      cacheL2[i].rp_value = associativityL2-1-i;

      if(i==0){LessUsedTagL2=randomTagL2;}           // Guarda el tag que va a tener valor de reemplazo asociatividad-1
    } 


/////////////////////     5. Read or Write (choose the operation randomly) AddressA.     ///////////////////
   
    bool LS = rand()%2;                               // Escritura o lectura Random
    opt
    lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL2,associativityL1,LS,C1_L1,cacheL2,&resultL1L2,0);  
       
  
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
    if (C1_L1[i].tag == adress_AL1)
    {
      EXPECT_EQ(C1_L1[i].rp_value, 0); // Revisa que ponga correctamente el valor de reemplazo en L1
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
  
  struct entry C1_L1[associativityL1];  // Set de L1
  struct entry cacheL2[associativityL2];  //Set de L2
  struct operation_result_L2 resultL1L2 = {};

  int randomTagL1, randomTagL2;
  int LessUsedTagL1;         // Almacenan el tag que tiene el valor de remplazo proximo a salir de la cache

/////////////////////     3. Fill a l1 cache line with random addresses, making sure AddressA is not added.     ///////////////////
  
  for (int j =  0; j < associativityL1; j++) {

      do{randomTagL1 = rand()%4096; }
      while(randomTagL1 == adress_AL1);

      if(j==0){LessUsedTagL1=randomTagL1;}         // Guarda el tag que va a tener valor de reemplazo asociatividad-1

      C1_L1[j].valid = true;
      C1_L1[j].tag = randomTagL1;                               // Llenando L1
      C1_L1[j].dirty = 0;
      C1_L1[j].rp_value = associativityL1-1-j;
    }  

/////////////////////     4. Fill a l2 cache line with random addresses, making sure AddressA is added.     ///////////////////
  

  for (int i =  0; i < associativityL2; i++) {

      do{randomTagL2 = rand()%4096; }
      while(randomTagL2 == adress_AL2);
      
      cacheL2[i].valid = true;
      cacheL2[i].tag = randomTagL2;                               // Llenando L2
      cacheL2[i].dirty = 0;
      cacheL2[i].rp_value = associativityL2-1-i;

      if(i == 0){cacheL2[i].tag = adress_AL2;}   // Guarda el valor del tag A en L2 para forzar un hit
    } 


/////////////////////     5. Read or Write (choose the operation randomly) AddressA.     ///////////////////
   
    bool LS = rand()%2;                               // Escritura o lectura Random

    lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL2,associativityL1,LS,C1_L1,cacheL2,&resultL1L2,0);  
       

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

   for (int i = 0; i < associativityL1; i++)
  {
    if (C1_L1[i].tag == adress_AL1)
    {
      EXPECT_EQ(C1_L1[i].rp_value, 0); // Revisa que ponga correctamente el valor de reemplazo en L1
    }
  }   

    for (int i = 0; i < associativityL2; i++)
  {
    if (cacheL2[i].tag == adress_AL2)
    {
      EXPECT_EQ(cacheL2[i].rp_value, 0); // Revisa que ponga correctamente el valor de reemplazo en L2
    }
  }  

  // 5. Check dirty bit value of AddressA was changed accordingly with the operation performed
 
    for (int i = 0; i < associativityL2; i++)
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
  
  struct entry C1_L1[associativityL1];  // Set de L1
  struct entry cacheL2[associativityL2];  //Set de L2
  struct operation_result_L2 resultL1L2 = {};

  int randomTagL1, randomTagL2;
  
/////////////////////     3. Fill a l1 cache line with random addresses, making sure AddressA is added.     ///////////////////
  
  for (int j =  0; j < associativityL1; j++) {

      do{randomTagL1 = rand()%4096; }
      while(randomTagL1 == adress_AL1);

      C1_L1[j].valid = true;
      C1_L1[j].tag = randomTagL1;                               // Llenando L1
      C1_L1[j].dirty = 0;
      C1_L1[j].rp_value = associativityL1-1-j;

      if(j == 0){C1_L1[j].tag = adress_AL1;}   // Guarda el valor del tag A en L1 para forzar un hit

    }  

/////////////////////     4. Fill a l2 cache line with random addresses, making sure AddressA is added.     ///////////////////
  

  for (int i =  0; i < associativityL2; i++) {

      do{randomTagL2 = rand()%4096; }
      while(randomTagL2 == adress_AL2);

      cacheL2[i].valid = true;
      cacheL2[i].tag = randomTagL2;                               // Llenando L2
      cacheL2[i].dirty = 0;
      cacheL2[i].rp_value = associativityL2-1-i;

      if(i == 0){cacheL2[i].tag = adress_AL2;}   // Guarda el valor del tag A en L2 para forzar un hit
    } 


/////////////////////     5. Read or Write (choose the operation randomly) AddressA.     ///////////////////
   
    bool LS = rand()%2;                               // Escritura o lectura Random

    lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL2,associativityL1,LS,C1_L1,cacheL2,&resultL1L2,0);  
       

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

   for (int i = 0; i < associativityL1 ; i++)
  {
    if (C1_L1[i].tag == adress_AL1)
    {
      EXPECT_EQ(C1_L1[i].rp_value, 0); // Revisa que ponga correctamente el valor de reemplazo en L1
    }
  }   

    for (int j = 0; j < associativityL2; j++)
  {
    if (cacheL2[j].tag == adress_AL2)
    {
      EXPECT_EQ(cacheL2[j].rp_value, 0); // Revisa que ponga correctamente el valor de reemplazo en L2
    }
  }  

  // 5. Check dirty bit value of AddressA was changed accordingly with the operation performed
 
    for (int m = 0; m < associativityL2 ; m++)
  {
    if (cacheL2[m].tag == adress_AL2 && LS)
    {
      EXPECT_EQ(cacheL2[m].dirty, 1); // Revisa que el valor de reemplazo del dato que hizo hit sea 0
    }
  }  
 
  
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
