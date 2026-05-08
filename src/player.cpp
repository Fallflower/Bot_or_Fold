#include "player.h"
#include "assistant.h"


inline std::string action2str(const ACTION& action) {
    static const std::string actionStr[] = {"Fold", "Check", "Call", "Bet", "Raise To", "All-in", "All-in to Call"};
    return actionStr[action];
}

std::ostream &actInfo::output(std::ostream &out) const {
    out << std::left << std::setw(8) << action2str(act);
    if (act == RAISE)
        out << std::right << std::setw(6) << betAmount;
    else if (act == BET)
        out << std::left << std::setw(6) << betAmount;
    else
        out << std::left << std::setw(6) << ".";
    return out;
}