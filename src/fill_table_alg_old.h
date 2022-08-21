#ifndef FILLTABLE2_H 
#define FILLTABLE2_H 
#include "hit_desc.h"
#include <vector>
#include <set>
#include <sys/time.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <cassert>

struct hit_desc_t;

// fill-table alg with g=25ns
template<int G>
class FillTableAlg2
{
public:
    FillTableAlg2(int row, uint32_t nhit);
    ~FillTableAlg2();
    FillTableAlg2(const FillTableAlg2&)=delete;
    FillTableAlg2& operator=(const FillTableAlg2&)=delete;
    
    void process(std::vector<std::vector<hit_desc_t>> &Hits, std::vector<uint64_t> &winLeft, std::vector<uint64_t> &winRight);
    // void process(void* input_data_ptr);
    // void scan(void* input_data_ptr, std::vector<std::vector<hit_desc_t> *> &hits);
    //void scan(void* input_data_ptr, std::vector<std::vector<hit_info_t>> &hits);
    void scan_and_filltable(void* input_data_ptr);
    //void pack(const std::vector<std::vector<hit_info_t>> &hits, 
            //std::vector<uint64_t> &winLeft, std::vector<uint64_t> &winRight);
    void slide(std::vector<uint64_t> &winLeft, std::vector<uint64_t> &winRight);
    // void pack(void* input_data_ptr, std::vector<uint64_t> &winLeft, std::vector<uint64_t> &winRight);
    void fillTable(const std::vector<std::vector<hit_desc_t>> &Hits);
    void clearTable();

    // std::vector<uint64_t> printTime();
    // int sumtriger();
    int runtimes;
   
private:
    // struct timeval start,end;
    // uint64_t timeuse;
    // uint64_t usetimelist[3];
    std::set<uint32_t> m_trgChannels;
    uint32_t m_nhit = 15;
    const uint64_t m_row = 1;
    const uint64_t m_column = INPUT_DATA_TIMESPAN_NS/G;
    unsigned int *m_trigerTable1 = nullptr;
    //unsigned int *m_trigerTable2 = nullptr;
    unsigned int * m_secondIndex = nullptr; 
    //unsigned int *m_timeTableLeft = nullptr;
    //unsigned int *m_timeTableRight = nullptr;
    //bool *m_flagTable = nullptr;

    //unsigned int m_testTable[10000]{}; //这里我先设宽度为1微秒
    uint64_t m_dataWin = G;
    uint64_t m_time_begin = 0;
    size_t m_totalTrigger;
    uint64_t m_trgWin = 100;
    uint64_t m_step = 0;
    uint8_t m_event[2048];

};
#endif
