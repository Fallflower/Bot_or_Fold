#ifndef __HAND_TYPE_H__
#define __HAND_TYPE_H__
#include "card.h"

enum HANDRANK {
    HIGH_CARD,      // 高牌
    ONE_PAIR,       // 一对
    TWO_PAIR,       // 两对
    THREE_OF_A_KIND,// 三条
    STRAIGHT,       // 顺子
    FLUSH,          // 同花
    FULL_HOUSE,     // 葫芦
    FOUR_OF_A_KIND, // 四条
    STRAIGHT_FLUSH  // 同花顺
};

// 与自然语言无关的牌型展示数据。前端根据 rank 和 definingRanks
// 自行选择语言与排版；kickerRank 独立保存，避免解析 to_string()。
struct HandTypeDisplayData {
    HANDRANK rank = HIGH_CARD;
    std::vector<int> definingRanks;
    int kickerRank = -1;
    bool royalFlush = false;

    bool hasKicker() const { return kickerRank >= 0; }
};

template<typename NumT = CARDNUM>
struct HandType
{
    HANDRANK rank;
    std::vector<NumT> keys;

    std::string to_string() const;
    HandTypeDisplayData displayData() const;
    static HandType evaluate(const std::vector<Card<NumT>>& cards);
    static int compareHandType(const HandType& t1, const HandType& t2);
};

template<typename NumT>
std::ostream& operator<<(std::ostream& out, const HandType<NumT>& t);

#endif
