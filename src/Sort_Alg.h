#ifndef SORT_ALG_H
#define SORT_ALG_H
#include <chrono>
#include <cstdint>
#include <list>
#include <map>
#include <set>
#include <vector>

#include "hit_desc.h"

class Sort_Alg
{
public:
    Sort_Alg(uint32_t nhit);
    ~Sort_Alg();
    void process(const std::vector<std::vector<hit_info_t>> &Hits, std::vector<uint64_t> &winLeft,
                 std::vector<uint64_t> &winRight);
    // void process(void* input_data_ptr);

    void scan(void *input_data_ptr, std::vector<std::vector<hit_info_t>> &hits);
    // using std::merge
    void sort(const std::vector<std::vector<hit_info_t>> &Hits, std::vector<hit_info_t> &sortedData);
    // using std::inplace_merge
    void sort_inplace(const std::vector<std::vector<hit_info_t>> &Hits, std::vector<hit_info_t> &sortedData);
    // using my merge
    void sort_merge(const std::vector<std::vector<hit_info_t>> &Hits, std::vector<hit_info_t> &sortedData);
    // using std::sort
    bool mergesort(const std::vector<std::vector<hit_info_t>> &Hits, std::vector<hit_info_t> &sortedData);

    void slide(std::vector<hit_info_t> &times, std::vector<uint64_t> &winLeft,
               std::vector<uint64_t> &winRight);
    void pack(const std::vector<std::vector<hit_info_t>> &hits, std::vector<uint64_t> &winLeft,
              std::vector<uint64_t> &winRight);

    std::vector<uint64_t> printTime();
    bool m_configDone = false;
    int sumtriger();
    int runtimes;

private:
    struct timeval start, end;
    uint64_t timeuse;
    uint64_t usetimelist[4];

    std::set<uint32_t> m_trgChannels;
    uint32_t m_nhit    = 15;
    uint32_t m_trgWin  = 100;   // T = 100ns
    uint32_t m_dataWin = 100;   // Ts = 100ns

    size_t m_totalTrigger;
    uint32_t m_lastTotalTrigger;
    double m_rate;
    uint64_t m_lastTime;
    uint8_t m_event[2048];
};

#endif  // SORT_ALG_H
