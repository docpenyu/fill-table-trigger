#include <cmdline.h>
#include <fill_table_alg.h>
#include <Sort_Alg.h>
#include <fill_multi_row_table_g_is_25ns.h>
//#include <fill_table_alg2.h>

#include <chrono>
#include <iostream>
#include <random>

void create_input_data(void* ptr, size_t total_channel, int hitRateHz)
{
    header* header_ptr         = reinterpret_cast<header*>(ptr);
    header_ptr->mark           = 0xcafecafe;
    header_ptr->channelNumbers = total_channel;
    header_ptr->total_size     = sizeof(header);

    double lam = hitRateHz * double(1e-9);
    std::default_random_engine e(10);
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

int main(int argc, char** argv)
{
    size_t rowOfTable = 0;  // the number of channels `m`
    size_t hitRateHz  = 0;  // the frequence of tq trigger per channels `p`
    int nhit          = 0;
    cmdline::parser cmd_parser;
    cmd_parser.add<int>("channelNum", 'c', "channel number", true);
    cmd_parser.add<int>("hitRate", 'r', "hit rate per channel", true);
    cmd_parser.add<int>("Nhit", 'n', "multiplicity trigger threshold", false, 100'000);
    cmd_parser.parse_check(argc, argv);

    rowOfTable = cmd_parser.get<int>("channelNum");
    hitRateHz  = cmd_parser.get<int>("hitRate");
    nhit       = cmd_parser.get<int>("Nhit");

    std::cout << "The channel num = " << rowOfTable << ", the hit rate per channel = " << hitRateHz << "Hz"
              << ", the nhit value = " << nhit << std::endl;

    char* ptr = new char[size_t(hitRateHz * 0.01 * rowOfTable * sizeof(hit_raw_data) * 2 + sizeof(header) +
                                rowOfTable * sizeof(channel_header))];

    create_input_data(ptr, rowOfTable, hitRateHz);

    FillTableAlg<25> fill_table_25(rowOfTable, nhit);
    FillTableAlg<1> fill_table_1(rowOfTable, nhit);
    Sort_Alg sort_alg(nhit);
    FillMultiRowTable fill_m_table(rowOfTable,nhit);

    std::vector<uint64_t> winLeft_of_FT_25, winLeft_of_FT2_25;
    // test
    {
        std::vector<uint64_t> winLeft, winRight;
        std::chrono::steady_clock::time_point start, scan_over, slide_over;
        start = std::chrono::steady_clock::now();
        // step 1: scan raw hit data
        fill_table_25.scan_and_filltable(ptr);
        scan_over = std::chrono::steady_clock::now();
        // step 2: slide
        fill_table_25.slide(winLeft, winRight);
        slide_over = std::chrono::steady_clock::now();

        auto scan_usage = std::chrono::duration_cast<std::chrono::microseconds>(scan_over - start).count();
        auto slide_usage =
            std::chrono::duration_cast<std::chrono::microseconds>(slide_over - scan_over).count();
        std::cout << "filltable 25ns:"
                  << " scan: " << scan_usage << " slide: " << slide_usage
                  << " trigger num: " << winLeft.size() << std::endl;
        winLeft_of_FT_25 = std::move(winLeft);
    }
    {
        std::vector<uint64_t> winLeft, winRight;
        std::vector<std::vector<hit_desc_t>> hits;
        std::chrono::steady_clock::time_point start, scan_over, slide_over;
        start = std::chrono::steady_clock::now();
        // step 1: scan raw hit data
        fill_table_1.scan_and_filltable(ptr);
        scan_over = std::chrono::steady_clock::now();
        // step 2: slide
        fill_table_1.slide(winLeft, winRight);
        slide_over = std::chrono::steady_clock::now();

        auto scan_usage = std::chrono::duration_cast<std::chrono::microseconds>(scan_over - start).count();
        auto slide_usage =
            std::chrono::duration_cast<std::chrono::microseconds>(slide_over - scan_over).count();
        std::cout << "filltable 1ns:"
                  << " scan: " << scan_usage << " slide: " << slide_usage
                  << " trigger num: " << winLeft.size() << std::endl;
        winLeft_of_FT2_25 = std::move(winLeft);
    }
    {
        std::vector<uint64_t> winLeft, winRight;
        std::vector<std::vector<hit_desc_t>> hits;
        std::vector<hit_desc_t> sorted_hits;
        std::chrono::steady_clock::time_point start,  slide_over;
        start = std::chrono::steady_clock::now();
        sort_alg.scan(ptr, hits);
        sort_alg.mergesort(hits,sorted_hits);
        sort_alg.slide(sorted_hits,winLeft, winRight);
        slide_over = std::chrono::steady_clock::now();

        auto usage_time =
            std::chrono::duration_cast<std::chrono::microseconds>(slide_over - start).count();
        std::cout << "sort alg:"
                  << " time: " << usage_time 
                  << " trigger num: " << winLeft.size() << std::endl;
    }
    {
        std::vector<uint64_t> winLeft, winRight;
        std::vector<std::vector<hit_desc_t>> hits;
        std::chrono::steady_clock::time_point start,  slide_over;
        start = std::chrono::steady_clock::now();
        fill_m_table.scan(ptr);
        fill_m_table.searchTriggerWin(winLeft,winRight);
        slide_over = std::chrono::steady_clock::now();

        auto usage_time =
            std::chrono::duration_cast<std::chrono::microseconds>(slide_over - start).count();
        std::cout << "fill_Mrow_table alg:"
                  << " time: " << usage_time 
                  << " trigger num: " << winLeft.size() << std::endl;
    }
    {  // check correct
        //assert(winLeft_of_FT_25.size() == winLeft_of_FT2_25.size());
        //for (size_t i = 0, end = winLeft_of_FT_25.size(); i < end; ++i)
        //{
            //if (winLeft_of_FT_25[i] != winLeft_of_FT2_25[i])
            //{
                //std::cout<<"i: "<<i<<std::endl;
                //exit(-1);
            //}
        //}

        //std::cout << "test pass, trigger number: " << winLeft_of_FT_25.size() << std::endl;
    }
    delete [] ptr;
}
