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
  int cp = rand()%2;
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
    int CI_C2 = resultL1L2.Coherence_Inv_C2;
  lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL1,associativityL1,LS,C1_L1,C2_L1,cp,cacheL2,&resultL1L2,false,0);      

////-- CHECKS
  if (!LS) // read
  {
    EXPECT_EQ(C1_L1[0].state, SHARED);
    EXPECT_EQ(hit_C1  + 1, resultL1L2.Hit_L1_C1);
     EXPECT_EQ(C1_L1[0].rp_value,0);
    if (C2_tiene_A) // Si el dato esta en C2
    {
      EXPECT_EQ(C2_L1[0].state, SHARED);
      EXPECT_EQ(hit_C2, resultL1L2.Hit_L1_C2);
      EXPECT_EQ(C2_L1[0].rp_value,associativityL1-1);
    }
  }

  if (LS) // store
  {
    EXPECT_EQ(C1_L1[0].state, MODIFIED); 
    EXPECT_EQ(hit_C1 + 1, resultL1L2.Hit_L1_C1 );
     EXPECT_EQ(C1_L1[0].rp_value,0);
    if (C2_tiene_A) // Si el dato esta en C2
    {
      EXPECT_EQ(C2_L1[0].state, INVALID);
      EXPECT_EQ(hit_C2, resultL1L2.Hit_L1_C2);
      EXPECT_EQ(CI_C2 + 1, resultL1L2.Coherence_Inv_C2);
      EXPECT_EQ(C2_L1[0].rp_value,associativityL1-1);
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
  int cp = rand()%2;

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
  
  lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL1,associativityL1,LS,C1_L1,C2_L1,cp,cacheL2,&resultL1L2,false,0);

////-- CHECKS
  if (!LS) // read
  {
    EXPECT_EQ(C1_L1[0].state, MODIFIED);
    EXPECT_EQ(hit_C1  + 1, resultL1L2.Hit_L1_C1);
    EXPECT_EQ(C1_L1[0].rp_value,0);
    if (C2_tiene_A) // Si el dato esta en C2
    {
      EXPECT_EQ(C2_L1[0].state, INVALID);
      EXPECT_EQ(C2_L1[0].rp_value,associativityL1-1);
    }
  }

  if (LS) // store
  {
    EXPECT_EQ(C1_L1[0].state, MODIFIED); 
    EXPECT_EQ(hit_C1  + 1, resultL1L2.Hit_L1_C1);
    EXPECT_EQ(C1_L1[0].rp_value,0);
    if (C2_tiene_A) // Si el dato esta en C2
    {
     // EXPECT_EQ(CI_C2 + 1, resultL1L2.Coherence_Inv_C2);
      EXPECT_EQ(C2_L1[0].state, INVALID);
      EXPECT_EQ(C2_L1[0].rp_value,associativityL1-1);
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
  lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL1,associativityL1,LS,C1_L1,C2_L1,1,cacheL2,&resultL1L2,false,0);
  

////-- CHECKS
  if (!LS) // read
  {
    EXPECT_EQ(C1_L1[0].state, EXCLUSIVE); 
    EXPECT_EQ(hit_C1 + 1, resultL1L2.Hit_L1_C1);
     EXPECT_EQ(C1_L1[0].rp_value,0);
  }

  if (LS) // store
  {
    EXPECT_EQ(C1_L1[0].state, MODIFIED); 
    EXPECT_EQ(hit_C1 + 1, resultL1L2.Hit_L1_C1);
     EXPECT_EQ(C1_L1[0].rp_value,0);
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
