#ifndef __HAND_TYPE__H__
#define __HAND_TYPE__H__
#include "card.h"
#include <unordered_map>

struct advancedHandType
{
    // Load pre-split binary file (flush_map + other_map sections)
    bool load(const std::string& bin_path);
    // Convert a raw full hash→rank map into split flush_map/other_map
    void convert(std::unordered_map<uint64_t, int>& strength_map);
    // Lookup: try flush_map first, then ranksHash + other_map
    int operator[](const uint64_t& hash);

    std::unordered_map<uint64_t, int> flush_map, other_map;
    bool loaded = false;
};

int advancedEvaluate(const std::vector<Card>& cards); //通过查表评估牌型，返回一个整数，整数越大牌型越好

#endif
