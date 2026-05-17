#include "handType.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>


std::string HandType::to_string() const {
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
        if (keys[0] == CARDNUM::ACE)
            temp = "皇家同花顺";
        else
            temp = std::string("同花顺")+num2str(keys[0]);
    default:
        break;
    }
    return temp;
}

HandType HandType::evaluate(const std::vector<Card<CARDNUM>>& cards) { //支持2-7张牌的评估
    std::map<SUIT, int> suit_statistic = {
        {SUIT::HEA, 0},
        {SUIT::CLU, 0},
        {SUIT::DIA, 0},
        {SUIT::SPA, 0}
    };
    std::map<CARDNUM, int> num_statistic;

    for (int ci = 0; ci <= static_cast<int>(CARDNUM::ACE); ci++)
        num_statistic[static_cast<CARDNUM>(ci)] = 0;

    for (Card<CARDNUM> c : cards) {
        suit_statistic[c.getSuit()]++;
        num_statistic[c.getNum()]++;
    }
    bool flush = 0, straight = 0, quadra = 0, tag = 0;
    CARDNUM quaNum= CARDNUM::NUM_2, straiNum= CARDNUM::NUM_2, quaKicker;
    int pair = 0;
    CARDNUM pairNum[3];// 最多三对（不含三条及以上）
    int trible = 0;
    CARDNUM triNum[2]; // 最多两个三条 (不含)
    std::vector<CARDNUM> highNum;
    std::vector<CARDNUM> flushCards;
    // check flush
    for (auto it : suit_statistic)
        if (it.second >= 5) {
            flush = 1;
            for (Card<CARDNUM> c : cards)
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

    // check others
    if (num_statistic[CARDNUM::NUM_2]
        && num_statistic[CARDNUM::NUM_3]
        && num_statistic[CARDNUM::NUM_4]
        && num_statistic[CARDNUM::NUM_5]
        && num_statistic[CARDNUM::ACE]
        && !tag
    ) {
        straight = 1;
        straiNum = CARDNUM::NUM_5;
    }

    CARDNUM strai_low = CARDNUM::NUM_2;
    int k = 0;
    for (int ci = 0; ci <= static_cast<int>(CARDNUM::ACE); ci++) {
        CARDNUM i = static_cast<CARDNUM>(ci);
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
            strai_low = static_cast<CARDNUM>(ci + 1);
            break;
        default:
            break;
        }
        if (num_statistic[i] >= 1 && i == static_cast<CARDNUM>(static_cast<int>(strai_low) + 1)) {//    判断顺子
            strai_low = i;
            k++;
            if (k >= 4 && !tag) {
                straight = 1;
                straiNum = strai_low;
            }
        }
    }
    if (quadra) {   // 四条牌型，三条和对子的牌都可能是kicker
        quaKicker = CARDNUM::NUM_2;
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
        std::vector<CARDNUM> kt = {pairNum[0]};
        if (highNum.size() > 0)
            kt.push_back(highNum[0]);
        return {ONE_PAIR, kt};
    }
    return {HIGH_CARD, highNum};
}

int HandType::compareHandType(const HandType& t1, const HandType& t2) {
    if (t1.rank != t2.rank) {
        return t1.rank > t2.rank ? 1 : -1;
    }

    for (size_t i = 0; i < t1.keys.size(); i++) {
        if (t1.keys[i] != t2.keys[i]) {
            return t1.keys[i] > t2.keys[i] ? 1 : -1;
        }
    }
    return 0;
}



std::ostream& operator<<(std::ostream& out, const HandType& t) {
    out << t.to_string();
    return out;
}
