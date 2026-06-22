#ifndef __BOT_PLAYER_H__
#define __BOT_PLAYER_H__

#include "player.h"

class BotPlayer : public Player {
public:
    BotPlayer(const std::string &name, int startingChips) : Player(name, startingChips) {}

    ACTION makeAction(const gameInfo& info, int &betAmount) override;
};


#endif