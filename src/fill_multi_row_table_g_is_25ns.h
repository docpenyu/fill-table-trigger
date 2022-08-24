#ifndef FILL_MULTI_ROW_TABLE_H
#define FILL_MULTI_ROW_TABLE_H
#include "hit_desc.h"
#include <vector>
#include <set>
//#include <sys/time.h>
#include <cstdint>
#include <cstring>

class FillMultiRowTable
{
public:
    FillMultiRowTable(int row, uint32_t nhit);
    ~FillMultiRowTable();
    
    void process(void* input_data_ptr);
    void scan(void*  input_data_ptr);
    void fillTable(const std::vector<std::vector<hit_info_t>*> &hits);
    void searchTriggerWin(std::vector<uint64_t> &winLeft, std::vector<uint64_t> &winRight);
    void clearTable();
    std::vector<uint64_t> printTime();
    int sumtriger();
    int runtimes;
   
private:
    //struct timeval start,end;
    uint64_t timeuse;
    uint64_t usetimelist[3];
    std::set<uint32_t> m_trgChannels;
    uint32_t m_nhit = 15;
    const uint64_t m_row ;
    const uint64_t m_column = INPUT_DATA_TIMESPAN_NS/25;
    uint8_t *m_trigerTable = nullptr;
    //unsigned int m_testTable[10000]{}; //这里我先设宽度为1微秒
    uint64_t m_dataWin = 100;
    uint64_t m_time_begin = 0;
    size_t m_totalTrigger;
    uint64_t m_trgWin = 100;

};

#endif //FILL_MULTI_ROW_TABLE_H
