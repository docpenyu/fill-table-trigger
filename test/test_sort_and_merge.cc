#include <Sort_Alg.h>
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
            if (time_now >= 100'000'000) // 100ms
                break;
            hits.emplace_back(time_now,ch);
            ++tri_num;
        }
    }
    // std::cout<<"create input hits data end."<<std::endl;
}
void test_merge_inplace()
{
}
void print_hits(std::vector<hit_info_t>& hits, size_t num)
{
    std::cout<<"Hits: ";
    for (size_t i = 0; i < num; ++i)
        std::cout<<hits[i].time<<" ";
    std::cout<<std::endl;
}
int main()
{
    size_t hit_rate = 10'000;
    size_t ch_num = 80000;
    Sort_Alg alg(10'000);

    std::vector<std::vector<hit_info_t>> Hits;
    create_input_Hits(Hits, hit_rate, ch_num);
    std::vector<hit_info_t> Sorted_Hits1;
    std::vector<hit_info_t> Sorted_Hits2;
    std::vector<hit_info_t> Sorted_Hits3;
    alg.sort(Hits,Sorted_Hits1);
    alg.mergesort(Hits, Sorted_Hits2);
    //alg.sort_inplace(Hits, Sorted_Hits3);
    alg.sort_merge(Hits, Sorted_Hits3);

    std::cout<<"merge size : "<<Sorted_Hits1.size()<<std::endl;
    std::cout<<"merge inplace size : "<<Sorted_Hits3.size()<<std::endl;
    std::cout<<"sort size : "<<Sorted_Hits2.size()<<std::endl;

    print_hits(Sorted_Hits2, 20);
    print_hits(Sorted_Hits3, 20);
    bool flag = true;
    bool flag2 = true;
    for(size_t i = 0; i < Sorted_Hits1.size(); ++i)
    {
        if (Sorted_Hits1[i].time != Sorted_Hits2[i].time) 
        {
            flag = false;
            std::cout<<"error i: "<<i<<std::endl; 
            break;
        }
    }
    if (flag)
        std::cout<<"hit1 == hit2"<<std::endl;
    for(size_t i = 0; i < Sorted_Hits1.size(); ++i)
    {
        if (Sorted_Hits1[i].time != Sorted_Hits3[i].time) 
        {
            flag2 = false;
            std::cout<<"error i: "<<i
                <<" hits_1: "<<Sorted_Hits1[i].time
                <<" hits_3: "<<Sorted_Hits3[i].time
                <<std::endl; 
            break;
        }
    }
    if (flag2)
        std::cout<<"hit1 == hit3"<<std::endl;
    return 0;
}
