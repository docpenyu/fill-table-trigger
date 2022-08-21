#include "fill_multi_row_table_g_is_25ns.h"
#include <sys/time.h>
#include <vector>
#include <cassert>
#include <iostream>

FillMultiRowTable::FillMultiRowTable(int row, uint32_t nhit)
    : m_row(row), m_nhit(nhit), m_totalTrigger(0), runtimes(0)
{
    m_trigerTable = new uint8_t[static_cast<uint64_t>(m_row) * static_cast<uint64_t>(m_column)]();
    std::memset(usetimelist, 0, sizeof(usetimelist));
}
FillMultiRowTable::~FillMultiRowTable()
{
    delete[] m_trigerTable;
}
void FillMultiRowTable::process(void*  input_data_ptr)
{
    gettimeofday(&start, NULL);
    scan(input_data_ptr);
    gettimeofday(&end, NULL);
    timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    usetimelist[0] += timeuse;

    std::vector<uint64_t> winLeft, winRight;
    gettimeofday(&start, NULL);
    searchTriggerWin(winLeft, winRight);
    gettimeofday(&end, NULL);
    timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    usetimelist[1] += timeuse;

    gettimeofday(&start, NULL);
    clearTable();
    gettimeofday(&end, NULL);
    timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    usetimelist[2] += timeuse;
    m_totalTrigger += winLeft.size();
    ++runtimes;
    //std::cout<<"new wcdaalg triger = "<<winLeft.size()<<std::endl;
}
void FillMultiRowTable::scan(void* input_data_ptr)
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
    uint64_t i = 0;//row number
    while (ptr < ptr_end){
        channel_header *ch_ptr = reinterpret_cast<channel_header*>(ptr);
        assert(ch_ptr->mark == 0xcacaffff);
        uint64_t timestamp = 0;
        uint8_t* tq_ptr = ptr + sizeof(channel_header);
        const uint8_t* tq_ptr_end = ptr + ch_ptr->total_size;
        while(tq_ptr < tq_ptr_end){
            hit_raw_data* hit = reinterpret_cast<hit_raw_data*>(tq_ptr);
            uint64_t temp = (hit->time - m_time_begin) / static_cast<uint64_t>(25);
            if (temp < m_column)
            {
                m_trigerTable[temp * m_row + i] = 1;
                switch (temp)
                {
                default:
                    m_trigerTable[(temp - 3) * m_row + i] = 1;
                case 2:
                    m_trigerTable[(temp - 2) * m_row + i] = 1;
                case 1:
                    m_trigerTable[(temp - 1) * m_row + i] = 1;
                case 0:
                    break;
                }
                //m_testTable[temp / 40] += 1;
            }
            else
                std::cout << "wrong" << temp << ',' << m_time_begin << std::endl;
            tq_ptr += sizeof(hit_raw_data);
        }
        ++i;
        ptr += ch_ptr->total_size;
    }

}

void FillMultiRowTable::searchTriggerWin(std::vector<uint64_t> &winL, std::vector<uint64_t> &winR)
{
    unsigned int hitNums = 0;
    // int length = sizeof(m_testTable) / sizeof(m_testTable[0]);

    for (int i = 0; i < m_column - 3; ++i)
    {
        uint8_t *col_200 = &m_trigerTable[i * m_row];
        for (int j = 0; j < m_row;)
        {
            uint64_t *temp1;
            temp1 = reinterpret_cast<uint64_t *>(col_200 + j);
            if (*temp1)
            {
                uint64_t num = *temp1;
                int k = 0;
                while (num)
                {
                    num = num & (num - 1); //判断一个二进制数中有多少1
                    ++k;
                }
                hitNums += static_cast<unsigned int>(k);
            }
            j += 8;
        }
        if (hitNums >= m_nhit)
        { //满足n-hit条件就把相应的时间记录下来
            uint64_t left = m_time_begin + i * 25 - m_dataWin / 25 * 25;
            uint64_t right = m_time_begin + i * 25 + m_dataWin / 25 * 25;
            winL.push_back(left);
            winR.push_back(right);
            i += m_dataWin / 25 - 1;
        }
        hitNums = 0;
    }
    clearTable();
}

//fill the trigger table
void FillMultiRowTable::fillTable(const std::vector<std::vector<hit_info_t> *> &hits)
{

    uint64_t time_begin, dif;
    //找到时间起点
    time_begin = 10000000;
    for (int i = 0; i < m_row; ++i)
    {
        if (hits[i]->empty())
            continue;
        if ((hits[i]->front()).time < time_begin)
            time_begin = (hits[i]->front()).time;
    }
    m_time_begin = time_begin;

    for (size_t i = 0; i < m_row; ++i)
    {
        std::vector<hit_info_t> &it = *hits[i];
        for (auto j : it)
        {
            uint64_t temp = j.time;
            if (temp > 10000000)
                std::cout << "wrong" << temp << std::endl;
            temp -= time_begin;
            temp /= (uint64_t)25;
            if (temp < m_column)
            {
                m_trigerTable[temp * m_row + i] = 1;
                switch (temp)
                {
                default:
                    m_trigerTable[(temp - 3) * m_row + i] = 1;
                case 2:
                    m_trigerTable[(temp - 2) * m_row + i] = 1;
                case 1:
                    m_trigerTable[(temp - 1) * m_row + i] = 1;
                case 0:
                    break;
                }
                //m_testTable[temp / 40] += 1;
            }
            else
                std::cout << "wrong" << temp << ',' << time_begin << std::endl;
        }
    }
}

void FillMultiRowTable::clearTable()
{
    std::memset(m_trigerTable, 0, m_row * m_column);
    //std::memset(m_testTable, 0, sizeof(m_testTable));
}

std::vector<uint64_t> FillMultiRowTable::printTime()
{
    std::vector<uint64_t> a;
    a.push_back(usetimelist[0] / runtimes);
    a.push_back(usetimelist[1] / runtimes);
    a.push_back(usetimelist[2] / runtimes);
    return a;
}
int FillMultiRowTable::sumtriger()
{
    return m_totalTrigger / runtimes;
}
