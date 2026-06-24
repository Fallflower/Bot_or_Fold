#ifndef __BOT_PLAYER_H__
#define __BOT_PLAYER_H__

#include "player.h"

enum HandTier {
    TIER_PREMIUM = 4,
    TIER_STRONG  = 3,
    TIER_PLAYABLE = 2,
    TIER_MARGINAL = 1,
    TIER_TRASH   = 0
};

template<typename NumT = CARDNUM>
class BotPlayer : public Player<NumT> {
public:
    BotPlayer(const std::string &name, int startingChips) : Player<NumT>(name, startingChips) {}

    ACTION makeAction(const gameInfo<NumT>& info, int &betAmount) override;

private:
    int calcPreflopScore(const gameInfo<NumT>& info) const;
    HandTier getTier(int score) const;
    bool isLatePosition(const std::string& pos) const;
    bool isBlind(const std::string& pos) const;

    ACTION actPreflop(const gameInfo<NumT>& info, int &betAmount);
    ACTION actPostflop(const gameInfo<NumT>& info, int &betAmount);
};

#endif
