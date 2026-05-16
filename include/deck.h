#ifndef __DECK_H__
#define __DECK_H__

#include "card.h"
#include <vector>
#include <string>

class Deck {
public:
    Deck();
    Deck(const std::vector<Card>&);

    void reset();
    void setShow(int stateCode);
    void shuffle();
    void deal(int playerNum, std::vector<std::vector<Card>>& hands);

    const std::vector<Card>& getPile() const { return pile_; }
    const std::vector<Card>& getPubCards() const { return pubCards_; }

    std::vector<Card> remainingDeck(int playerNum, int knownPubCards) const;
    // std::string pubCardsStr(int stateCode) const;
    // std::string pubCardsColStr(int stateCode) const;

    std::vector<Card> getFrontN(int n) const;
private:
    std::vector<Card> pile_;
    std::vector<Card> pubCards_;
};

#endif
