#ifndef HOLDEM_POT_CALCULATOR_H
#define HOLDEM_POT_CALCULATOR_H

#include "gameState.h"

namespace holdem {

struct PotCalculation {
    std::vector<SidePot> pots;
    std::vector<PlayerAward> refunds;
};

PotCalculation calculatePots(const std::vector<int>& contributions,
                             const std::vector<bool>& folded);

} // namespace holdem

#endif
