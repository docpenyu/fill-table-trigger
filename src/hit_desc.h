#ifndef HIT_DESC_H
#define HIT_DESC_H

#pragma once

#include <cstdint>
#define INPUT_DATA_TIMESPAN_NS 100'000'000ULL  // nano second

struct header
{
    uint32_t mark = 0xcafecafe;
    uint32_t channelNumbers; //存该头对应多少通道
    uint64_t total_size;   //存数据整体大小

};

struct channel_header
{
    uint32_t mark = 0xcacaffff;
    uint32_t channel_id;
    uint64_t hitNumbers;  //该通道对应多少hit
    uint64_t total_size;   //存数据整体大小
};
struct hit_raw_data
{
    // uint8_t channelId;
    uint32_t coarseTime;
    uint16_t charge;
    uint16_t fineTime;
    uint64_t time;
};

struct hit_desc_t{
 //probably need change
    uint8_t *ptr{nullptr};
    uint64_t time;
    uint32_t channelTag;
    hit_desc_t(uint64_t t,uint32_t chatag):time(t),channelTag(chatag){};
    hit_desc_t() = default;
};


using hit_info_t = hit_desc_t;
#endif
