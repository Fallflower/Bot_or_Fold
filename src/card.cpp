#include "card.h"
#define RESET "\033[0m"
#define WHITE "\033[47m"
#define RED "\033[31m"
#define BLACK "\033[30m"

const std::map<char, SUIT> smap = {
    {'H', HEA},
    {'C', CLU},
    {'D', DIA},
    {'S', SPA}
};

const std::map<char, CARDNUM> numap = {
    {'2', NUM_2},
    {'3', NUM_3},
    {'4', NUM_4},
    {'5', NUM_5},
    {'6', NUM_6},
    {'7', NUM_7},
    {'8', NUM_8},
    {'9', NUM_9},
    {'T', NUM_10},
    {'J', JACK},
    {'Q', QUEEN},
    {'K', KING},
    {'A', ACE}
};

const std::string num2str(const CARDNUM& c) {
    switch (c)
    {
    case NUM_2: return "2"; 
    case NUM_3: return "3";
    case NUM_4: return "4";
    case NUM_5: return "5";
    case NUM_6: return "6";
    case NUM_7: return "7";
    case NUM_8: return "8";
    case NUM_9: return "9";
    case NUM_10: return "T";
    case JACK: return "J";
    case QUEEN: return "Q";
    case KING: return "K";
    case ACE: return "A";
    }
    return "";
}

const std::string suit2str(const SUIT& s) {
    switch (s) {
    case HEA: return "\u2665";
    case CLU: return "\u2663";
    case DIA: return "\u2666";
    case SPA: return "\u2660";
    }
    return "";
}

std::string Card::toString() const {
    return num2str(cnum) + suit2str(suit);
}

std::string Card::toColorString() const {
    if (!show) return std::string(BLACK) + WHITE + "[??]" + RESET;
    // ANSI color: red for hearts/diamonds, black for clubs/spades
    std::string color = (suit == HEA || suit == DIA) ? RED : BLACK;

    return color + WHITE + "[" + num2str(cnum) + suit2str(suit) + "]" + RESET;
}

std::vector<std::string> Card::toAsciiArt() const {
    if (!show) {
        std::string cb = "\033[37;44m";   // 白色字，蓝色底
        return {
            cb + "┌─────┐" + RESET,
            cb + "│░░░░░│" + RESET,
            cb + "│░░░░░│" + RESET,
            cb + "│░░░░░│" + RESET,
            cb + "└─────┘" + RESET
        };
    }
    std::string color = (suit == HEA || suit == DIA) ? RED : BLACK;
    std::string cb = color + WHITE;   // 颜色+背景前缀

    // 牌点（数字转字符串后再处理对齐）
    std::string rank = num2str(cnum);   // "A" "2" ... "T" "J" "Q" "K"
    std::string leftRank  = rank == "T" ? "10" : rank + " ";   // 2字符，左对齐
    std::string rightRank = rank == "T" ? "10" : " " + rank;   // 2字符，右对齐

    // 花色符号
    std::string suitStr = suit2str(suit);   // "♠" "♥" "♦" "♣"

    std::vector<std::string> art;
    art.push_back(cb + "┌─────┐" + RESET);
    art.push_back(cb + "│" + leftRank  + "   │" + RESET);
    art.push_back(cb + "│  " + suitStr + "  │" + RESET);
    art.push_back(cb + "│   " + rightRank + "│" + RESET);
    art.push_back(cb + "└─────┘" + RESET);
    return art;
}

SUIT Card::getSuit() const {
    return suit;
}

CARDNUM Card::getNum() const {
    return cnum;
}

int Card::toInt() const {
    return int(cnum) * 4 + int(suit);
}

uint64_t Card::toLong() const {
    return (uint64_t)(1) << this->toInt();
}

uint64_t Card::cardsToLong(const std::vector<Card>& cards) {
    uint64_t res = 0;
    for (const Card& c : cards) {
        res |= c.toLong();      // 按位或运算更robust，避免重复卡牌导致的错误
    }
    return res;
}

bool operator>(const Card& p1, const Card& p2) {
    return p1.getNum() > p2.getNum();
}

bool operator<(const Card &p1, const Card &p2) {
    return p1.getNum() < p2.getNum();
}

bool operator==(const Card &p1, const Card &p2) {
    return p1.getNum() == p2.getNum() && p1.getSuit() == p2.getSuit();
}
std::ostream &Card::output(std::ostream &out) const
{
    out << toColorString();
    return out;
}

std::ostream &operator<<(std::ostream &out, const Card &p)
{
    p.output(out);
    return out;
}

// 将多张牌的asciiArt并排显示
std::ostream &operator<<(std::ostream &out, const std::vector<Card> &cards) {
    if (cards.empty()) return out;

    // 获取每张牌的矩阵
    std::vector<std::vector<std::string>> arts;
    for (const auto& c : cards)
        arts.push_back(c.toAsciiArt());

    int lines = arts[0].size();   // 所有牌的行数都一样
    for (int row = 0; row < lines; ++row) {
        for (size_t i = 0; i < arts.size(); ++i) {
            out << arts[i][row];
            if (i != arts.size() - 1)
                out << " ";      // 牌与牌之间空一格
        }
        out << "\n";             // 每张牌的同一行输出完成后再换行
    }
    return out;
}
