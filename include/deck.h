#ifndef __DECK_H__
#define __DECK_H__

#include "card.h"
#include <vector>
#include <string>

class Deck {
public:
    Deck();
    Deck(const std::vector<Card<CARDNUM>>&);

    void reset();
    void setShow(int stateCode);
    void shuffle();
    void deal(int playerNum, std::vector<std::vector<Card<CARDNUM>>>& hands);

    const std::vector<Card<CARDNUM>>& getPile() const { return pile_; }
    const std::vector<Card<CARDNUM>>& getPubCards() const { return pubCards_; }

    std::vector<Card<CARDNUM>> remainingDeck(int playerNum, int knownPubCards) const;

    std::vector<Card<CARDNUM>> getFrontN(int n) const;
private:
    std::vector<Card<CARDNUM>> pile_;
    std::vector<Card<CARDNUM>> pubCards_;
};

#endif
