#include "pot_calculator.h"

#include <algorithm>

namespace holdem {

PotCalculation calculatePots(const std::vector<int>& contributions,
                             const std::vector<bool>& folded) {
    PotCalculation result;
    if (contributions.size() != folded.size()) return result;

    std::vector<int> levels;
    levels.reserve(contributions.size());
    for (int contribution : contributions) {
        if (contribution > 0) levels.push_back(contribution);
    }
    std::sort(levels.begin(), levels.end());
    levels.erase(std::unique(levels.begin(), levels.end()), levels.end());

    int previousLevel = 0;
    for (int level : levels) {
        std::vector<int> contributors;
        std::vector<int> eligible;
        for (size_t player = 0; player < contributions.size(); ++player) {
            if (contributions[player] < level) continue;
            contributors.push_back(static_cast<int>(player));
            if (!folded[player]) eligible.push_back(static_cast<int>(player));
        }

        const int amount = (level - previousLevel)
            * static_cast<int>(contributors.size());
        previousLevel = level;
        if (amount <= 0) continue;

        if (contributors.size() == 1) {
            const int player = contributors.front();
            auto refund = std::find_if(
                result.refunds.begin(), result.refunds.end(),
                [player](const PlayerAward& item) {
                    return item.playerIndex == player;
                });
            if (refund == result.refunds.end())
                result.refunds.push_back({player, amount});
            else
                refund->amount += amount;
            continue;
        }

        if (eligible.empty()) continue;
        if (!result.pots.empty()
            && result.pots.back().eligiblePlayers == eligible) {
            result.pots.back().amount += amount;
        } else {
            result.pots.push_back({amount, std::move(eligible)});
        }
    }
    return result;
}

} // namespace holdem
