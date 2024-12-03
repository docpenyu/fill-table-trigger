#ifndef FILL_MULTI_ROW_TABLE_H
#define FILL_MULTI_ROW_TABLE_H
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <vector>

#include "hit_desc.h"

//#define TIME_FRAGEMENT_LENGTH 10'000'000

template <int G>
class FillMultiRowTableAlg
{
public:
    FillMultiRowTableAlg(int row, uint32_t nhit);
    ~FillMultiRowTableAlg();
    FillMultiRowTableAlg(const FillMultiRowTableAlg &) = delete;
    FillMultiRowTableAlg &operator=(const FillMultiRowTableAlg &) = delete;

    // void process(std::vector<std::vector<hit_desc_t>> &Hits, std::vector<uint64_t> &winLeft,
    // std::vector<uint64_t> &winRight);
    // void scan(void* input_data_ptr, std::vector<std::vector<hit_info_t>> &hits);
    // void scan_and_filltable(void* input_data_ptr);
    // void pack(const std::vector<std::vector<hit_info_t>> &hits,
    //        std::vector<uint64_t> &winLeft, std::vector<uint64_t> &winRight);
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
    uint32_t m_nhit         = 15;
    const uint64_t m_row    = 1;
    const uint64_t m_column = INPUT_DATA_TIMESPAN_NS / G;
    uint8_t *m_trigerTable  = nullptr;
    // unsigned int *m_trigerTable2 = nullptr;
    // unsigned int * m_secondIndex = nullptr;
    // unsigned int *m_timeTableLeft = nullptr;
    // unsigned int *m_timeTableRight = nullptr;
    // bool *m_flagTable = nullptr;

    // unsigned int m_testTable[10000]{}; //这里我先设宽度为1微秒
    uint64_t m_dataWin    = 100;
    uint64_t m_time_begin = 0;
    size_t m_totalTrigger;
    uint64_t m_trgWin = 100;
    uint64_t m_step   = 0;
};
template <int G>
FillMultiRowTableAlg<G>::FillMultiRowTableAlg(int row, uint32_t nhit)
    : m_row(row), m_nhit(nhit), m_totalTrigger(0), runtimes(0)
{
    m_step = m_trgWin / G;
    m_trigerTable =
        new uint8_t[static_cast<uint64_t>(m_row) * (static_cast<uint64_t>(m_column) + m_step)]();
    // std::memset(usetimelist, 0, sizeof(usetimelist));
}
template <int G>
FillMultiRowTableAlg<G>::~FillMultiRowTableAlg()
{
    delete[] m_trigerTable;
}
template <int G>
void FillMultiRowTableAlg<G>::fillTable(const std::vector<std::vector<hit_desc_t>> &Hits)
{
    uint64_t time_begin, dif;
    //找到时间起点
    time_begin = 10000000;
    for (int i = 0; i < m_row; ++i)
    {
        if (Hits[i].empty())
            continue;
        if ((Hits[i].front()).time < time_begin)
            time_begin = (Hits[i].front()).time;
    }
    m_time_begin = time_begin;

    for (size_t i = 0; i < m_row; ++i)
    {
        auto &hits = Hits[i];
        for (auto hit : hits)
        {
            uint64_t temp = hit.time;
            if (temp > INPUT_DATA_TIMESPAN_NS)
                std::cout << "wrong" << temp << std::endl;
            temp -= time_begin;
            temp /= G;
            uint64_t End = temp + m_step;
            for (auto k = temp; k < End; ++k)
            {
                m_trigerTable[k * m_row + i] = 1;
            }
        }
    }
}
template <int G>
void FillMultiRowTableAlg<G>::slide(std::vector<uint64_t> &winLeft, std::vector<uint64_t> &winRight)
{
    unsigned int hitNums = 0;
    // int length = sizeof(m_testTable) / sizeof(m_testTable[0]);

    for (int i = m_step - 1, end = m_column + m_step; i < end; ++i)
    {
        uint8_t *col_200 = &m_trigerTable[i * m_row];
        for (int j = 0;;)
        {
            uint64_t *temp1;
            temp1 = reinterpret_cast<uint64_t *>(col_200 + j);
            if (*temp1)
            {
                uint64_t num = *temp1;
                int k        = 0;
                while (num)
                {
                    num = num & (num - 1);  //判断一个二进制数中有多少1
                    ++k;
                }
                hitNums += static_cast<unsigned int>(k);
            }
            j += 8;
            if (j + 8 > m_row)
            {
                for (auto k = j; k < m_row; ++k)
                {
                    if (col_200[k] > 0)
                        ++hitNums;
                }
                break;
            }
        }
        if (hitNums >= m_nhit)
        {  //满足n-hit条件就把相应的时间记录下来
            uint64_t left  = m_time_begin + i * G - m_dataWin / G * G;
            uint64_t right = m_time_begin + i * G + m_dataWin / G * G;
            winLeft.push_back(left);
            winRight.push_back(right);
            i += m_dataWin / G - 1;
        }
        hitNums = 0;
    }
    clearTable();
}

template <int G>
void FillMultiRowTableAlg<G>::clearTable()
{
    std::memset(m_trigerTable, 0, m_row * (m_column + m_step));
}

#endif // FILL_MULTI_ROW_TABLE_H