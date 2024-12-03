cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/installed/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/installed/bin)

project(software_trigger_test LANGUAGES C CXX VERSION 0.1.1)

message("PROJECT_NAME: ${PROJECT_NAME}")
message("PROJECT_VERSION: ${PROJECT_VERSION}")

## pthread
#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)
#link_libraries(Threads::Threads)

add_library(sort OBJECT)
target_sources(sort PRIVATE src/Sort_Alg.cpp)
target_include_directories(sort PUBLIC src)
target_compile_options(sort PUBLIC -g -O2)

add_library(fill_M_table OBJECT src/fill_multi_row_table_g_is_25ns.cpp)
target_include_directories(fill_M_table PUBLIC src)
target_compile_options(fill_M_table PUBLIC -g -O2)

find_package(benchmark)
#if(benchmark_FOUND)
    #add_executable(sort_vs_merge test/BM_sort_vs_merge.cc)
    #target_link_libraries(sort_vs_merge PUBLIC benchmark::benchmark sort)
    #target_compile_options(sort_vs_merge PUBLIC -g -O2)

    #add_executable(slide test/BM_test_slide.cc)
    #target_link_libraries(slide PUBLIC benchmark::benchmark sort)
    #target_compile_options(slide PUBLIC -g -O2)
#endif()
add_executable(test_sort_and_merge test/test_sort_and_merge.cc)
target_link_libraries(test_sort_and_merge sort)

add_executable(main src/main.cpp)
target_link_libraries(main sort fill_M_table)
target_compile_options(main PUBLIC -g -O2)

add_executable(test_filltable test/test_filltable.cc)
target_link_libraries(test_filltable sort fill_M_table)
target_compile_options(test_filltable PUBLIC -g -O2)

add_executable(improved_alg src/soft_improvedAlg.cc)
target_link_libraries(improved_alg)
target_compile_options(improved_alg PUBLIC -g -O2 -msse4 -msse2 -msse -mavx -mavx2 )

add_executable(sorted_hit_alg src/soft_sortedHitAlg.cc)
target_link_libraries(sorted_hit_alg sort)
target_compile_options(sorted_hit_alg PUBLIC -g -O2 -msse4 -msse2 -msse -mavx -mavx2 )

add_executable(tabled_hit_alg src/soft_tabledHitAlg.cc)
target_link_libraries(tabled_hit_alg)
target_compile_options(tabled_hit_alg PUBLIC -g -O2 -msse4 -msse2 -msse -mavx -mavx2 )
