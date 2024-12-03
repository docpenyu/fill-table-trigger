#include <iostream>
#include <random>
#include <vector>

#include "hit_desc.h"

inline void create_input_Hits(std::vector<std::vector<hit_desc_t>>& Hits, int hitRateHz,
                              size_t total_channel)
{
    double lam = hitRateHz * double(1e-9);
    std::default_random_engine e(time(0));
    std::exponential_distribution<double> ex(lam);
    Hits.clear();
    size_t total_hit = 0;
    for (size_t ch = 0; ch < total_channel; ++ch)
    {
        Hits.push_back(std::vector<hit_desc_t>());
        auto& hits        = Hits.back();
        uint64_t time_now = 0;
        while (true)
        {
            time_now += static_cast<uint64_t>(std::floor(ex(e)));
            if (time_now >= INPUT_DATA_TIMESPAN_NS)
                break;
            hits.emplace_back(time_now, ch);
            ++total_hit;
        }
    }
    std::cout << "total hit number is " << total_hit << std::endl;
    // std::cout<<"create input hits data end."<<std::endl;
}

inline void create_input_data(void* ptr, size_t total_channel, int hitRateHz)
{
    header* header_ptr         = reinterpret_cast<header*>(ptr);
    header_ptr->mark           = 0xcafecafe;
    header_ptr->channelNumbers = total_channel;
    header_ptr->total_size     = sizeof(header);

    double lam = hitRateHz * double(1e-9);
    std::default_random_engine e(time(0));
    std::exponential_distribution<double> ex(lam);

    for (int ch = 0; ch < total_channel; ch++)
    {
        channel_header* ch_ptr =
            reinterpret_cast<channel_header*>(reinterpret_cast<uint8_t*>(ptr) + header_ptr->total_size);
        ch_ptr->mark       = 0xcacaffff;
        ch_ptr->channel_id = ch;
        ch_ptr->total_size = sizeof(channel_header);

        uint64_t time_now     = 0;
        hit_raw_data* hit_ptr = reinterpret_cast<hit_raw_data*>(
            reinterpret_cast<uint8_t*>(ptr) + header_ptr->total_size + ch_ptr->total_size);
        while (true)
        {
            time_now += static_cast<uint64_t>(floor(ex(e)));
            if (time_now >= INPUT_DATA_TIMESPAN_NS)
                break;
            hit_ptr->time       = time_now;
            hit_ptr->coarseTime = 0;
            hit_ptr->fineTime   = 0;
            hit_ptr++;
        }
        ch_ptr->total_size = reinterpret_cast<char*>(hit_ptr) - reinterpret_cast<char*>(ch_ptr);
        header_ptr->total_size += ch_ptr->total_size;
    }
}