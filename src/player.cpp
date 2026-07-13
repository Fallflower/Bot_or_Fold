#include "player.h"
#include "assistant.h"


std::ostream &actInfo::output(std::ostream &out) const {
    if (act == CALL && betAmount == 0)
        out << std::left << std::setw(6) << "Check";
    else {
        out << std::left << std::setw(6) << action2str(act);
        if (act == RAISE || act == CALL)
            out << std::right << std::setw(4) << betAmount;
        else
            out << std::left << std::setw(4) << ".";
    }
    return out;
}