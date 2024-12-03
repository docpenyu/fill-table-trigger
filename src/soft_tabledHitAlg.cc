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
#include "create_hits.h"

#include "cmdline.h"
#include "fill_multi_row_table.h"
#include "hit_desc.h"


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
    FillMultiRowTableAlg<1> fill_Mtable_1(rowOfTable, nhit);

    const int N = 10;
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
