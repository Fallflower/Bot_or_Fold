#include "player.h"
#include "assistant.h"


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