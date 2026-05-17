#ifndef __DECK_H__
#define __DECK_H__

#include "card.h"
#include <vector>
#include <string>
#include <random>
#include <algorithm>

template<typename NumT = CARDNUM>
class Deck {
public:
    Deck();
    Deck(const std::vector<Card<NumT>>&);

    void reset();
    void setShow(int stateCode);
    void shuffle();
    void deal(int playerNum, std::vector<std::vector<Card<NumT>>>& hands);

    const std::vector<Card<NumT>>& getPile() const { return pile_; }
    const std::vector<Card<NumT>>& getPubCards() const { return pubCards_; }

    std::vector<Card<NumT>> remainingDeck(int playerNum, int knownPubCards) const;
    std::vector<Card<NumT>> getFrontN(int n) const;
private:
    std::vector<Card<NumT>> pile_;
    std::vector<Card<NumT>> pubCards_;
};

template<typename NumT>
Deck<NumT>::Deck() {
    constexpr int first = kFirstRank<NumT>;
    constexpr int count = kRankCount<NumT>;
    for (int i = 0; i < 4; i++)
        for (int j = first; j < first + count; j++)
            pile_.push_back(Card<NumT>(static_cast<NumT>(j), static_cast<SUIT>(i)));
}

template<typename NumT>
Deck<NumT>::Deck(const std::vector<Card<NumT>>& cards) {
    constexpr int first = kFirstRank<NumT>;
    constexpr int count = kRankCount<NumT>;
    for (int i = 0; i < 4; i++)
        for (int j = first; j < first + count; j++)
            if (std::find(cards.begin(), cards.end(), Card<NumT>(static_cast<NumT>(j), static_cast<SUIT>(i))) == cards.end())
                pile_.push_back(Card<NumT>(static_cast<NumT>(j), static_cast<SUIT>(i)));
}

template<typename NumT>
void Deck<NumT>::reset() {
    for (size_t i = 0; i < pile_.size(); i++) {
        pile_[i].show = 0;
    }
    pubCards_.clear();
}

template<typename NumT>
void Deck<NumT>::setShow(int sc) {
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

template<typename NumT>
void Deck<NumT>::shuffle() {
    std::random_device seed;
    std::ranlux48 engine(seed());
    std::shuffle(pile_.begin(), pile_.end(), engine);
}

template<typename NumT>
void Deck<NumT>::deal(int playerNum, std::vector<std::vector<Card<NumT>>>& hands) {
    hands.clear();
    for (int i = 0; i < playerNum; i++)
        if (pile_[i] > pile_[i + playerNum])
            hands.push_back({pile_[i], pile_[i + playerNum]});
        else
            hands.push_back({pile_[i + playerNum], pile_[i]});
    int j = 2 * playerNum;
    pubCards_.assign(pile_.begin() + j, pile_.begin() + j + 5);
}

template<typename NumT>
std::vector<Card<NumT>> Deck<NumT>::getFrontN(int n) const {
    return std::vector<Card<NumT>>(pubCards_.begin(), pubCards_.begin() + n);
}

template<typename NumT>
std::vector<Card<NumT>> Deck<NumT>::remainingDeck(int playerNum, int knownPubCards) const {
    return std::vector<Card<NumT>>(
        pile_.begin() + 2 * playerNum + knownPubCards,
        pile_.end()
    );
}

#endif
