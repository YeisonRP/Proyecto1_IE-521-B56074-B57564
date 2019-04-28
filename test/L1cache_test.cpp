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

  /* Fill a random cache entry */
  idx = rand()%1024;
  tag = rand()%4096;
  associativity = 1 << (rand()%4);  // associativity puede ser 1, 2 , 4, 8 , 16
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

    EXPECT_EQ(status, 0);                                   //listo
    EXPECT_EQ(result.dirty_eviction, 0);                    //listo
    expected_miss_hit = loadstore ? MISS_STORE: MISS_LOAD;  //listo
    EXPECT_EQ(result.miss_hit, expected_miss_hit);          //listo
  }

  /*
   * Check for hit: block was replaced in last iteration, if we used the same 
   * tag now we will get a hit
   */
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

}

/*
 * TEST5: Verifies an error is return when invalid parameters are pass
 * performed.
 * 1. Choose a random policy 
 * 2. Choose invalid parameters for idx, tag and asociativy
 * 3. Check function returns a PARAM error condition
 */
TEST(L1cache, boundaries){

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
