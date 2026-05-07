#include "position.h"
#include "assistant.h"

/**
 * 根据 playerNum 初始化位置序列，
 * 通过 dealer 建立逻辑索引到位置序列索引的映射。
 * pn=2 -> {SB, BB}
 * pn=3 -> {SB, BB, D}
 * pn=4 -> {SB, BB, UTG, D}
 * pn=5 -> {SB, BB, UTG, CO, D}
 * pn=6 -> {SB, BB, UTG, HJ, CO, D}
 * pn=7 -> {SB, BB, UTG, MP, HJ, CO, D}
 * pn>7 -> {SB, BB, UTG, UTG+1, ... , MP, HJ, CO, D}
 */
Position::Position(int pn, int di) : dealer(di), playerNum(pn) {
    if (pn < 2)
        throw Error(100, "System Error: Player number must be at least 2.");
    poss = {" S B ", " B B "};
    switch (pn)
    {
    case 2: return;
    default: [[fallthrough]];
    case 7:
        poss.push_back(" M P "); [[fallthrough]];
    case 6:
        poss.push_back(" H J "); [[fallthrough]];
    case 5:
        poss.push_back(" C O "); [[fallthrough]];
    case 4:
        poss.insert(poss.begin() + 2, " UTG "); [[fallthrough]];
    case 3:
        poss.push_back("  D  "); break;
    }
    for (int i = 1; i <= pn - 7; i++)
        poss.insert(poss.begin() + 2 + i, "UTG+" + std::to_string(i));
}

int Position::find(const std::string& s) const {
    for (int i = 0; i < playerNum; i++)
        if (s == poss[i])
            return (i + dealer + 1) % playerNum;
    return playerNum;
}

void Position::step() {
    dealer = (dealer + 1) % playerNum;
}

std::string Position::operator[](int pi) const {
    return poss[(playerNum - 1 - dealer + pi) % playerNum];
}




// for test
std::ostream &Position::output(std::ostream &out) const {
    for (int i = 0; i < playerNum; i++)
        out << (*this)[i] << "\t";
    out << std::endl;
    return out;
}

std::ostream &operator<<(std::ostream &out, const Position& p) {
    p.output(out);
    return out;
}