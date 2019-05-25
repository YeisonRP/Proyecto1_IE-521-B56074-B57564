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
#define YEL   "\x1B[33m"

/* Globals */
int debug_on = 0;

/* Test Helpers */
#define DEBUG(x) if (debug_on) printf("%s\n",#x)
/*
 * TEST1: Verifies miss and hit scenarios for srrip policy
 * 1. Choose a random associativity
 * 2. Fill a cache entry
 * 3. Force a miss load
 * 4. Check  miss_hit_status == MISS_LOAD
 * 5. Force a miss store
 * 6. Check miss_hit_status == MISS_STORE
 * 7. Force a hit read
 * 8. Check miss_hit_status == HIT_READ
 * 9. Force a hit store
 * 10. miss_hit_status == HIT_STORE
 */
TEST(L1cache, hit_miss_srrip){
  int status;
  int i;
  int idx;
  int tag;
  int associativity;
  enum miss_hit_status expected_miss_hit;
  bool loadstore = 1;
  bool debug = 0;
  struct operation_result result = {};

  /* Fill a random cache entry */
  idx = rand()%1024;
  tag = rand()%4096;
  associativity = 1 << (rand()%4);
  if (debug_on) {
    printf("Entry Info\n idx: %d\n tag: %d\n associativity: %d\n",
          idx,
          tag,
          associativity);
  }
 
  struct entry cache_line[associativity];
  /* Check for a miss */
  DEBUG(Checking miss operation);
  for (i = 0 ; i < 2; i++){
    /* Fill cache line */
    for ( i =  0; i < associativity; i++) {
      cache_line[i].valid = true;
      cache_line[i].tag = rand()%4096;
      cache_line[i].dirty = 0;
      cache_line[i].rp_value = (associativity <= 2)? rand()%associativity: 3;
      while (cache_line[i].tag == tag) {
        cache_line[i].tag = rand()%4096;
      }
    }
    loadstore = (bool)i; // en 0 es un Load, en 1 un store
    status = srrip_replacement_policy(idx, 
                                     tag, 
                                     associativity,
                                     loadstore,
                                     cache_line,
                                     &result,
                                     bool(debug_on));
    EXPECT_EQ(status, 0);
    EXPECT_EQ(result.dirty_eviction, 0);
    expected_miss_hit = loadstore ? MISS_STORE: MISS_LOAD;
    EXPECT_EQ(result.miss_hit, expected_miss_hit);
  }
  /*
   * Check for hit: block was replaced in last iteration, if we used the same 
   * tag now we will get a hit
   */
  DEBUG(Checking hit operation);
  for (i = 0 ; i < 2; i++){
    loadstore = (bool)i;
    status = srrip_replacement_policy(idx, 
                                     tag, 
                                     associativity,
                                     loadstore,
                                     cache_line,
                                     &result,
                                     (bool)debug_on);
    EXPECT_EQ(status, 0);
    EXPECT_EQ(result.dirty_eviction, 0);
    expected_miss_hit = loadstore ? HIT_STORE: HIT_LOAD;
    EXPECT_EQ(result.miss_hit, expected_miss_hit);
  }

}

/*
 * TEST2: Verifies miss and hit scenarios for lru policy
 * 1. Choose a random associativity
 * 2. Fill a cache entry
 * 3. Force a miss load
 * 4. Check  miss_hit_status == MISS_LOAD
 * 5. Force a miss store
 * 6. Check miss_hit_status == MISS_STORE
 * 7. Force a hit read
 * 8. Check miss_hit_status == HIT_READ
 * 9. Force a hit store
 * 10. miss_hit_status == HIT_STORE
 */
TEST(L1cache, hit_miss_lru) {
  int status;
  int i;
  int idx;
  int tag;
  int associativity;
  enum miss_hit_status expected_miss_hit;
  bool loadstore = 1;
  bool debug = 0;
  struct operation_result result = {};


  idx = rand()%1024;
  tag = rand()%4096;

  /////////////////////      1. Choose a random associativity      ///////////////////

  associativity = 1 << (rand()%4);  // associativity puede ser 1, 2 , 4, 8 , 16
  if (debug_on) {
    printf("Entry Info\n idx: %d\n tag: %d\n associativity: %d\n",
          idx,
          tag,
          associativity);
  }

  struct entry cache_line[associativity];

  DEBUG(Checking miss operation);

  //iteracion 1 del for: 
  /////////////////////      2. Fill a cache entry      ///////////////////
  /////////////////////      3. Force a miss load      ///////////////////
  /////////////////////      4. Check  miss_hit_status == MISS_LOAD     ///////////////////

  // iteracion 2 del for:
  /////////////////////      2. Fill a cache entry      ///////////////////
  /////////////////////      5. Force a miss store     ///////////////////
  /////////////////////      6. Check miss_hit_status == MISS_STORE     ///////////////////

  for (i = 0 ; i < 2; i++){
    /* Fill cache line */
    for ( i =  0; i < associativity; i++) {
      cache_line[i].valid = true;
      cache_line[i].tag = rand()%4096;
      cache_line[i].dirty = 0;
      cache_line[i].rp_value = i;
      while (cache_line[i].tag == tag) {  // hace que los tag de la cache nunca coincidan con el tag brindado
        cache_line[i].tag = rand()%4096;
      }
    }
    loadstore = (bool)i; // en 0 es un Load, en 1 un store
    status = lru_replacement_policy( idx, 
                                     tag, 
                                     associativity,
                                     loadstore,
                                     cache_line,
                                     &result,
                                     bool(debug_on));

    EXPECT_EQ(status, 0);                                   
    EXPECT_EQ(result.dirty_eviction, 0);                    
    expected_miss_hit = loadstore ? MISS_STORE: MISS_LOAD;  
    EXPECT_EQ(result.miss_hit, expected_miss_hit);          
  }


  //iteracion 1 del for: 
  /////////////////////      7. Force a hit read      ///////////////////
  /////////////////////      8. Check miss_hit_status == HIT_READ      ///////////////////

  // iteracion 2 del for:
  /////////////////////      9. Force a hit store     ///////////////////
  /////////////////////      10. miss_hit_status == HIT_STORE      ///////////////////

  DEBUG(Checking hit operation);
  for (i = 0 ; i < 2; i++){
    loadstore = (bool)i;  // en 0 es un Load, en 1 un store
    status = lru_replacement_policy(idx, 
                                     tag, 
                                     associativity,
                                     loadstore,
                                     cache_line,
                                     &result,
                                     (bool)debug_on);
    EXPECT_EQ(status, 0);
    EXPECT_EQ(result.dirty_eviction, 0);
    expected_miss_hit = loadstore ? HIT_STORE: HIT_LOAD;
    EXPECT_EQ(result.miss_hit, expected_miss_hit);
  }
}

/*
 * TEST3: Verifies replacement policy promotion and eviction
 * 1. Choose a random policy 
 * 2. Choose a random associativity
 * 3. Fill a cache entry
 * 4. Insert a new block A
 * 5. Force a hit on A
 * 6. Check rp_value of block A
 * 7. Keep inserting new blocks until A is evicted
 * 8. Check eviction of block A happen after N new blocks were inserted
 * (where N depends of the number of ways)
 */
TEST(L1cache, promotion){
  int status;
  int i;
  int associativity;
  int tag;
  int idx;
  int policy;
  enum miss_hit_status expected_miss_hit;
  bool loadstore = 0; //solo loads
  bool debug = 0;
  bool tag_A_evicted = false;
  bool politica = 0;
  struct operation_result result = {};

    // Choose a random policy
      policy = rand()%2;      // Escoge un valor aleatorio entre 0 y 1.

    // Choose a random associativity
      associativity = 1 << (rand()%4);  // Puede ser 1, 2, 4, 8, 16.

    if (debug_on) {
    printf("Entry Info\n associativity: %d\n  Politica Remplazo (0 LRU, 1 SRRIP): %d\n",
          associativity,
          policy); } 

    // Fill a cache entry
      struct entry cache_line[associativity];
      idx = rand()%1024;

      for(i=0; i<associativity; i++)
      {
        cache_line[i].valid = false;
        cache_line[i].tag = 0;
        cache_line[i].dirty = false;
        cache_line[i].rp_value = (associativity-1);
      } 

    // Insert a new block A
    // Force a hit on block A
      int Block_A = 1196;  // Se elige un valor menor a 4096.

      for(i = 0; i<2; i++){
        if(politica){ // SRRIP
        status = srrip_replacement_policy(idx, 
                                        Block_A, 
                                        associativity,
                                        loadstore,
                                        cache_line,
                                        &result,
                                        bool(debug_on));
        }
        else{         //LRU 
        status = lru_replacement_policy( idx, 
                                        Block_A,  
                                        associativity,
                                        loadstore,
                                        cache_line,
                                        &result,
                                        bool(debug_on));
        }
        
        // Check rp_value of block A
        EXPECT_EQ(cache_line[0].rp_value, 0); // En la posición 0 porque ahí se va a dar el hit.
        
        // Keep inserting new blocks until A is evicted

        for(i=0; i < associativity; i++) // Insertará N valores nuevos, donde N = associativity
        {
          tag = rand()%4096; // Escoge un valor random para tag
          while(tag == Block_A){tag=rand()%4096;} // Se asegura de que sea diferente a Block_A
          if(politica){ // SRRIP
          status = srrip_replacement_policy(idx, 
                                          tag, 
                                          associativity,
                                          loadstore,
                                          cache_line,
                                          &result,
                                          bool(debug_on));
          }
          else{         //LRU 
          status = lru_replacement_policy( idx, 
                                          tag,  
                                          associativity,
                                          loadstore,
                                          cache_line,
                                          &result,
                                          bool(debug_on));
          }

          // Si el bloque expulsado es el A
          if(result.evicted_address == Block_A && !tag_A_evicted) 
            {
              tag_A_evicted = true;
            //  EXPECT_EQ(tag_A_evicted, true);
              EXPECT_EQ(i,associativity-1);
            }
        }


        
    }

}



/*
 * TEST4: Verifies evicted lines have the dirty bit set accordantly to the operations
 * performed.
 * 1. Choose a random policy
 * 2. Choose a random associativity
 * 3. Fill a cache entry with only read operations
 * 4. Force a write hit for a random block A
 * 5. Force a read hit for a random block B
 * 6. Force read hit for random block A
 * 7. Insert lines until B is evicted
 * 8. Check dirty_bit for block B is false
 * 9. Insert lines until A is evicted
 * 10. Check dirty bit for block A is true
 */
TEST(L1cache, writeback){
  int status;
  int i;
  int associativity;
  enum miss_hit_status expected_miss_hit;
  bool loadstore = 0; //solo loads
  bool debug = 0;
  bool politica = 0;
  struct operation_result result = {};



/////////////////////      1. Choose a random policy      ///////////////////
  politica = rand()%2;    //numero random de 0 a 1
  
/////////////////////      2. Choose a random associativity      ///////////////////
  associativity = 1 << (rand()%4);  // associativity puede ser 1, 2 , 4, 8 

  struct entry cache_line[2][associativity];  //----Primer bloque A, segundo B----

  DEBUG(Asociatividad y politica de remplazo);
  if (debug_on) {
    printf("Entry Info\n associativity: %d\n  Politica Remplazo (0 LRU, 1 SRRIP): %d\n",
          associativity,
          politica);
  }

  /* ----Inicializar los set del cache en valores por defecto ------*/
  for(int i = 0; i < 2; i++)
  {
    for (int j =  0; j < associativity; j++) {
      cache_line[i][j].valid = false;
      cache_line[i][j].tag = 0;
      cache_line[i][j].dirty = 0;
      cache_line[i][j].rp_value = associativity-1;
    }  
  }
  


/////////////////////      3. Fill a cache entry with only read operations:      ///////////////////
  int idx = rand()%1024;
  int tag[associativity]; 
  tag[0] = rand()%4096;     
  loadstore = 0; // en 0 es un Load/read, en 1 un store/write
  for(int j = 0; j < 2; j++)
  {
    for(int i = 0; i < associativity; i++)
    {
      tag[i] = tag[0] + i; // -----Para que todos los tag ingresados sean diferentes-----,
      if(politica){ // SRRIP
      status = srrip_replacement_policy(idx, 
                                      tag[i], 
                                      associativity,
                                      loadstore,
                                      cache_line[j],
                                      &result,
                                      bool(debug_on));
      }
      else{         //LRU 
      status = lru_replacement_policy( idx, 
                                      tag[i], 
                                      associativity,
                                      loadstore,
                                      cache_line[j],
                                      &result,
                                      bool(debug_on));
      }
      expected_miss_hit = MISS_LOAD;
      EXPECT_EQ(status, 0);
      EXPECT_EQ(result.miss_hit, expected_miss_hit);   // Se espera que todos sean miss de read   
    }
  }
  

//Iteracion 1  
/////////////////////      4. Force a write hit for a random block A     ///////////////////

//Iteracion 2  
/////////////////////      5. Force a read hit for a random block B     ///////////////////  

  loadstore = 1; //---- en 0 es un Load/read, en 1 un store/write----
  int random_A_B;
  random_A_B = rand()%associativity; //----Calculando Bloque A y B-----


  for(int j = 0; j < 2; j++)
  {
    
    loadstore = false;
    if(j == 0){ loadstore = true;  }  //-------store/write--------

    if(politica){ // SRRIP
    status = srrip_replacement_policy(idx, 
                                    tag[random_A_B], 
                                    associativity,
                                    loadstore,
                                    cache_line[j],
                                    &result,
                                    bool(debug_on));
    }
    else{         //LRU 
    status = lru_replacement_policy( idx, 
                                    tag[random_A_B], 
                                    associativity,
                                    loadstore,
                                    cache_line[j],
                                    &result,
                                    bool(debug_on));
    }
    if(j == 0)
    {
      expected_miss_hit = HIT_STORE;
      EXPECT_EQ(status, 0);
      EXPECT_EQ(result.miss_hit, expected_miss_hit); 
      EXPECT_EQ(cache_line[j][random_A_B].dirty, 1);///////////asd
    }
    if(j == 1)
    {
      expected_miss_hit = HIT_LOAD;
      EXPECT_EQ(status, 0);
      EXPECT_EQ(result.miss_hit, expected_miss_hit);     
    }
  }
  
/////////////////////      6. Force read hit for random block A     ///////////////////


  loadstore = 0; // ---en 0 es un Load/read, en 1 un store/write---
  
  if(politica){ // SRRIP
  status = srrip_replacement_policy(idx, 
                                  tag[random_A_B ], 
                                  associativity,
                                  loadstore,
                                  cache_line[0],
                                  &result,
                                  bool(debug_on));
  }
  else{         //LRU 
  status = lru_replacement_policy( idx, 
                                  tag[random_A_B], 
                                  associativity,
                                  loadstore,
                                  cache_line[0],
                                  &result,
                                  bool(debug_on));
  }
  expected_miss_hit = HIT_LOAD;
  EXPECT_EQ(status, 0);
  EXPECT_EQ(result.miss_hit, expected_miss_hit);  

/////////////////////      7. Insert lines until B is evicted     ///////////////////

/////////////////////      8. Check dirty_bit for block B is false     ///////////////////





  int tag_random;
  bool tag_B_evicted = false; //Para saber si ya se saco el tag B
   
  while(!tag_B_evicted){
      // ---------------Encontrando un tag que no se encuentre en la cache---------------
     
      for(int i = 1; i <= associativity; i++)
      {           
        if(i == 1){ tag_random = rand()%4096; } // ----Definiendo el tag random---   
        if(tag_random == tag[i-1]){  i = 0;  }    // ----si el tag random ya es uno que esta, se comienza de nuevo el for---
      }    
      // insert lines
      loadstore = 0;
      if(politica){ // SRRIP
      status = srrip_replacement_policy(idx, 
                                      tag_random, 
                                      associativity,
                                      loadstore,
                                      cache_line[1],
                                      &result,
                                      bool(debug_on));
      }
      else{         //LRU 
      status = lru_replacement_policy( idx, 
                                      tag_random, 
                                      associativity,
                                      loadstore,
                                      cache_line[1],
                                      &result,
                                      bool(debug_on));
      }

      // -------------Si el bloque expulsado es el B----------------
      if(result.evicted_address == tag[random_A_B] && !tag_B_evicted)
      {
        tag_B_evicted = true;
        EXPECT_EQ(result.dirty_eviction, false);
      }     
  }


/////////////////////      9. Insert lines until A is evicted     ///////////////////

/////////////////////      10. Check dirty bit for block A is true     ///////////////////
  


  bool tag_A_evicted = false; //Para saber si ya se saco el tag A
        
  while(!tag_A_evicted){
      // -------------Encontrando un tag que no se encuentre en la cache----------------
      
      for(int i = 1; i <= associativity; i++)
      {

        if(i == 1){ tag_random = rand()%4096; } // ----Definiendo el tag random---   
        if(tag_random == tag[i-1]){  i = 0; }    // si el tag random ya es uno que esta, se comienza de nuevo el for
      }    
      // ----------------insert lines---------------
      loadstore = 0;
      if(politica){ // SRRIP
      status = srrip_replacement_policy(idx, 
                                      tag_random, 
                                      associativity,
                                      loadstore,
                                      cache_line[0],
                                      &result,
                                      bool(debug_on));
      }
      else{         //LRU 
      status = lru_replacement_policy( idx, 
                                      tag_random, 
                                      associativity,
                                      loadstore,
                                      cache_line[0],
                                      &result,
                                      bool(debug_on));
      }

      // ------------Si el bloque expulsado es el A-----------------
      if(result.evicted_address == tag[random_A_B ] && !tag_A_evicted)
      {
        tag_A_evicted = true;
        EXPECT_EQ(result.dirty_eviction, true);
      }     
  }

}

/*
 * TEST5: Verifies an error is return when invalid parameters are pass
 * performed.
 * 1. Choose a random policy 
 * 2. Choose invalid parameters for idx, tag and asociativy
 * 3. Check function returns a PARAM error condition
 */
TEST(L1cache, boundaries){
  
  int status;
  int idx_invalid;
  int tag_invalid;
  int associativity_invalid;
  int idx;
  int tag;
  int associativity;  
  bool politica;
  bool loadstore = 0;
  struct operation_result result = {};
  // Valores validos
  idx = rand()%1024;
  tag = rand()%4096;
  associativity = 1 << (rand()%4); 

/////////////////////      1. Choose a random policy      ///////////////////
  politica = rand()%2;    //numero random de 0 a 1

/////////////////////      2. Choose invalid parameters for idx, tag and asociativy     ///////////////////

  idx_invalid = (rand()%1024) * -1 ;
  tag_invalid = (rand()%4096) * -1 ;
  associativity_invalid = 1 << (rand()%4);
  if(associativity_invalid == 1){ associativity_invalid += 2;}
  else{associativity_invalid += 1;}
  

/////////////////////     3. Check function returns a PARAM error condition     ///////////////////

  //C------------------on Tag invalido y demas valores validos----------------------
    if(politica){ // SRRIP
    struct entry cache_line[associativity];
    status = srrip_replacement_policy(idx, 
                                    tag_invalid, 
                                    associativity,
                                    loadstore,
                                    cache_line,
                                    &result,
                                    bool(debug_on));
    }
    else{         //LRU 
    struct entry cache_line[associativity];
    status = lru_replacement_policy( idx, 
                                    tag_invalid, 
                                    associativity,
                                    loadstore,
                                    cache_line,
                                    &result,
                                    bool(debug_on));
    }
    EXPECT_EQ(status, ERROR);

  //------------------------Con index invalido y demas valores validos-----------------
    if(politica){ // SRRIP
    struct entry cache_line[associativity];
    status = srrip_replacement_policy(idx_invalid, 
                                    tag, 
                                    associativity,
                                    loadstore,
                                    cache_line,
                                    &result,
                                    bool(debug_on));
    }
    else{         //LRU 
    struct entry cache_line[associativity];
    status = lru_replacement_policy( idx_invalid, 
                                    tag, 
                                    associativity,
                                    loadstore,
                                    cache_line,
                                    &result,
                                    bool(debug_on));
    }
    EXPECT_EQ(status, ERROR);


  //---------------Con associativity invalido y demas valores validos-------------------
    if(politica){ // SRRIP
    struct entry cache_line[associativity_invalid];
    status = srrip_replacement_policy(idx, 
                                    tag, 
                                    associativity_invalid,
                                    loadstore,
                                    cache_line,
                                    &result,
                                    bool(debug_on));
    }
    else{         //LRU 
    struct entry cache_line[associativity_invalid];
    status = lru_replacement_policy( idx, 
                                    tag, 
                                    associativity_invalid,
                                    loadstore,
                                    cache_line,
                                    &result,
                                    bool(debug_on));
    }
    EXPECT_EQ(status, ERROR);
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
