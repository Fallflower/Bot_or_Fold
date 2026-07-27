#include "handType.h"
#include <vector>
#include <map>
#include <optional>
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
        temp = std::string("High: ") + num2str(keys[0]);
        break;
    case ONE_PAIR:
        temp = std::string("Pair: ") + num2str(keys[0]);
        if (keys.size() > 1)
            temp += " (kicker " + num2str(keys[1]) + ")";
        break;
    case TWO_PAIR:
        temp = std::string("Two Pair: ") + num2str(keys[0]) + " " + num2str(keys[1]);
        if (keys.size() > 2)
            temp += " (kicker " + num2str(keys[2]) + ")";
        break;
    case THREE_OF_A_KIND:
        temp = std::string("Trips: ") + num2str(keys[0]);
        if (keys.size() > 1)
            temp += " (kicker " + num2str(keys[1]) + ")";
        break;
    case STRAIGHT:
        temp = std::string("Straight: ") + num2str(keys[0]) + "-high";
        break;
    case FLUSH:
        temp = std::string("Flush: ") + num2str(keys[0]) + "-high";
        break;
    case FULL_HOUSE:
        temp = std::string("Full House: ") + num2str(keys[0]) + " over " + num2str(keys[1]);
        break;
    case FOUR_OF_A_KIND:
        temp = std::string("Quads: ") + num2str(keys[0]);
        if (keys.size() > 1)
            temp += " (kicker " + num2str(keys[1]) + ")";
        break;
    case STRAIGHT_FLUSH:
        if (keys[0] == static_cast<NumT>(NumT::ACE))
            temp = "Royal Flush";
        else
            temp = std::string("Straight Flush: ") + num2str(keys[0]) + "-high";
        break;
    default:
        break;
    }
    return temp;
}

template<typename NumT>
HandTypeDisplayData HandType<NumT>::displayData() const {
    HandTypeDisplayData data;
    data.rank = rank;

    const auto addDefiningRank = [&](size_t index) {
        if (index < keys.size())
            data.definingRanks.push_back(static_cast<int>(keys[index]));
    };
    const auto setKicker = [&](size_t index) {
        if (index < keys.size())
            data.kickerRank = static_cast<int>(keys[index]);
    };

    switch (rank) {
    case HIGH_CARD:
        addDefiningRank(0);
        break;
    case ONE_PAIR:
    case THREE_OF_A_KIND:
    case FOUR_OF_A_KIND:
        addDefiningRank(0);
        setKicker(1);
        break;
    case TWO_PAIR:
        addDefiningRank(0);
        addDefiningRank(1);
        setKicker(2);
        break;
    case STRAIGHT:
    case FLUSH:
        addDefiningRank(0);
        break;
    case FULL_HOUSE:
        addDefiningRank(0);
        addDefiningRank(1);
        break;
    case STRAIGHT_FLUSH:
        addDefiningRank(0);
        data.royalFlush = !keys.empty()
            && keys[0] == static_cast<NumT>(NumT::ACE);
        break;
    }
    return data;
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

    for (int i = kFirstRank<NumT>; i <= 12; i++)
        num_statistic[static_cast<NumT>(i)] = 0;

    for (Card<NumT> c : cards) {
        suit_statistic[c.getSuit()]++;
        num_statistic[c.getNum()]++;
    }
    bool flush = 0, straight = 0, quadra = 0, tag = 0;
    NumT quaNum= static_cast<NumT>(0), quaKicker= static_cast<NumT>(0);
    int pair = 0;
    NumT pairNum[3];// 最多三对（不含三条及以上）
    int trible = 0;
    NumT triNum[2]; // 最多两个三条 (不含)
    std::vector<NumT> highNum;
    std::vector<NumT> flushCards;
    
    NumT straiNum= static_cast<NumT>(0), fStraiNum= static_cast<NumT>(0);

    // Ace-low straight check (A2345 for long, A6789 for short)
    auto low0 = static_cast<NumT>(kFirstRank<NumT>);
    auto low1 = static_cast<NumT>(kFirstRank<NumT> + 1);
    auto low2 = static_cast<NumT>(kFirstRank<NumT> + 2);
    auto low3 = static_cast<NumT>(kFirstRank<NumT> + 3);

    // check flush
    for (auto it : suit_statistic)
        if (it.second >= 5) {
            flush = 1;
            for (Card<NumT> c : cards)
                if (c.getSuit() == it.first)
                    flushCards.push_back(c.getNum());
            // prepare for check straight flush
            std::map<NumT, int> fcns; // flush cards num statistic
            for (int i = kFirstRank<NumT>; i <= 12; i++)
                fcns[static_cast<NumT>(i)] = 0;
            for (Card<NumT> c : flushCards)
                fcns[c.getNum()]++;
            // check Ace-low straight flush
            if (fcns[low0]
                && fcns[low1]
                && fcns[low2]
                && fcns[low3]
                && fcns[NumT::ACE]) {
                tag = 1;
                fStraiNum = low3;
            }
            // check normal straight flush
            int sf_low = kFirstRank<NumT>;
            int k = 0;
            for (int i = kFirstRank<NumT>; i <= 12; i++) {
                if (fcns[static_cast<NumT>(i)] && i == sf_low + 1) {
                    sf_low = i;
                    k++;
                    if (k >= 4) {
                        tag = 1;
                        fStraiNum = static_cast<NumT>(sf_low);
                    }
                } else if (fcns[static_cast<NumT>(i)] == 0) {
                    k = 0;
                    sf_low = i + 1;
                }
            }
        }
    // return if straight flush found
    if (flush && tag) return {STRAIGHT_FLUSH, {fStraiNum}};
    // else, continue
    if (num_statistic[low0]
        && num_statistic[low1]
        && num_statistic[low2]
        && num_statistic[low3]
        && num_statistic[NumT::ACE]
        && !tag
    ) {
        straight = 1;
        straiNum = low3;
    }

    int strai_low = kFirstRank<NumT>;
    int k = 0;
    for (int i = kFirstRank<NumT>; i <= 12; i++) {
        NumT ci = static_cast<NumT>(i);
        switch (num_statistic[ci])
        {
        case 4:
            quadra = 1;
            quaNum = ci;
            break;
        case 3:
            triNum[trible++] = ci;
            break;
        case 2:
            pairNum[pair++] = ci;
            break;
        case 1://  记录kicker
            highNum.insert(highNum.begin(), ci);
            break;
        case 0://   清空顺子记录
            k = 0;
            strai_low = i + 1;
            break;
        default:
            break;
        }
        if (num_statistic[ci] >= 1 && i == strai_low + 1) {//    判断顺子
            strai_low = i;
            k++;
            if (k >= 4) {
                straight = 1;
                straiNum = static_cast<NumT>(strai_low);
            }
        }
    }
    if (quadra) {   // 四条牌型，三条和对子的牌都可能是kicker
        quaKicker = static_cast<NumT>(kFirstRank<NumT>);
        if (highNum.size() > 0) quaKicker = highNum[0];
        if (trible && triNum[trible - 1] > quaKicker) quaKicker = triNum[trible-1];
        if (pair && pairNum[pair-1] > quaKicker) quaKicker = pairNum[pair-1];
    }
    // return result
    if (quadra) return {FOUR_OF_A_KIND, {quaNum, quaKicker}};
    if (trible >= 2) return {FULL_HOUSE, {triNum[1], triNum[0]}};
    if (trible==1 && pair) return {FULL_HOUSE, {triNum[0], pairNum[pair - 1]}};
    if (flush) {std::sort(flushCards.rbegin(), flushCards.rend()); return {FLUSH, flushCards}; }
    if (straight) return {STRAIGHT, {straiNum}};
    if (trible == 1) {
        highNum.insert(highNum.begin(), triNum[0]);
        return {THREE_OF_A_KIND, highNum};
    }
    if (pair >=2 ) {// 两队牌型，可能是第三对做kicker，可能无kicker(6张牌成3对)
        std::optional<NumT> kicker;
        if (highNum.size() > 0) kicker = highNum[0];
        if (pair >= 3) {
            NumT k = (kicker.has_value() && pairNum[0] < *kicker ? *kicker : pairNum[0]);
            return {TWO_PAIR, {pairNum[2], pairNum[1], k}};
        }
        if (kicker)
            return {TWO_PAIR, {pairNum[1], pairNum[0], *kicker}};
        return {TWO_PAIR, {pairNum[1], pairNum[0]}};
    }
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

    size_t minSize = std::min({t1.keys.size(), t2.keys.size(), size_t(5)});
    for (size_t i = 0; i < minSize; i++) {
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
template struct HandType<CARDNUM>;
template struct HandType<SHORT_CARDNUM>;
template std::ostream& operator<<<CARDNUM>(std::ostream&, const HandType<CARDNUM>&);
template std::ostream& operator<<<SHORT_CARDNUM>(std::ostream&, const HandType<SHORT_CARDNUM>&);
