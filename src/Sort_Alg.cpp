#include "Sort_Alg.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <time.h>
#include <algorithm>
#include <deque>
#include <functional>
#include <numeric>
#include <cassert>
#include <unordered_map>

#define STREAM_TAG_WCDA_EVENT 0x00030001
#define STREAM_TAG_FULLREC 0xFFFF0000
#define DO_TRIGGER

Sort_Alg::Sort_Alg(uint32_t nhit) : m_nhit(nhit), m_totalTrigger(0),runtimes(0)
{
    std::memset(usetimelist, 0, sizeof(usetimelist));
}
Sort_Alg::~Sort_Alg() { }

void Sort_Alg::process(const std::vector<std::vector<hit_info_t>> &Hits, std::vector<uint64_t> &winLeft, std::vector<uint64_t> &winRight)
// void Sort_Alg::process(void* input_data_ptr)
{
    
    // sort
    // gettimeofday(&start, NULL);
    // std::vector<std::vector<hit_info_t>> Hits;
    // scan(input_data_ptr, Hits);
    // std::vector<uint64_t> winLeft, winRight;
    std::vector<hit_info_t> sortedHits;
    sortedHits.reserve(10000);
    mergesort(Hits, sortedHits);
    // gettimeofday(&end, NULL);
    // timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // usetimelist[1] += timeuse;
    // slide
    // gettimeofday(&start, NULL);
    // std::vector<uint64_t> winLeft, winRight;
    slide(sortedHits, winLeft, winRight);
    //-----------zhangsh added 04/12/2022----
    // pack(Hits, winLeft, winRight);
    //---------------------------------------
    // gettimeofday(&end, NULL);
    // timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // usetimelist[2] += timeuse;
    // pack raw data into event
    // gettimeofday(&start, NULL);
    // pack(hits, winLeft, winRight);
    // gettimeofday(&end, NULL);
    // timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // usetimelist[3] += timeuse;

    // m_totalTrigger += winLeft.size();

    // ++runtimes;
    // if (runtimes%10==1){
    //     std::cout<< "total hit : "<<sortedHits.size()<<std::endl;
    // }
    //std::cout<<"sort alg triger = "<<winLeft.size()<<std::endl;
}

void Sort_Alg::scan(void* input_data_ptr, std::vector<std::vector<hit_info_t>> &hits)
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
            hit_one_channel.push_back(hit_info);
            tq_ptr += sizeof(hit_raw_data);
        }
        hits.push_back(hit_one_channel);
        ptr += ch_ptr->total_size;
    }
}

void Sort_Alg::slide(std::vector<hit_info_t> &hits,
                     std::vector<uint64_t> &winLeft, std::vector<uint64_t> &winRight)
{
    if (hits.size() == 0)
    {
        return;
    }
    {
        uint64_t firstHitTime = hits[0].time;
        uint64_t lastHitTime = hits[hits.size() - 1].time;

        if (lastHitTime - firstHitTime > 1000000000)
        {
            std::cerr << "last hit time - first hit time > 1s"
                      << ", first hit time: " << firstHitTime
                      << ", last hit time: " << lastHitTime
                      << std::endl;
        }
    }

    size_t start = 0;
    size_t end = 0;
    // int flag = 0;  // 0, searching win right; 1, searching win left
    //std::map<uint32_t, int> winHits;
    std::unordered_map<uint32_t, int> winHits;
    int maxSize = 0;
    size_t total_hit_n = hits.size(); // total hit num
    while (true)
    {
        while (end < total_hit_n && hits[end].time - hits[start].time < m_trgWin)
        {
            hit_info_t &h = hits[end];
            if (winHits.count(h.channelTag))
            {
                winHits[h.channelTag]++;
            }
            else
            {
                winHits[h.channelTag] = 1;
            }
            end++;
        }
        if (winHits.size() > maxSize)
        {
            maxSize = winHits.size();
        }

        if (winHits.size() >= m_nhit)
        {
            // trigger!!

            // (start_hit-10us, end_hit+10us) is trigger window
            uint64_t left = hits[start].time - m_dataWin;
            uint64_t right = hits[start].time + m_dataWin;
            winLeft.push_back(left);
            winRight.push_back(right);

            // skip to data window end
            while (end < hits.size() && hits[end].time < right)
                end++;

            start = end;
            winHits.clear();
        }
        else
        {
            // not trigger, move start point forward
            hit_info_t &h = hits[start];
            winHits[h.channelTag]--;
            if (winHits[h.channelTag] == 0)
            {
                winHits.erase(h.channelTag);
            }
            start++;
        }

        // case 1: triggered, sliding window pointers have already been moved to the of this event;
        // case 2: not triggered

        // test sliding window end
        if (end >= total_hit_n)
        {
            // finish searching
            break;
        }
    }
}

bool compareHits(const hit_info_t& a, const hit_info_t& b)
{
    return a.time < b.time;
}

void Sort_Alg::sort(const std::vector<std::vector<hit_info_t>> &Hits, std::vector<hit_info_t> &sortedData)
{
    size_t channel_size = Hits.size();
    if (channel_size == 0)
        return;
    if (channel_size == 1)
    {
        sortedData = Hits.front();
        return;
    }
    if (channel_size == 2)
    {
        std::merge(Hits.front().begin(), Hits.front().end(),
                   Hits.back().begin(), Hits.back().end(),
                   std::back_inserter(sortedData), compareHits);
        return;
    }

    std::deque<std::vector<hit_info_t>> temp_hits;

    for(auto iter1 = Hits.begin(), iter2 = iter1 + 1, end = Hits.end(); iter1 < end;)
    {
        if (iter2 == end){
            temp_hits.push_back(*iter1);
            break;
        }
        std::vector<hit_info_t> temp;
        std::merge(iter1->begin(), iter1->end(),
                   iter2->begin(), iter2->end(),
                   std::back_inserter(temp), compareHits);
        temp_hits.push_back(temp);
        iter1 = iter2 + 1;
        iter2 = iter1 + 1;
    }
    while (temp_hits.size() > 2){
        std::vector<hit_info_t>& temp1 = temp_hits[0];
        std::vector<hit_info_t>& temp2 = temp_hits[1];
        std::vector<hit_info_t> temp;
        std::merge(temp1.begin(), temp1.end(),
                   temp2.begin(), temp2.end(),
                   std::back_inserter(temp), compareHits);
        temp_hits.push_back(temp);
        temp_hits.pop_front();
        temp_hits.pop_front();
    }
    std::merge(temp_hits[0].begin(), temp_hits[0].end(),
               temp_hits[1].begin(), temp_hits[1].end(),
               std::back_inserter(sortedData), compareHits);
}
void MergeHits(std::vector<hit_info_t>& hits, std::vector<hit_info_t>& temp, size_t l, size_t mid, size_t r, std::function<bool(const hit_info_t&, const hit_info_t&)>comp)
{
    size_t first{l}, last{mid};
    size_t pos{0};
    while(first < mid && last < r)
    {
        if(comp(hits[first], hits[last]))
            temp[pos++] = hits[first++];
        else
            temp[pos++] = hits[last++];
    }
    while(first < mid)
        temp[pos++] = hits[first++];
    while(last < r)
        temp[pos++] = hits[last++];
    for (size_t i = 0; i < pos; ++i)
        hits[l + i] = temp[i];
}
void Sort_Alg::sort_merge(const std::vector<std::vector<hit_info_t>> &Hits, std::vector<hit_info_t> &sortedData)
{
    sortedData.clear();
    size_t channel_size = Hits.size();
    if (channel_size == 0)
        return;
    size_t total_hits{0};
    for(auto& hit : Hits)total_hits += hit.size();
    sortedData.resize(total_hits);
    std::vector<hit_info_t> temp_hits(total_hits);
    // 先copy到sortedData中
    auto iter = sortedData.begin();
    std::vector<size_t> merge_partition;
    for(auto& hits : Hits)
    {
        std::copy(hits.begin(), hits.end(), iter);
        iter += hits.size();
        merge_partition.push_back(hits.size());
    }
    // 然后将sortedData中的每部分分别merge
    size_t sum = std::accumulate(merge_partition.begin(), merge_partition.end(),0);
    assert(sum == total_hits);
    while(merge_partition.size() > 1)
    {
        size_t pos_in_sortedvec{0};
        size_t pos_in_merge_partition{0};
        size_t p1{0}, p2{1};
        for (size_t par_size = merge_partition.size(); p2 < par_size;)
        {
            size_t l = pos_in_sortedvec;
            size_t m = l + merge_partition[p1];
            size_t r = m + merge_partition[p2];
            MergeHits(sortedData, temp_hits, l, m, r, compareHits);
            p1 = p2 + 1;
            p2 = p1 + 1;
            pos_in_sortedvec = r;
            merge_partition[pos_in_merge_partition++] = r - l;
        }
        if (p1 < merge_partition.size())
            merge_partition[pos_in_merge_partition++] = merge_partition[p1];
        merge_partition.resize(pos_in_merge_partition);
    }
}
// still exist a bug, output result is not right
void Sort_Alg::sort_inplace(const std::vector<std::vector<hit_info_t>> &Hits, std::vector<hit_info_t> &sortedData)
{
    size_t channel_size = Hits.size();
    if (channel_size == 0)
        return;
    if (channel_size == 1)
    {
        sortedData = Hits.front();
        return;
    }
    if (channel_size == 2)
    {
        std::merge(Hits.front().begin(), Hits.front().end(),
                   Hits.back().begin(), Hits.back().end(),
                   std::back_inserter(sortedData), compareHits);
        return;
    }
    std::vector<size_t> poses; // save the position
    for(auto iter1 = Hits.begin(), iter2 = iter1 + 1, end = Hits.end(); iter1 < end;)
    {
        if (iter2 == end){
            sortedData.insert(sortedData.end(),iter1->begin(),iter1->end());
            poses.push_back(iter1->size());
            break;
        }
        std::merge(iter1->begin(), iter1->end(),
                   iter2->begin(), iter2->end(),
                   std::back_inserter(sortedData), compareHits);
        poses.push_back(iter1->size() + iter2->size());
        iter1 = iter2 + 1;
        iter2 = iter1 + 1;
    }
    while(poses.size() == 1)
    {
        std::vector<size_t> temp_poses;
        auto iter1 = poses.begin();
        auto merge_begin_pos = sortedData.begin();
        for(auto iter2 = iter1 + 1, end = poses.end(); iter2 < end;)
        {
            size_t merged_vec_size = *iter1 + *iter2;
            std::inplace_merge(merge_begin_pos, merge_begin_pos + *iter1,
                               merge_begin_pos + merged_vec_size,
                               compareHits);
            temp_poses.push_back(merged_vec_size);
            merge_begin_pos += merged_vec_size;
            iter1 = iter2 + 1;
            iter2 = iter1 + 1;
        }
        if (iter1 != poses.end())
        {
            temp_poses.push_back(*iter1);
        }
        poses.swap(temp_poses);
    }
}

bool Sort_Alg::mergesort(const std::vector<std::vector<hit_info_t>> &hits, std::vector<hit_info_t> &sortedData)
{
    sortedData.clear();
    for (auto &chHits : hits)
    {
        for (auto &hitInfo : chHits)
        {
            sortedData.push_back(hitInfo);
        }
    }

    std::sort(sortedData.begin(), sortedData.end(), compareHits);
    return true;
}

//zhangsh modified 04/12/2022
void Sort_Alg::pack(const std::vector<std::vector<hit_info_t>> &Hits, std::vector<uint64_t> &winLeft, std::vector<uint64_t> &winRight)
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

// void Sort_Alg::pack(std::vector<std::vector<hit_info_t> *> &Hits, std::vector<uint64_t> &winLeft, std::vector<uint64_t> &winRight)
// {
//     std::vector<int> v_pos(Hits.size(),0);
//     for (int i = 0; i < winLeft.size(); ++i){
//         uint64_t time_start = winLeft[i];
//         uint64_t time_end = winRight[i];
    
//     for (int ch = 0; ch < Hits.size(); ++ch){
//         std::vector<hit_info_t> * hits = Hits[ch];
//         if (hits->size() == 0){
//             continue;
//         }
//         int start = v_pos[ch];
//         while ( start < hits->size()-1 && (*hits)[start].time < time_start){
//             ++start;
//         }
//         int end = start;
//         while ( end < hits->size()-1 && (*hits)[end].time < time_end){
//             ++end;
//         }
//         v_pos[ch] = end;
//         uint8_t *ptr_begin = hits->at(start).ptr;
//         uint8_t *ptr_end = hits->at(end).ptr;
//         std::memmove((void*)m_event,(void*)ptr_begin,(ptr_end-ptr_begin));
//     }
//     }
// }

// std::vector<uint64_t> Sort_Alg::printTime()
// {
//     std::vector<uint64_t> a;
//     a.push_back(usetimelist[0]/runtimes);
//     a.push_back(usetimelist[1]/runtimes);
//     a.push_back(usetimelist[2]/runtimes);
//     a.push_back(usetimelist[3]/runtimes);
//     return a;
// }
// int Sort_Alg::sumtriger()
// {
//     return m_totalTrigger/runtimes;
// }
