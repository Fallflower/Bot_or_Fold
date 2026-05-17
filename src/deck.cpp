#include "deck.h"
#include <random>
#include <algorithm>
// #include <sstream>

Deck::Deck() {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 13; j++)
            pile_.push_back(Card<CARDNUM>(static_cast<CARDNUM>(j), static_cast<SUIT>(i)));
}

// 构造一个除了输入的cards以外的牌的牌堆
Deck::Deck(const std::vector<Card<CARDNUM>>& cards) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 13; j++)
            if (std::find(cards.begin(), cards.end(), Card<CARDNUM>(static_cast<CARDNUM>(j), static_cast<SUIT>(i))) == cards.end())
            // 如果当前牌不在输入的cards中，则加入牌堆
                pile_.push_back(Card<CARDNUM>(CARDNUM(j), SUIT(i)));
}

void Deck::reset() {
    for (size_t i = 0; i < pile_.size(); i++) {
        pile_[i].show = 0;
    }
    pubCards_.clear();
}

void Deck::setShow(int sc) {
    switch (sc) {
    case 4: [[fallthrough]];
    case 3: pubCards_[4].show = 1; [[fallthrough]];
    case 2: pubCards_[3].show = 1; [[fallthrough]];
    case 1:
        pubCards_[2].show = 1;
        pubCards_[1].show = 1;
        pubCards_[0].show = 1;
        break;
    default: break;
    }
}

void Deck::shuffle() {
    std::random_device seed;
    std::ranlux48 engine(seed());
    std::shuffle(pile_.begin(), pile_.end(), engine);
}

void Deck::deal(int playerNum, std::vector<std::vector<Card<CARDNUM>>>& hands) {
    hands.clear();
    for (int i = 0; i < playerNum; i++)
        if (pile_[i] > pile_[i + playerNum])
            hands.push_back({pile_[i], pile_[i + playerNum]});
        else
            hands.push_back({pile_[i + playerNum], pile_[i]});
    int j = 2 * playerNum;
    pubCards_.assign(pile_.begin() + j, pile_.begin() + j + 5);
}

std::vector<Card<CARDNUM>> Deck::getFrontN(int n) const {     // n < 5
    return std::vector<Card<CARDNUM>>(pubCards_.begin(), pubCards_.begin() + n);
}

std::vector<Card<CARDNUM>> Deck::remainingDeck(int playerNum, int knownPubCards) const {
    return std::vector<Card<CARDNUM>>(
        pile_.begin() + 2 * playerNum + knownPubCards,
        pile_.end()
    );
}
