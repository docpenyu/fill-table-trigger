#ifndef FILLTABLE_H 
#define FILLTABLE_H 
#include "hit_desc.h"
#include <vector>
#include <set>
#include <sys/time.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <cassert>

//#define TIME_FRAGEMENT_LENGTH 10'000'000

// improved fill-table alg with g=25ns
template<int G>
class FillTableAlg
{
public:
    FillTableAlg(int row, uint32_t nhit);
    ~FillTableAlg();
    FillTableAlg(const FillTableAlg&)=delete;
    FillTableAlg& operator=(const FillTableAlg&)=delete;
    
    void process(std::vector<std::vector<hit_desc_t>> &Hits, std::vector<uint64_t> &winLeft, std::vector<uint64_t> &winRight);
    // void process(void* input_data_ptr);
    // void scan(void* input_data_ptr, std::vector<std::vector<hit_desc_t> *> &hits);
    void scan(void* input_data_ptr, std::vector<std::vector<hit_info_t>> &hits);
    void scan_and_filltable(void* input_data_ptr);
    void pack(const std::vector<std::vector<hit_info_t>> &hits, 
            std::vector<uint64_t> &winLeft, std::vector<uint64_t> &winRight);
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
    unsigned int *m_trigerTable2 = nullptr;
    unsigned int * m_secondIndex = nullptr; 
    //unsigned int *m_timeTableLeft = nullptr;
    //unsigned int *m_timeTableRight = nullptr;
    //bool *m_flagTable = nullptr;

    //unsigned int m_testTable[10000]{}; //这里我先设宽度为1微秒
    uint64_t m_dataWin = 100;
    uint64_t m_time_begin = 0;
    size_t m_totalTrigger;
    uint64_t m_trgWin = 100;
    uint64_t m_step = 0;
    uint8_t m_event[2048];

};
template<int G>
FillTableAlg<G>::FillTableAlg(int row, uint32_t nhit)
    : m_row(row), m_nhit(nhit), m_totalTrigger(0), runtimes(0)
{
    m_step = m_trgWin / G ; // 触发窗对应的表格宽度 100ns 对应 4 个 25ns 的格子
    m_trigerTable1 = new unsigned int[m_column + m_step]{};
    m_trigerTable2 = new unsigned int[m_column + m_step]{};
    // m_secondIndex = new unsigned int[TIME_FRAGEMENT_LENGTH/m_trgWin+1]{0};
    // std::memset(usetimelist, 0, sizeof(usetimelist));
    // std::cout<<"create the fill-table alg, g = "<<G<<std::endl;
}

template<int G>
FillTableAlg<G>::~FillTableAlg()
{
    clearTable();
    delete[] m_trigerTable1;
    delete[] m_trigerTable2;
}

template<int G>
void FillTableAlg<G>::process(std::vector<std::vector<hit_desc_t>> &Hits, std::vector<uint64_t> &winLeft, std::vector<uint64_t> &winRight)
// void FillTableAlg<G>::process(void* input_data_ptr)
{
    // std::vector<std::vector<hit_info_t>> Hits;
    // scan(input_data_ptr, Hits);
    fillTable(Hits);            // 填表
    // std::vector<uint64_t> winLeft, winRight;
    slide(winLeft, winRight);   // 滑窗
     //-----------zhangsh added 04/12/2022----
    // pack(Hits, winLeft, winRight);
    //---------------------------------------
    assert(winLeft.size() == winRight.size());
}
template<int G>
void FillTableAlg<G>::scan(void* input_data_ptr, std::vector<std::vector<hit_info_t>> &hits)
{
    header* h_ptr = reinterpret_cast<header*>(input_data_ptr);
    if (h_ptr->mark != 0xcafecafe){
        std::cout<< "header::mark wrong."<<h_ptr->mark<<std::endl;
        exit(-1);
    }
    uint8_t *ptr = reinterpret_cast<uint8_t*>(input_data_ptr);
    uint8_t *ptr_end = ptr + h_ptr->total_size;
    ptr += sizeof(header);
    while (ptr < ptr_end){
        channel_header *ch_ptr = reinterpret_cast<channel_header*>(ptr);
        if (ch_ptr->mark != 0xcacaffff){
            std::cout<< "channel header::mark wrong."<<std::endl;
            exit(-1);
        }
        // std::vector<hit_info_t> *hit_one_channel = new std::vector<hit_info_t>;
        std::vector<hit_info_t> hit_one_channel;
        hit_one_channel.reserve(50);
        uint8_t* tq_ptr = ptr + sizeof(channel_header);
        uint8_t* tq_ptr_end = ptr + ch_ptr->total_size;
        while(tq_ptr < tq_ptr_end){
            hit_raw_data* hit = reinterpret_cast<hit_raw_data*>(tq_ptr);
            hit_info_t hit_info(hit->time, ch_ptr->channel_id);
            hit_info.ptr = tq_ptr;
            // hit_info.channelTag = ch_ptr->channel_id;
            // hit_info.time = hit->time;
            hit_one_channel.push_back(hit_info);
            tq_ptr += sizeof(hit_raw_data);
        }
        hits.push_back(hit_one_channel);
        ptr += ch_ptr->total_size;
    }
}

template<int G>
void FillTableAlg<G>::scan_and_filltable(void* input_data_ptr)
{
    header* h_ptr = reinterpret_cast<header*>(input_data_ptr);
    if (h_ptr->mark != 0xcafecafe){
        std::cout<< "header::mark wrong."<<h_ptr->mark<<std::endl;
        exit(-1);
    }
    uint8_t *ptr = reinterpret_cast<uint8_t*>(input_data_ptr);
    const uint8_t *ptr_end = ptr + h_ptr->total_size;
    ptr += sizeof(header);
    // find out the beginning time of input data
    uint64_t time_begin = std::numeric_limits<uint64_t>::max();
    while (ptr < ptr_end)
    {
        channel_header *ch_ptr = reinterpret_cast<channel_header*>(ptr);
        if (ch_ptr->mark != 0xcacaffff){
            std::cout<< "channel header::mark wrong."<<std::endl;
            exit(-1);
        }
        uint8_t* tq_ptr = ptr + sizeof(channel_header);
        uint8_t* tq_ptr_end = ptr + ch_ptr->total_size;
        while(tq_ptr < tq_ptr_end){
            hit_raw_data* hit = reinterpret_cast<hit_raw_data*>(tq_ptr);
            if (hit->time < time_begin)
                time_begin = hit->time;
            break;
        }
        ptr += ch_ptr->total_size;
    }
    m_time_begin = time_begin;
    // scan data & fill the table
    ptr = reinterpret_cast<uint8_t*>(input_data_ptr) + sizeof(header);
    uint64_t timeTableNum = 0;
    uint64_t timeTableNumEnd = 0;
    while (ptr < ptr_end){
        channel_header *ch_ptr = reinterpret_cast<channel_header*>(ptr);
        assert(ch_ptr->mark == 0xcacaffff);
        uint64_t timestamp = 0;
        uint8_t* tq_ptr = ptr + sizeof(channel_header);
        const uint8_t* tq_ptr_end = ptr + ch_ptr->total_size;
        while(tq_ptr < tq_ptr_end){
            hit_raw_data* hit = reinterpret_cast<hit_raw_data*>(tq_ptr);
            // the position of the hit in the table
            timeTableNum = (hit->time - m_time_begin) / G;
            // the position of the new timestamp
            timeTableNumEnd = timeTableNum + m_step; 
            if (timeTableNum >= timestamp){
                m_trigerTable1[timeTableNum] += 1;
            }
            else {
                for (size_t k = timestamp; k < timeTableNumEnd; ++k){
                    m_trigerTable2[k] += 1;
                }
            }
            timestamp = timeTableNumEnd;

            tq_ptr += sizeof(hit_raw_data);
        }
        ptr += ch_ptr->total_size;
    }
}

//zhangsh modified 04/12/2022
template<int G>
void FillTableAlg<G>::pack(const std::vector<std::vector<hit_info_t>> &Hits, std::vector<uint64_t> &winLeft, std::vector<uint64_t> &winRight)
{
    std::vector<int> v_pos(Hits.size(),0);
    for (int i = 0; i < winLeft.size(); ++i){
        uint64_t time_start = winLeft[i];
        uint64_t time_end = winRight[i];
    
    for (int ch = 0; ch < Hits.size(); ++ch){
        std::vector<hit_info_t> hits = Hits[ch];
        if (hits.size() == 0){
            continue;
        }
        int start = v_pos[ch];
        while ( start < hits.size()-1 && hits[start].time < time_start){
            ++start;
        }
        int end = start;
        while ( end < hits.size()-1 && hits[end].time < time_end){
            ++end;
        }
        v_pos[ch] = end;
        uint8_t *ptr_begin = hits.at(start).ptr;
        uint8_t *ptr_end = hits.at(end).ptr;
        std::memmove((void*)m_event,(void*)ptr_begin,(ptr_end-ptr_begin));
    }
    }
}
template<int G>
void FillTableAlg<G>::fillTable(const std::vector<std::vector<hit_desc_t>> &Hits)
{
    // 找时间起点
    uint64_t time_begin = std::numeric_limits<uint64_t>::max();
    for (auto&hits:Hits){
        if (hits.size()==0)continue;
        if (hits[0].time < time_begin)
            time_begin = hits[0].time;
    }
    m_time_begin = time_begin;

    uint64_t timeTableNum = 0;
    uint64_t timeTableNumEnd = 0;
    for (auto&hits : Hits){
        uint64_t timestamp = 0;
        for (auto &hit : hits){// 填表
            timeTableNum = (hit.time - m_time_begin) / G;
            timeTableNumEnd = timeTableNum + m_step ; // 尾后 位置
            if (timeTableNum >= timestamp){
                m_trigerTable1[timeTableNum] += 1;
            }
            else {
                for (size_t k = timestamp; k < timeTableNumEnd; ++k){
                    m_trigerTable2[k] += 1;
                }
            }
            timestamp = timeTableNumEnd;
        }
    }
}
template<int G>
void FillTableAlg<G>::slide(std::vector<uint64_t> &winL, std::vector<uint64_t> &winR)
{
    unsigned int hitNums = 0;
    uint32_t nhit = 0;
    bool flag = true;
    size_t start = 0;
    size_t end = start + m_step -1; // 尾，而非尾后
    while(true){
        if(flag){// 第一次滑窗需要对触发窗内的每一格求和
            for (int i = start; i <= end; ++i)
            {
                nhit += m_trigerTable1[i];
            }
            nhit += m_trigerTable2[end];
            flag = false;
        }
        else{// 此后滑窗只需要统计进入和离开窗口的hit数
            nhit -= m_trigerTable1[start-1];
            nhit += m_trigerTable1[end];
            nhit -= m_trigerTable2[end-1];
            nhit += m_trigerTable2[end];
        }
        if(nhit >= m_nhit){
            winL.push_back(start * G + m_time_begin - m_dataWin);
            winR.push_back(start * G + m_time_begin + m_dataWin);
            start += m_dataWin / G ;
            end = start + m_step - 1;
            flag = true;
            nhit = 0;
        }
        else{
            start += 1;
            end += 1;
        }
        if(end > m_column)
            break;
    }
    clearTable();
}
template<int G>
void FillTableAlg<G>::clearTable()
{
    std::memset(m_trigerTable1, 0, sizeof(unsigned int) * (m_column + m_step));
    std::memset(m_trigerTable2, 0, sizeof(unsigned int) * (m_column + m_step));
}

#endif
