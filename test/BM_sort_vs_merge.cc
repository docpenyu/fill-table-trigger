#include <Sort_Alg.h>
#include <benchmark/benchmark.h>
#include <random>
#include <iostream>

void create_input_Hits(std::vector<std::vector<hit_info_t>> &Hits, int hitRateHz, size_t total_channel)
{
    double lam = hitRateHz * double(1e-9);
    std::default_random_engine e(time(0));
    std::exponential_distribution<double> ex(lam);
    Hits.clear();
    for (size_t ch = 0; ch < total_channel; ++ch){
        Hits.push_back(std::vector<hit_info_t>());
        auto& hits = Hits.back();
        uint64_t time_now = 0;
        uint64_t tri_num = 0;
        while(true){
            time_now += static_cast<uint64_t>(std::floor(ex(e)));
            if (time_now >= 1'000'000) // 1ms
                break;
            hits.emplace_back(time_now,ch);
            ++tri_num;
        }
    }
    // std::cout<<"create input hits data end."<<std::endl;
}

static void BM_Sort(benchmark::State& state)
{
    size_t  hit_rate = state.range(0);
    size_t ch_num = state.range(1);
    size_t nhit = state.range(2);
    Sort_Alg alg(nhit);
    std::vector<std::vector<hit_info_t>> Hits;
    create_input_Hits(Hits, hit_rate, ch_num);
    std::vector<hit_info_t> Sorted_Hits;
    for (auto _ : state){
        Sorted_Hits.clear();
        alg.mergesort(Hits,Sorted_Hits);
    }
    state.SetComplexityN(ch_num);
}
static void BM_Merge(benchmark::State& state)
{
    size_t  hit_rate = state.range(0);
    size_t ch_num = state.range(1);
    size_t nhit = state.range(2);
    Sort_Alg alg(nhit);
    std::vector<std::vector<hit_info_t>> Hits;
    create_input_Hits(Hits, hit_rate, ch_num);
    std::vector<hit_info_t> Sorted_Hits;
    for (auto _ : state){
        Sorted_Hits.clear();
        alg.sort(Hits,Sorted_Hits);
    }
    state.SetComplexityN(ch_num);
}

static void BM_Merge_2(benchmark::State& state)
{
    size_t  hit_rate = state.range(0);
    size_t ch_num = state.range(1);
    size_t nhit = state.range(2);
    Sort_Alg alg(nhit);
    std::vector<std::vector<hit_info_t>> Hits;
    create_input_Hits(Hits, hit_rate, ch_num);
    std::vector<hit_info_t> Sorted_Hits;
    for (auto _ : state){
        Sorted_Hits.clear();
        alg.sort_merge(Hits,Sorted_Hits);
    }
    state.SetComplexityN(ch_num);
}

BENCHMARK(BM_Sort)->Unit(benchmark::kMicrosecond)->ArgsProduct({
    {1000},
    benchmark::CreateDenseRange(1<<7, 1<<16, 1000),
    {100000}
})->Iterations(1)->Repetitions(10)->Complexity();
//BENCHMARK(BM_Merge)->Unit(benchmark::kMicrosecond)->ArgsProduct({
    //{1000},
    //benchmark::CreateDenseRange(1<<7, 1<<10, 1<<7),
    //{100000}
//})->Iterations(1)->Repetitions(10)->Complexity();
BENCHMARK(BM_Merge_2)->Unit(benchmark::kMicrosecond)->ArgsProduct({
    {1'000},
    benchmark::CreateDenseRange(1<<7, 1<<16, 1000),
    {100000}
})->Iterations(1)->Repetitions(10)->Complexity(benchmark::BigO::oN);

BENCHMARK_MAIN();
