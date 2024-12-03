#ifndef ONLY_FILLTABLE_H
#define ONLY_FILLTABLE_H
#include <set>
#include <vector>

#include "hit_desc.h"
//#include <sys/time.h>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
// #include <intrin.h>
#include <smmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>

template <int G>
class FakeFillTable
{
public:
    FakeFillTable();
    ~FakeFillTable();
    void fillTable(const std::vector<std::vector<hit_desc_t>> &Hits);
    void clearTable();

private:
    const uint64_t m_column      = INPUT_DATA_TIMESPAN_NS / G;
    unsigned int *m_trigerTable1 = nullptr;
    // unsigned int *m_trigerTable2 = nullptr;
    uint64_t m_trgWin = 100;
    uint64_t m_step   = 0;
};

template <int G>
FakeFillTable<G>::FakeFillTable()
{
    m_step         = m_trgWin / G;  // 触发窗对应的表格宽度 100ns 对应 4 个 25ns 的格子
    m_trigerTable1 = new unsigned int[m_column + m_step]{};
    // m_trigerTable2 = new unsigned int[m_column + m_step]{};
    clearTable();
    // m_secondIndex = new unsigned int[TIME_FRAGEMENT_LENGTH/m_trgWin+1]{0};
    // std::memset(usetimelist, 0, sizeof(usetimelist));
    // std::cout<<"create the fill-table alg, g = "<<G<<std::endl;
}

template <int G>
FakeFillTable<G>::~FakeFillTable()
{
    clearTable();
    delete[] m_trigerTable1;
    // delete[] m_trigerTable2;
}
#pragma optimize("", off)
template <int G>
void FakeFillTable<G>::fillTable(const std::vector<std::vector<hit_desc_t>> &Hits)
{
    uint64_t timeTableNum = 0;
    uint64_t timeTableNumEnd = 0;
    for (auto&hits : Hits){
        uint64_t timestamp = 0;
        for (auto &hit : hits){// 填表
            timeTableNum = hit.time;
            timeTableNumEnd = timeTableNum + m_step ; // 尾后 位置
            // if (timeTableNum >= timestamp) {
                // m_trigerTable1[timeTableNum] += 1;
                int value = m_trigerTable1[timeTableNum] + 1;
                _mm_stream_si32((int*)(m_trigerTable1+timeTableNum),value);
            // }
            // else {
            //     for (size_t k = timestamp; k < timeTableNumEnd; ++k){
            //         m_trigerTable2[k] += 1;
            //     }
            // }
            timestamp = timeTableNumEnd;
        }
    }
}
#pragma optimize("", on)
template <int G>
void FakeFillTable<G>::clearTable()
{
    std::memset(m_trigerTable1, 0, sizeof(unsigned int) * (m_column + m_step));
    // std::memset(m_trigerTable2, 0, sizeof(unsigned int) * (m_column + m_step));
}
#endif