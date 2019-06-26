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
    1. Escoger una asociatividad random
    2. Escoger un protocolo random
    3. Llenar un set de L1\_C1 con asociatividad datos, asegurándose que un dato A está en estado S.
    4. Llenar un set de L1\_C2 y asegurarse de que el dato A tenga una probabilidad del 50\% de estar o no en este Core (si está es con estado S)
    5. Forzar un read o write (random 50\%) del dato A y realizar la Verificación 1.
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

  struct entry C1_L1[associativityL1];  // Set de C1L1
  struct entry C2_L1[associativityL1];  // Set de C2L1
  struct entry cacheL2[associativityL2];  //Set de L2
  struct operation_result_L2 resultL1L2 = {};

  int randomTagL1;
  bool C2_tiene_A = rand()%2; // 0 no lo tiene, 1 si

//-- LLenar el set de C1 y C2 si correspone, asegurando que el dato A este en C1 y en C2 tal vez, ambos en estado S
  for (int j =  0; j < associativityL1; j++) {

      do{randomTagL1 = rand()%4096; }
      while(randomTagL1 == adress_AL1);

      C1_L1[j].valid = true;
      C1_L1[j].tag = randomTagL1;                               // Llenando C1L1
      C1_L1[j].dirty = 0;
      C1_L1[j].rp_value = associativityL1-1-j;

      C2_L1[j].valid = true;
      C2_L1[j].tag = randomTagL1;                               // Llenando C2L1
      C2_L1[j].dirty = 0;
      C2_L1[j].rp_value = associativityL1-1-j;
      if(j == 0) // Guarda el valor del tag A en L1 para forzar un hit
      {
        C1_L1[j].tag = adress_AL1;
        C1_L1[j].state = SHARED;
        if(C2_tiene_A)  // Guarda el valor en L1C2
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

  // funcion que hace todo
  lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL1,associativityL1,LS,C1_L1,C2_L1,cp,cacheL2,&resultL1L2,false,0);      


/* CHECKS
    Verificación 1: Si es un read: Verificar que el dato A en C1 se queda en el estado S, y si el dato estaba en C2 revisar 
                    que también esté en S.
                    Si es un write: Verificar que el dato A en C1 pasó a M y si el dato A estaba en el C2 que pasó a I.
    Verificación 2: Verificar que existiera hit segun corresponda y si se pasó al estado I que el C2 regrese
                    un coherency invalidation.
 */

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



/*
  1. Escoger una asociatividad random
  2. Llenar un set de L1\_C1 con asociatividad datos, asegurándose que un dato A está en estado M. 
  3. Llenar un set de L1\_C2 y asegurarse de que el dato A tenga una probabilidad del 50\% 
     de estar o no en este Core (si está es con estado I)
  4. Forzar un read o write (random 50\%) del dato A y realizar la Verificación 1.
 */
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

  struct entry C1_L1[associativityL1];      // Set de L1C1
  struct entry C2_L1[associativityL1];      // Set de L1C2
  struct entry cacheL2[associativityL2];    //Set de L2
  struct operation_result_L2 resultL1L2 = {};

  int randomTagL1;
  bool C2_tiene_A = rand()%2; // 0 no lo tiene, 1 si

//-- LLenar el set de C1 y C2 si correspone, asegurando que el dato A este en C1 y M, de estar en C2 en estado I
  for (int j =  0; j < associativityL1; j++) {

      do{randomTagL1 = rand()%4096; }
      while(randomTagL1 == adress_AL1);

      C1_L1[j].valid = true;
      C1_L1[j].tag = randomTagL1;                               // Llenando L1C1
      C1_L1[j].dirty = 0;
      C1_L1[j].rp_value = associativityL1-1-j;

      C2_L1[j].valid = true;
      C2_L1[j].tag = randomTagL1;                               // Llenando L1C2
      C2_L1[j].dirty = 0;
      C2_L1[j].rp_value = associativityL1-1-j;
      if(j == 0) // Guarda el valor del tag A en L1C1 para forzar un hit
      {
        C1_L1[j].tag = adress_AL1;
        C1_L1[j].state = MODIFIED;
        if(C2_tiene_A)  // Guarda el valor del tag A en L1C2 
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
  
  // Funcion que se encarga de todo
  lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL1,associativityL1,LS,C1_L1,C2_L1,cp,cacheL2,&resultL1L2,false,0);

/* CHECKS
     Verificación 1: Verificar que en cualquier caso el dato se mantiene en M,
                     y verificar si el dato estaba en C2 que está en invalido y 
                     que ocurriera un coherency invalidation.

     Verificación 2: que existiera hit segun corresponda y que los datos tengan el 
                     valor de remplazo adecuad0.
 */
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
      EXPECT_EQ(C2_L1[0].state, INVALID);
      EXPECT_EQ(C2_L1[0].rp_value,associativityL1-1);
    }
  } 
}




/*
    1. Escoger una asociatividad random
    2. Llenar un set de L1\_C1 con asociatividad datos, asegurándose que un dato A está en estado E. 
    4. Llenar un set de L1\_C2 y asegurarse de que el dato A no esté.
    3. Forzar un read o write (random 50\%) del dato A y realizar la Verificación 1.
 */

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

//-- LLenar el set de C1 y C2 si correspone, asegurando que el dato A este en C1 y en C2 tal vez, ambos en estado EXCLUSIVE
  for (int j =  0; j < associativityL1; j++) {

      do{randomTagL1 = rand()%4096; }
      while(randomTagL1 == adress_AL1);

      C1_L1[j].valid = true;
      C1_L1[j].tag = randomTagL1;                               // Llenando L1C1
      C1_L1[j].dirty = 0;
      C1_L1[j].rp_value = associativityL1-1-j;

      C2_L1[j].valid = true;
      C2_L1[j].tag = randomTagL1;                               // Llenando L1C2
      C2_L1[j].dirty = 0;
      C2_L1[j].rp_value = associativityL1-1-j;
      if(j == 0) // Guarda el valor del tag A en L1C1 para forzar un hit
      {
        C1_L1[j].tag = adress_AL1;
        C1_L1[j].state = EXCLUSIVE;
      }   
    }  

  // Forzar un read o write
  bool LS = rand()%2;                               // Escritura o lectura Random

  int hit_C1 = resultL1L2.Hit_L1_C1;
  int hit_C2 = resultL1L2.Hit_L1_C2;

  // Funcion que se encarga de todo
  lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL1,associativityL1,LS,C1_L1,C2_L1,1,cacheL2,&resultL1L2,false,0);
  

/* CHECKS
  Verificación 1: Si es un read:
                                 Verificar que el dato A en C1 se quede en al estado E.
                  Si es un write:
                                 Verificar que el dato A pasa al estado M

  Verificación 2: Verificar que existiera hit segun corresponda y que la política de remplazo del dato A es la adecuada.
 */
  if (!LS) // read
  {
    EXPECT_EQ(C1_L1[0].state, EXCLUSIVE); 
    EXPECT_EQ(hit_C1 + 1, resultL1L2.Hit_L1_C1);
     EXPECT_EQ(C1_L1[0].rp_value,0);
  }

  if (LS) // write
  {
    EXPECT_EQ(C1_L1[0].state, MODIFIED); 
    EXPECT_EQ(hit_C1 + 1, resultL1L2.Hit_L1_C1);
     EXPECT_EQ(C1_L1[0].rp_value,0);
  } 
}


/*
    1.  Escoger una asociatividad random.
    2.  Escoger una optimización random.
    3.  Llenar un bloque de la cache L2 con un dato A asegurándose que esté en estado S
    4.  Llenar un set de L1 C1 con datos distintos de A y L1 C2 con una probabilidad del 50\% de tener el dato A.
    5.  Forzar un load o store (50\%) y realizar verificación 1 
 */
TEST(L1_L2, miss_hit){

  int associativityL1, associativityL2;
  int adress_AL1;

//--    1. Choose a random associativity    
  associativityL1 = 1 << (rand()%4);  // associativity puede ser 1, 2 , 4, 8 
  associativityL2 = associativityL1 << 1;

//--    2. Choose a random opt  
  int cp = rand()%2;

//-- Choose a random address (AddressA)    
  adress_AL1 = rand()%4096; // tag random

  struct entry C1_L1[associativityL1];  // Set de L1C1
  struct entry C2_L1[associativityL1];  // Set de L2C2
  struct entry cacheL2[associativityL2];  //Set de L2
  struct operation_result_L2 resultL1L2 = {};

  int randomTagL1;
  bool C2_tiene_A = rand()%2; // 0 no lo tiene, 1 si

//-- LLenar el set de C1 y C2 si correspone, asegurando que el dato A no este en C1 y en C2 tal vez
  for (int j =  0; j < associativityL1; j++) {

      do{randomTagL1 = rand()%4096; }
      while(randomTagL1 == adress_AL1);

      C1_L1[j].valid = true;
      C1_L1[j].tag = randomTagL1;                               // Llenando L1C1
      C1_L1[j].dirty = 0;
      C1_L1[j].rp_value = associativityL1-1-j;

      C2_L1[j].valid = true;
      C2_L1[j].tag = randomTagL1;                               // Llenando L1C2
      C2_L1[j].dirty = 0;
      C2_L1[j].rp_value = associativityL1-1-j;
      if(j == 0) // Guarda el valor del tag A en L1C2 para forzar un hit
      {
        if(C2_tiene_A)
        {
          C2_L1[j].tag = adress_AL1; 
          C2_L1[j].state = SHARED;
        } 
      }   
    }  

  for (int j =  0; j < associativityL2; j++) {

      do{randomTagL1 = rand()%4096; }
      while(randomTagL1 == adress_AL1);

      cacheL2[j].valid = true;
      cacheL2[j].tag = randomTagL1;                               // Llenando L2
      cacheL2[j].dirty = 0;
      cacheL2[j].rp_value = associativityL2-1-j;

      if(j == 0) // Guarda el valor del tag A en L2 para forzar un hit
      {
        cacheL2[j].tag = adress_AL1;
        cacheL2[j].state = SHARED;
      }   
    }  


  // Forzar un read o write
  bool LS = rand()%2;                               // Escritura o lectura Random

  int miss_C1 = resultL1L2.Miss_L1_C1;
  int hit_L2 = resultL1L2.Hit_L2;
  int CI_C2 = resultL1L2.Coherence_Inv_C2;
  //debug
  cout << "cp = " << cp << ".   LS= "  << LS << "   DATO EN C2? = " << C2_tiene_A << "   asociatividad = " << associativityL1 << endl;
  lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL1,associativityL1,LS,C1_L1,C2_L1,cp,cacheL2,&resultL1L2,false,0);

  /*
    Checks
    Verificación 1: Si C2 tiene el dato:  
                                        LOAD:  Verificar que el dato está en S en C1 y C2
                                        STORE: Verificar que el dato en C2 pasa a invalido, y que C1 tiene el dato M
                    Si C2 no tiene el dato:  
                                        LOAD: Verificar que el dato está en S en C1 
                                        STORE: Verificar que el dato está en M en C1
   */
  EXPECT_EQ( resultL1L2.Hit_L2, hit_L2 + 1);
  EXPECT_EQ( resultL1L2.Miss_L1_C1, miss_C1 + 1);
  if (!LS) // read
  {
    for (int i = 0; i < associativityL1; i++)
    {
      if (C1_L1[i].tag == adress_AL1)
      {
          if (C2_tiene_A){EXPECT_EQ(C1_L1[i].state, SHARED); }
          if (!C2_tiene_A & cp){EXPECT_EQ(C1_L1[i].state, EXCLUSIVE); }
      }
      if (C2_tiene_A) // Si el dato esta en C2
      {
        if (C2_L1[i].tag == adress_AL1)
        {
          EXPECT_EQ(C2_L1[i].state, SHARED);
        }
      }    
    }
  }
  if (LS) // WRITE
  {
    for (int i = 0; i < associativityL1; i++)
    {
      if (C1_L1[i].tag == adress_AL1)
      {
        EXPECT_EQ(C1_L1[i].state, MODIFIED);
      }
      if (C2_tiene_A) // Si el dato esta en C2
      {
        if (C2_L1[i].tag == adress_AL1)
        {
          EXPECT_EQ(C2_L1[i].state, INVALID);
          EXPECT_EQ(resultL1L2.Coherence_Inv_C2, CI_C2 + 1);
        }
      }    
    }
  }
 
}


/*
    1. Escoger una asociatividad random.
    2. Escoger una optimización random.
    3. Llenar un bloque de la cache L2 asegurándose que el dato A no esté
    4. Llenar un set de L1 C1 con datos distintos de A y L1 C2 con una probabilidad del 50\% de tener el dato A en S.
    5. Forzar un load o store (50\%) y realizar verificación 1 
 */


TEST(L1_L2, miss_miss){

  int associativityL1, associativityL2;
  int adress_AL1;

//--    1. Choose a random associativity    
  associativityL1 = 1 << (rand()%4);  // associativity puede ser 1, 2 , 4, 8 
  associativityL2 = associativityL1 << 1;

//--    2. Choose a random opt  
  int cp = 1;

//-- Choose a random address (AddressA)    
  adress_AL1 = rand()%4096; // tag random

  struct entry C1_L1[associativityL1];  // Set de L1C1
  struct entry C2_L1[associativityL1];  // Set de L2C2
  struct entry cacheL2[associativityL2];  //Set de L2
  struct operation_result_L2 resultL1L2 = {};

  int randomTagL1;
  bool C2_tiene_A = rand()%2; // 0 no lo tiene, 1 si

//-- LLenar el set de C1 y C2 si correspone, asegurando que el dato no A este en C1 y en C2 tal vez,
  for (int j =  0; j < associativityL1; j++) {

      do{randomTagL1 = rand()%4096; }
      while(randomTagL1 == adress_AL1);

      C1_L1[j].valid = true;
      C1_L1[j].tag = randomTagL1;                               // Llenando L1C1
      C1_L1[j].dirty = 0;
      C1_L1[j].rp_value = associativityL1-1-j;

      C2_L1[j].valid = true;
      C2_L1[j].tag = randomTagL1;                               // Llenando L1C2
      C2_L1[j].dirty = 0;
      C2_L1[j].rp_value = associativityL1-1-j;
      if(j == 0) // Guarda el valor del tag A en L1 para forzar un hit
      {
        if(C2_tiene_A)
        {
          C2_L1[j].tag = adress_AL1; 
          C2_L1[j].state = SHARED;
        } 
      }   
    }  

  for (int j =  0; j < associativityL2; j++) {

      do{randomTagL1 = rand()%4096; }
      while(randomTagL1 == adress_AL1);

      cacheL2[j].valid = true;
      cacheL2[j].tag = randomTagL1;                               // Llenando L2
      cacheL2[j].dirty = 0;
      cacheL2[j].rp_value = associativityL2-1-j;
    }  


  // Forzar un read o write
  bool LS = rand()%2;                               // Escritura o lectura Random

  int miss_C1 = resultL1L2.Miss_L1_C1;
  int miss_L2 = resultL1L2.Miss_L2;
  int CI_C2 = resultL1L2.Coherence_Inv_C2;
  //debug
  cout << "cp = " << cp << ".   LS= "  << LS << "   DATO EN C2? = " << C2_tiene_A << "   asociatividad = " << associativityL1 << endl;
  
  //  funcion que hace todo
  lru_L1_L2_replacement_policy(0,adress_AL1,0,adress_AL1,associativityL1,LS,C1_L1,C2_L1,cp,cacheL2,&resultL1L2,false,0);

  /*
    Checks
    Verificación 1: Si C2 tiene el dato:
                                        LOAD: Verificar que el dato en C2 se queda en S, también en C1
                                        STORE: verificar que C2 tenga el dato I y C1 en M
                    Si C2 no tiene el dato: 
                                        LOAD: El dato entra en E a C1. 
                                        STORE: El dato entra a M a C1
   */
  EXPECT_EQ( resultL1L2.Miss_L2, miss_L2 + 1);
  EXPECT_EQ( resultL1L2.Miss_L1_C1, miss_C1 + 1);
  if (!LS) // read
  {
    for (int i = 0; i < associativityL1; i++)
    {
      if (C2_tiene_A)  // Si C2 tiene el dato:
      {
        if (C2_L1[i].tag == adress_AL1 & C2_L1[i].valid)
        {
          EXPECT_EQ(C2_L1[i].state, SHARED);
        }
        if (C1_L1[i].tag == adress_AL1 &  C1_L1[i].valid)
        {
          EXPECT_EQ(C1_L1[i].state, SHARED);
        }
      }
      else // Si C2 no tiene el dato: 
      {
        if (C1_L1[i].tag == adress_AL1 &  C1_L1[i].valid)
        {
          EXPECT_EQ(C1_L1[i].state, EXCLUSIVE);
        }
      }    
    }
  }

  if (LS) // store
  {
    for (int i = 0; i < associativityL1; i++)
    {
      if (C2_tiene_A)  // Si C2 tiene el dato:
      {
        if (C2_L1[i].tag == adress_AL1  & C2_L1[i].valid)
        {
          EXPECT_EQ(C2_L1[i].state, INVALID);
        }
        if (C1_L1[i].tag == adress_AL1 &  C1_L1[i].valid)
        {
          EXPECT_EQ(C1_L1[i].state, MODIFIED);
        }
      }
      else // Si C2 no tiene el dato: 
      {
        if (C1_L1[i].tag == adress_AL1  & C1_L1[i].valid )
        {
          EXPECT_EQ(C1_L1[i].state, MODIFIED);
        }
      }    
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
