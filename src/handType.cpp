#include "handType.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>

// 不同牌型的 HANDRANK 到实际大小的映射（短牌中同花 > 葫芦）
template<typename T> constexpr int handRankOrder(HANDRANK r);

template<> constexpr int handRankOrder<CARDNUM>(HANDRANK r) {
    constexpr int map[] = {0,1,2,3,4,5,6,7,8};
    return map[r];
}

template<> constexpr int handRankOrder<SHORT_CARDNUM>(HANDRANK r) {
    constexpr int map[] = {0,1,2,3,4,6,5,7,8}; // FLUSH(5)→6, FULL_HOUSE(6)→5
    return map[r];
}

template<typename NumT>
std::string HandType<NumT>::to_string() const {
    std::string temp;
    switch (rank)
    {
    case HIGH_CARD:
        temp = std::string("高牌")+num2str(keys[0]);
        break;
    case ONE_PAIR:
        if (keys.size() > 1)
            temp = std::string("对")+num2str(keys[0])+" "+num2str(keys[1])+"踢";
        else
            temp = std::string("对")+num2str(keys[0]);
        break;
    case TWO_PAIR:
        temp = std::string("对")+num2str(keys[0])+"对"+num2str(keys[1])+" "+num2str(keys[2])+"踢";
        break;
    case THREE_OF_A_KIND:
        temp = std::string("三条")+num2str(keys[0])+" "+num2str(keys[1])+"踢";
        break;
    case STRAIGHT:
        temp = std::string("顺子")+num2str(keys[0]);
        break;
    case FLUSH:
        temp = std::string("同花")+num2str(keys[0]);
        break;
    case FULL_HOUSE:
        temp = num2str(keys[0])+" "+num2str(keys[1])+"的葫芦";
        break;
    case FOUR_OF_A_KIND:
        temp = std::string("四条")+num2str(keys[0])+" "+num2str(keys[1])+"踢";
        break;
    case STRAIGHT_FLUSH:
        if (keys[0] == static_cast<NumT>(NumT::ACE))
            temp = "皇家同花顺";
        else
            temp = std::string("同花顺")+num2str(keys[0]);
        break;
    default:
        break;
    }
    return temp;
}

template<typename NumT>
HandType<NumT> HandType<NumT>::evaluate(const std::vector<Card<NumT>>& cards) { //支持2-7张牌的评估
    std::map<SUIT, int> suit_statistic = {
        {SUIT::HEA, 0},
        {SUIT::CLU, 0},
        {SUIT::DIA, 0},
        {SUIT::SPA, 0}
    };
    std::map<NumT, int> num_statistic;

    for (int ci = 0; ci <= static_cast<int>(NumT::ACE); ci++)
        num_statistic[static_cast<NumT>(ci)] = 0;

    for (Card<NumT> c : cards) {
        suit_statistic[c.getSuit()]++;
        num_statistic[c.getNum()]++;
    }
    bool flush = 0, straight = 0, quadra = 0, tag = 0;
    NumT quaNum= static_cast<NumT>(0), straiNum= static_cast<NumT>(0), quaKicker= static_cast<NumT>(0);
    int pair = 0;
    NumT pairNum[3];// 最多三对（不含三条及以上）
    int trible = 0;
    NumT triNum[2]; // 最多两个三条 (不含)
    std::vector<NumT> highNum;
    std::vector<NumT> flushCards;
    // check flush
    for (auto it : suit_statistic)
        if (it.second >= 5) {
            flush = 1;
            for (Card<NumT> c : cards)
                if (c.getSuit() == it.first)
                    flushCards.push_back(c.getNum());
            std::sort(flushCards.rbegin(), flushCards.rend());
            for (size_t i = 0; i <= flushCards.size() - 5; i++) {
                if (static_cast<int>(flushCards[i]) == static_cast<int>(flushCards[i + 4]) - 4) {
                    tag = 1;
                    straiNum = flushCards[i];
                    break;
                }
            }
        }

    // Ace-low straight check (A2345 for long, A6789 for short)
    auto low0 = static_cast<NumT>(kFirstRank<NumT>);
    auto low1 = static_cast<NumT>(kFirstRank<NumT> + 1);
    auto low2 = static_cast<NumT>(kFirstRank<NumT> + 2);
    auto low3 = static_cast<NumT>(kFirstRank<NumT> + 3);
    if (num_statistic[low0]
        && num_statistic[low1]
        && num_statistic[low2]
        && num_statistic[low3]
        && num_statistic[static_cast<NumT>(NumT::ACE)]
        && !tag
    ) {
        straight = 1;
        straiNum = low3;
    }

    NumT strai_low = static_cast<NumT>(0);
    int k = 0;
    for (int ci = 0; ci <= static_cast<int>(NumT::ACE); ci++) {
        NumT i = static_cast<NumT>(ci);
        switch (num_statistic[i])
        {
        case 4:
            quadra = 1;
            quaNum = i;
            break;
        case 3:
            triNum[trible++] = i;
            break;
        case 2:
            pairNum[pair++] = i;
            break;
        case 1://  记录kicker
            highNum.insert(highNum.begin(), i);
            break;
        case 0://   清空顺子记录
            k = 0;
            strai_low = static_cast<NumT>(ci + 1);
            break;
        default:
            break;
        }
        if (num_statistic[i] >= 1 && i == static_cast<NumT>(static_cast<int>(strai_low) + 1)) {//    判断顺子
            strai_low = i;
            k++;
            if (k >= 4 && !tag) {
                straight = 1;
                straiNum = strai_low;
            }
        }
    }
    if (quadra) {   // 四条牌型，三条和对子的牌都可能是kicker
        quaKicker = static_cast<NumT>(0);
        if (highNum.size() > 0) quaKicker = highNum[0];
        if (trible && triNum[trible - 1] > quaKicker) quaKicker = triNum[trible-1];
        if (pair && pairNum[pair-1] > quaKicker) quaKicker = pairNum[pair-1];
    }
    // return result
    if (flush && straight && tag) return {STRAIGHT_FLUSH, {straiNum}};
    if (quadra) return {FOUR_OF_A_KIND, {quaNum, quaKicker}};
    if (trible >= 2) return {FULL_HOUSE, {triNum[1], triNum[0]}};
    if (trible==1 && pair) return {FULL_HOUSE, {triNum[0], pairNum[pair - 1]}};
    if (flush) return {FLUSH, flushCards};
    if (straight) return {STRAIGHT, {straiNum}};
    if (trible == 1) return {THREE_OF_A_KIND, {triNum[0], highNum[0], highNum[1]}};
    if (pair >= 2) return {TWO_PAIR, {pairNum[pair-1], pairNum[pair-2], highNum[0]}};
    if (pair == 1) {
        std::vector<NumT> kt = {pairNum[0]};
        if (highNum.size() > 0)
            kt.push_back(highNum[0]);
        return {ONE_PAIR, kt};
    }
    return {HIGH_CARD, highNum};
}

template<typename NumT>
int HandType<NumT>::compareHandType(const HandType& t1, const HandType& t2) {
    int r1 = handRankOrder<NumT>(t1.rank);
    int r2 = handRankOrder<NumT>(t2.rank);
    if (r1 != r2) {
        return r1 > r2 ? 1 : -1;
    }

    for (size_t i = 0; i < t1.keys.size(); i++) {
        if (t1.keys[i] != t2.keys[i]) {
            return t1.keys[i] > t2.keys[i] ? 1 : -1;
        }
    }
    return 0;
}

template<typename NumT>
std::ostream& operator<<(std::ostream& out, const HandType<NumT>& t) {
    out << t.to_string();
    return out;
}

// Explicit instantiations
template class HandType<CARDNUM>;
template class HandType<SHORT_CARDNUM>;
template std::ostream& operator<<<CARDNUM>(std::ostream&, const HandType<CARDNUM>&);
template std::ostream& operator<<<SHORT_CARDNUM>(std::ostream&, const HandType<SHORT_CARDNUM>&);
