#ifndef __HUMAN_PLAYER_H__
#define __HUMAN_PLAYER_H__

#include "player.h"

template<typename NumT = CARDNUM>
class HumanPlayer : public Player<NumT> {
public:
    HumanPlayer(const std::string &name, int startingChips) : Player<NumT>(name, startingChips) {}

    ACTION makeAction(const gameInfo<NumT>& info, int &betAmount) override;
};


#endif
