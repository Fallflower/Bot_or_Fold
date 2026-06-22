#ifndef __LLM_PLAYER_H__
#define __LLM_PLAYER_H__

#include "player.h"
#include <string>

class LLMPlayer : public Player {

    std::string buildPrompt(const gameInfo& info) const;

public:
    LLMPlayer(const std::string &name, int startingChips);

    ACTION makeAction(const gameInfo& info, int &betAmount) override;
};

#endif
