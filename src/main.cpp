#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <sstream>
#include <string>

#include "Sort_Alg.h"
#include "cmdline.h"
// #include "fill_multi_row_table_g_is_25ns.h"
#include "fill_multi_row_table.h"
#include "fill_table_alg.h"
#include "hit_desc.h"

//#define INPUT_DATA_TIMESPAN_NS 1'000'000  // nano second
// extern template class FillTableAlg<25>;
// extern template class FillTableAlg<1>;

void create_input_Hits(std::vector<std::vector<hit_desc_t>>& Hits, int hitRateHz, size_t total_channel)
{
    double lam = hitRateHz * double(1e-9);
    std::default_random_engine e(time(0));
    std::exponential_distribution<double> ex(lam);
    Hits.clear();
    for (size_t ch = 0; ch < total_channel; ++ch)
    {
        Hits.push_back(std::vector<hit_desc_t>());
        auto& hits        = Hits.back();
        uint64_t time_now = 0;
        uint64_t tri_num  = 0;
        while (true)
        {
            time_now += static_cast<uint64_t>(std::floor(ex(e)));
            if (time_now >= INPUT_DATA_TIMESPAN_NS)
                break;
            hits.emplace_back(time_now, ch);
            ++tri_num;
        }
    }
    // std::cout<<"create input hits data end."<<std::endl;
}

void create_input_data(void* ptr, size_t total_channel, int hitRateHz)
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

    std::vector<std::vector<hit_info_t>> Hits;
    create_input_Hits(Hits, hitRateHz, rowOfTable);

    // just process 1ms data fragment. keep double redundancy
    // size_t buffer_size = hitRateHz * rowOfTable * sizeof(hit_raw_data) * 2 * 0.01 + sizeof(header) +
    //                      rowOfTable * sizeof(channel_header);
    // char* ptr = new char[buffer_size];
    // char *ptr = new char[50000];
    // create_input_data(ptr, rowOfTable, hitRateHz);

    Sort_Alg sort_alg(nhit);
    // FillTableAlg<25> fill_table_25(rowOfTable, nhit);
    FillTableAlg<1> fill_table_1(rowOfTable, nhit);
    FillMultiRowTableAlg<1> fill_Mtable_1(rowOfTable, nhit);
    const int N = 50;
    {
        std::vector<size_t> scan, sort, slide;
        size_t ev_num;
        std::cout<<"sort-hit: ";
        for (int i = 0; i < N; ++i)
        {
            // sort alg
            // std::vector<std::vector<hit_info_t>> Hits;
            std::vector<hit_info_t> sorted_hits;
            std::vector<uint64_t> winLeft, winRight;  // store the trigger result
            std::chrono::steady_clock::time_point start, scan_over, sort_over, slide_over, pack_over;
            start = std::chrono::steady_clock::now();
            // step 1: scan raw hit data
            // sort_alg.scan(ptr, Hits);
            // sort_alg.sort(Hits, sorted_hits);
            scan_over = std::chrono::steady_clock::now();
            // std::cout<<"channel : "<<Hits.size()<<std::endl;
            // step 2: sort hits
            // sort_alg.sort_merge(Hits, sorted_hits);
            sort_alg.mergesort(Hits, sorted_hits);
            sort_over = std::chrono::steady_clock::now();
            // std::cout<<"total hits: "<< sorted_hits.size()<<std::endl;
            // step 3: slide
            sort_alg.slide(sorted_hits, winLeft, winRight);
            slide_over = std::chrono::steady_clock::now();
            // step 4: pack
            ev_num = winLeft.size();
            scan.push_back(
                std::chrono::duration_cast<std::chrono::microseconds>(scan_over - start).count());
            sort.push_back(
                std::chrono::duration_cast<std::chrono::microseconds>(sort_over - scan_over).count());
            slide.push_back(
                std::chrono::duration_cast<std::chrono::microseconds>(slide_over - sort_over).count());
            std::cout<<std::chrono::duration_cast<std::chrono::microseconds>(slide_over - start).count()<<" ";
        }
        std::cout<<std::endl;

        size_t avg_scan  = std::accumulate(scan.begin(), scan.end(), 0) / scan.size();
        size_t avg_sort  = std::accumulate(sort.begin(), sort.end(), 0) / sort.size();
        size_t avg_slide = std::accumulate(slide.begin(), slide.end(), 0) / slide.size();
        std::cout << "sort-hit alg: scan " << avg_scan << " us"
                  << ", sort " << avg_sort << " us"
                  << ", slide " << avg_slide << " us"
                  << ", total_usage " << avg_scan + avg_sort + avg_slide << " us"
                  << ", ev num " << ev_num << std::endl;
    }
    /*{
        std::vector<size_t> scan, fill, slide;
        for (int i = 0; i < N; ++i)
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

            auto scan_usage =
                std::chrono::duration_cast<std::chrono::microseconds>(scan_over - start).count();
            auto slide_usage =
                std::chrono::duration_cast<std::chrono::microseconds>(slide_over - scan_over).count();

            scan.push_back(scan_usage);
            slide.push_back(slide_usage);
        }

        size_t avg_scan  = std::accumulate(scan.begin(), scan.end(), 0) / scan.size();
        size_t avg_slide = std::accumulate(slide.begin(), slide.end(), 0) / slide.size();
        std::cout << "fill-table-25ns alg: scan " << avg_scan << " us"
                  << ", slide " << avg_slide << " us"
                  << ", total_usage " << avg_scan + avg_slide << " us" << std::endl;
    }*/
    {
        std::vector<size_t> scan, fill, slide;
        size_t ev_num;
        std::cout<<"fill-table-1ns: ";
        for (int i = 0; i < N; ++i)
        {
            std::vector<uint64_t> winLeft, winRight;
            std::chrono::steady_clock::time_point start, scan_over, slide_over;
            start = std::chrono::steady_clock::now();
            // step 1: scan raw hit data
            fill_table_1.fillTable(Hits);
            // fill_table_1.scan_and_filltable(ptr);
            scan_over = std::chrono::steady_clock::now();
            // step 2: slide
            fill_table_1.slide(winLeft, winRight);
            slide_over = std::chrono::steady_clock::now();
            ev_num     = winLeft.size();
            auto scan_usage =
                std::chrono::duration_cast<std::chrono::microseconds>(scan_over - start).count();
            auto slide_usage =
                std::chrono::duration_cast<std::chrono::microseconds>(slide_over - scan_over).count();

            scan.push_back(scan_usage);
            slide.push_back(slide_usage);
            std::cout<<slide_usage+scan_usage<<" ";
        }
        std::cout<<std::endl;

        size_t avg_scan  = std::accumulate(scan.begin(), scan.end(), 0) / scan.size();
        size_t avg_slide = std::accumulate(slide.begin(), slide.end(), 0) / slide.size();
        std::cout << "fill-table-1ns alg: filltable " << avg_scan << " us"
                  << ", slide " << avg_slide << " us"
                  << ", total_usage " << avg_scan + avg_slide << " us"
                  << ", ev num " << ev_num << std::endl;
    }
    {
        std::vector<size_t> scan, fill, slide;
        size_t ev_num;
        std::cout<<"fill_Mtable-1ns: ";
        for (int i = 0; i < N; ++i)
        {
            std::vector<uint64_t> winLeft, winRight;
            std::chrono::steady_clock::time_point start, scan_over, slide_over;
            start = std::chrono::steady_clock::now();
            // step 1: scan raw hit data
            fill_Mtable_1.fillTable(Hits);
            scan_over = std::chrono::steady_clock::now();
            // step 2: slide
            fill_Mtable_1.slide(winLeft, winRight);
            slide_over = std::chrono::steady_clock::now();

            ev_num = winLeft.size();
            auto scan_usage =
                std::chrono::duration_cast<std::chrono::microseconds>(scan_over - start).count();
            auto slide_usage =
                std::chrono::duration_cast<std::chrono::microseconds>(slide_over - scan_over).count();

            scan.push_back(scan_usage);
            slide.push_back(slide_usage);
            std::cout<<slide_usage + scan_usage<<" ";
        }
        std::cout<<std::endl;

        size_t avg_scan  = std::accumulate(scan.begin(), scan.end(), 0) / scan.size();
        size_t avg_slide = std::accumulate(slide.begin(), slide.end(), 0) / slide.size();
        std::cout << "fill-Mtable-1ns alg: filltable " << avg_scan << " us"
                  << ", slide " << avg_slide << " us"
                  << ", total_usage " << avg_scan + avg_slide << " us"
                  << ", ev num " << ev_num << std::endl;
    }
    /*{
        char* data_copy        = new char[buffer_size];
        header* header_ptr     = reinterpret_cast<header*>(ptr);
        size_t input_data_size = header_ptr->total_size;
        std::memset(data_copy, 1, input_data_size);
        std::chrono::steady_clock::time_point start, end;
        start = std::chrono::steady_clock::now();
        std::memcpy(data_copy, ptr, input_data_size);
        end = std::chrono::steady_clock::now();
        delete[] data_copy;
        size_t time_usage = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "std::memcpy time: " << time_usage << " us" << std::endl;
    }
    delete[] ptr;
    {
        std::vector<std::vector<hit_desc_t>> Hits;
        std::vector<hit_desc_t> sorted_hits;
        create_input_Hits(Hits, hitRateHz, rowOfTable);
        for (auto& hits : Hits) sorted_hits.insert(sorted_hits.end(), hits.begin(), hits.end());

        std::chrono::steady_clock::time_point start, end;
        start = std::chrono::steady_clock::now();
        std::sort(sorted_hits.begin(), sorted_hits.end(),
                  [](const hit_desc_t& l, const hit_desc_t& r) { return l.time < r.time; });
        end               = std::chrono::steady_clock::now();
        size_t time_usage = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "std::sort time: " << time_usage << " us" << std::endl;
    }*/
    return 0;
}
