#include "handType_.h"
#include <fstream>
#include <climits>
#include <cstring>


#define SUIT_0_MASK   0x1111111111111ULL
#define SUIT_1_MASK   0x2222222222222ULL
#define SUIT_2_MASK   0x4444444444444ULL
#define SUIT_3_MASK   0x8888888888888ULL
#define BIT_SUM_0     0x5555555555555ULL
#define BIT_SUM_1     0x3333333333333ULL
// File-scoped ranker instance (not a global)
namespace {
    advancedHandType s_ranker;
}

bool initAdvancedRanker(const unsigned char* data, unsigned int size) {
    return s_ranker.loadFromMemory(data, size);
}


// Compress rank bits: for each of the 13 ranks, count how many suits are present
static uint64_t ranksHash(uint64_t cards_hash) {
    cards_hash = (cards_hash & BIT_SUM_0) + ((cards_hash >> 1) & BIT_SUM_0);
    cards_hash = (cards_hash & BIT_SUM_1) + ((cards_hash >> 2) & BIT_SUM_1);
    return cards_hash;
}

// Check whether all cards in the bitmask share one suit
static bool isFlush(uint64_t hash) {
    int cnt = (hash & SUIT_0_MASK) != 0;
    cnt += (hash & SUIT_1_MASK) != 0;
    if (cnt > 1) return false;
    cnt += (hash & SUIT_2_MASK) != 0;
    if (cnt > 1) return false;
    cnt += (hash & SUIT_3_MASK) != 0;
    return cnt <= 1;
}

bool advancedHandType::load(const std::string& bin_path) {
    std::ifstream file(bin_path, std::ios::binary);
    if (!file) return false;

    flush_map.clear();
    other_map.clear();

    int cnt, val;
    uint64_t key;

    file.read(reinterpret_cast<char*>(&cnt), sizeof(cnt));
    for (int i = 0; i < cnt; i++) {
        file.read(reinterpret_cast<char*>(&key), sizeof(key));
        file.read(reinterpret_cast<char*>(&val), sizeof(val));
        flush_map[key] = val;
    }

    file.read(reinterpret_cast<char*>(&cnt), sizeof(cnt));
    for (int i = 0; i < cnt; i++) {
        file.read(reinterpret_cast<char*>(&key), sizeof(key));
        file.read(reinterpret_cast<char*>(&val), sizeof(val));
        other_map[key] = val;
    }

    file.close();
    loaded = true;
    return true;
}

bool advancedHandType::loadFromMemory(const unsigned char* data, unsigned int size) {
    flush_map.clear();
    other_map.clear();

    unsigned int offset = 0;

    // Read flush_map count
    int cnt, val;
    uint64_t key;

    if (offset + sizeof(cnt) > size) return false;
    std::memcpy(&cnt, data + offset, sizeof(cnt));
    offset += sizeof(cnt);

    for (int i = 0; i < cnt; i++) {
        if (offset + sizeof(key) + sizeof(val) > size) return false;
        std::memcpy(&key, data + offset, sizeof(key));
        offset += sizeof(key);
        std::memcpy(&val, data + offset, sizeof(val));
        offset += sizeof(val);
        flush_map[key] = val;
    }

    // Read other_map count
    if (offset + sizeof(cnt) > size) return false;
    std::memcpy(&cnt, data + offset, sizeof(cnt));
    offset += sizeof(cnt);

    for (int i = 0; i < cnt; i++) {
        if (offset + sizeof(key) + sizeof(val) > size) return false;
        std::memcpy(&key, data + offset, sizeof(key));
        offset += sizeof(key);
        std::memcpy(&val, data + offset, sizeof(val));
        offset += sizeof(val);
        other_map[key] = val;
    }

    loaded = true;
    return true;
}

void advancedHandType::convert(std::unordered_map<uint64_t, int>& strength_map) {
    flush_map.clear();
    other_map.clear();
    for (auto it = strength_map.begin(); it != strength_map.end(); ++it) {
        if (isFlush(it->first))
            flush_map[it->first] = it->second;
        else
            other_map[ranksHash(it->first)] = it->second;
    }
    loaded = true;
}

int advancedHandType::operator[](const uint64_t& hash) {
    auto it = flush_map.find(hash);
    if (it != flush_map.end()) return it->second;

    uint64_t hash_compressed = ranksHash(hash);
    auto it2 = other_map.find(hash_compressed);
    if (it2 != other_map.end()) return it2->second;

    return -1;
}

// Iterative k-combination generator: calls callback(set[i]...) for each combination
template<typename T, typename F>
void forEachCombination(const std::vector<T>& set, int k, F&& callback) {
    int n = set.size();
    if (k <= 0 || k > n) return;
    std::vector<int> idx(k);
    for (int i = 0; i < k; i++) idx[i] = i;
    while (true) {
        callback(set[idx[0]], set[idx[1]], set[idx[2]], set[idx[3]], set[idx[4]]);
        int i = k - 1;
        while (i >= 0 && idx[i] == n - k + i) i--;
        if (i < 0) break;
        idx[i]++;
        for (int j = i + 1; j < k; j++) idx[j] = idx[j - 1] + 1;
    }
}

int advancedEvaluate(const std::vector<Card<CARDNUM>>& cards) {
    std::vector<int> ints;
    ints.reserve(cards.size());
    for (const auto& c : cards) ints.push_back(c.toInt());

    int best_rank = INT_MAX;
    forEachCombination(ints, 5, [&](int a, int b, int c, int d, int e) {
        uint64_t mask = (1ULL << a) | (1ULL << b) | (1ULL << c) | (1ULL << d) | (1ULL << e);
        int rank = s_ranker[mask];
        if (rank >= 0 && rank < best_rank) best_rank = rank;
    });

    return best_rank;
}