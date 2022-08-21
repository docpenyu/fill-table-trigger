#include <Sort_Alg.h>
#include <benchmark/benchmark.h>

#include <iostream>
#include <random>

void create_input_Hits(std::vector<std::vector<hit_info_t>>& Hits, int hitRateHz, size_t total_channel)
{
    double lam = hitRateHz * double(1e-9);
    std::default_random_engine e(time(0));
    std::exponential_distribution<double> ex(lam);
    Hits.clear();
    for (size_t ch = 0; ch < total_channel; ++ch)
    {
        Hits.push_back(std::vector<hit_info_t>());
        auto& hits        = Hits.back();
        uint64_t time_now = 0;
        uint64_t tri_num  = 0;
        while (true)
        {
            time_now += static_cast<uint64_t>(std::floor(ex(e)));
            if (time_now >= 1'000'000)  // 1ms
                break;
            hits.emplace_back(time_now, ch);
            ++tri_num;
        }
    }
    // std::cout<<"create input hits data end."<<std::endl;
}

static void BM_slide(benchmark::State& state)
{
    size_t hit_rate = state.range(0);
    size_t ch_num   = state.range(1);
    size_t nhit     = state.range(2);
    Sort_Alg alg(nhit);
    std::vector<std::vector<hit_info_t>> Hits;
    create_input_Hits(Hits, hit_rate, ch_num);
    std::vector<hit_info_t> Sorted_Hits;
    alg.mergesort(Hits, Sorted_Hits);
    for (auto _ : state)
    {
        std::vector<uint64_t> winl, winr;
        alg.slide(Sorted_Hits, winl, winr);
    }
    //state.SetComplexityN(ch_num * hit_rate);
    state.SetComplexityN(ch_num);
}


BENCHMARK(BM_slide)->Unit(benchmark::kMicrosecond)->ArgsProduct({
        { 10000},
    benchmark::CreateRange(1000, 100'000, 2),
    //benchmark::CreateRange(1000, 100'000, 10),
    {100000}
//})->Iterations(1)->Repetitions(10)->Complexity();
})->Complexity(benchmark::BigO::oN);

BENCHMARK_MAIN();
