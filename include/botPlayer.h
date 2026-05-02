#ifndef __BOT_PLAYER_H__
#define __BOT_PLAYER_H__

#include "player.h"

class BotPlayer : public Player {
    double equity = 0;
public:
    BotPlayer(const std::string &name, int startingChips) : Player(name, startingChips), equity(0) {}

    void setEquity(const double& eq) override { equity = eq; }
    ACTION makeAction(const int& chipsToCall, int &betAmount) override;
};


#endif