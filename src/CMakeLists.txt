include_directories(${CacheSim_SOURCE_DIR}/include)
add_executable(l1cache main.cpp)
target_sources(l1cache PUBLIC debug_utilities.cpp L1cache)
