# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
# Link Cache Cachetest GTest and pthread library
include_directories(${CacheSim_SOURCE_DIR}/include)
add_executable(cachetest L1cache_test.cpp)
target_sources(cachetest PUBLIC ${CacheSim_SOURCE_DIR}/src/debug_utilities.cpp
	                              ${CacheSim_SOURCE_DIR}/src/L1cache.cpp )
target_link_libraries(cachetest ${GTEST_LIBRARIES} pthread)
