#include "pot_calculator.h"

#include <cassert>

int main() {
    {
        const holdem::PotCalculation result = holdem::calculatePots(
            {200, 100}, {false, false});
        assert(result.pots.size() == 1);
        assert(result.pots[0].amount == 200);
        assert((result.pots[0].eligiblePlayers == std::vector<int>{0, 1}));
        assert(result.refunds.size() == 1);
        assert(result.refunds[0].playerIndex == 0);
        assert(result.refunds[0].amount == 100);
    }
    {
        const holdem::PotCalculation result = holdem::calculatePots(
            {100, 200}, {false, false});
        assert(result.pots.size() == 1);
        assert(result.pots[0].amount == 200);
        assert(result.refunds.size() == 1);
        assert(result.refunds[0].playerIndex == 1);
        assert(result.refunds[0].amount == 100);
    }
    {
        const holdem::PotCalculation result = holdem::calculatePots(
            {200, 100}, {false, true});
        assert(result.pots.size() == 1);
        assert(result.pots[0].amount == 200);
        assert((result.pots[0].eligiblePlayers == std::vector<int>{0}));
        assert(result.refunds.size() == 1);
        assert(result.refunds[0].playerIndex == 0);
        assert(result.refunds[0].amount == 100);
    }
    {
        const holdem::PotCalculation result = holdem::calculatePots(
            {200, 200, 100}, {false, false, false});
        assert(result.pots.size() == 2);
        assert(result.pots[0].amount == 300);
        assert(result.pots[1].amount == 200);
        assert(result.refunds.empty());
    }
    return 0;
}
