#ifndef __CARD_H__
#define __CARD_H__
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

enum class SUIT
{
    CLU,	// 梅花
    DIA,	// 方片
    HEA,	// 红桃
    SPA		// 黑桃
};

const std::string suit2str(const SUIT&);

enum class CARDNUM
{
    NUM_2,
    NUM_3,
    NUM_4,
    NUM_5,
    NUM_6,
    NUM_7,
    NUM_8,
    NUM_9,
    NUM_10,
    JACK,
    QUEEN,
    KING,
    ACE
};

enum class SHORT_CARDNUM
{
    NUM_6,
    NUM_7,
    NUM_8,
    NUM_9,
    NUM_10,
    JACK,
    QUEEN,
    KING,
    ACE
};

const std::string num2str(const CARDNUM&);
const std::string num2str(const SHORT_CARDNUM&);

#define RESET "\033[0m"
#define WHITE "\033[47m"
#define RED   "\033[31m"
#define BLACK "\033[30m"

template<typename NumT = CARDNUM>
struct Card
{
private:
    SUIT suit;
    NumT cnum;
public:
    bool show;

    Card(NumT c = NumT(), SUIT s = SUIT::CLU) : suit(s), cnum(c), show(0) {}

    std::string toString() const {
        return num2str(cnum) + suit2str(suit);
    }

    std::string toColorString() const {
        if (!show) return std::string(BLACK) + WHITE + "[??]" + RESET;
        std::string color = (suit == SUIT::HEA || suit == SUIT::DIA) ? RED : BLACK;
        return color + WHITE + "[" + num2str(cnum) + suit2str(suit) + "]" + RESET;
    }

    std::vector<std::string> toAsciiArt() const {
        if (!show) {
            std::string cb = "\033[37;44m";
            return {
                cb + "┌─────┐" + RESET,
                cb + "│░░░░░│" + RESET,
                cb + "│░░░░░│" + RESET,
                cb + "│░░░░░│" + RESET,
                cb + "└─────┘" + RESET
            };
        }
        std::string color = (suit == SUIT::HEA || suit == SUIT::DIA) ? RED : BLACK;
        std::string cb = color + WHITE;
        std::string rank = num2str(cnum);
        std::string leftRank  = rank == "T" ? "10" : rank + " ";
        std::string rightRank = rank == "T" ? "10" : " " + rank;
        std::string suitStr = suit2str(suit);

        std::vector<std::string> art;
        art.push_back(cb + "┌─────┐" + RESET);
        art.push_back(cb + "│" + leftRank  + "   │" + RESET);
        art.push_back(cb + "│  " + suitStr + "  │" + RESET);
        art.push_back(cb + "│   " + rightRank + "│" + RESET);
        art.push_back(cb + "└─────┘" + RESET);
        return art;
    }

    SUIT getSuit() const { return suit; }
    NumT getNum() const { return cnum; }

    int toInt() const {
        return static_cast<int>(cnum) * 4 + static_cast<int>(suit);
    }

    uint64_t toLong() const {
        return (uint64_t)(1) << this->toInt();
    }

    static uint64_t cardsToLong(const std::vector<Card>& cards) {
        uint64_t res = 0;
        for (const Card& c : cards) {
            res |= c.toLong();
        }
        return res;
    }

    std::ostream& output(std::ostream& out) const {
        out << toColorString();
        return out;
    }
};

template<typename NumT>
bool operator>(const Card<NumT>& p1, const Card<NumT>& p2) {
    return p1.getNum() > p2.getNum();
}

template<typename NumT>
bool operator<(const Card<NumT>& p1, const Card<NumT>& p2) {
    return p1.getNum() < p2.getNum();
}

template<typename NumT>
bool operator==(const Card<NumT>& p1, const Card<NumT>& p2) {
    return p1.getNum() == p2.getNum() && p1.getSuit() == p2.getSuit();
}

template<typename NumT>
std::ostream& operator<<(std::ostream& out, const Card<NumT>& p) {
    p.output(out);
    return out;
}

template<typename NumT>
std::ostream& operator<<(std::ostream& out, const std::vector<Card<NumT>>& cards) {
    if (cards.empty()) return out;

    std::vector<std::vector<std::string>> arts;
    for (const auto& c : cards)
        arts.push_back(c.toAsciiArt());

    int lines = static_cast<int>(arts[0].size());
    for (int row = 0; row < lines; ++row) {
        for (size_t i = 0; i < arts.size(); ++i) {
            out << arts[i][row];
            if (i != arts.size() - 1)
                out << " ";
        }
        out << "\n";
    }
    return out;
}

template<typename NumT>
void printCards(std::ostream& out, const std::vector<Card<NumT>>& cards,
                const std::string& indent = "") {
    if (cards.empty()) return;

    std::vector<std::vector<std::string>> arts;
    for (const auto& c : cards)
        arts.push_back(c.toAsciiArt());

    int lines = static_cast<int>(arts[0].size());
    for (int row = 0; row < lines; ++row) {
        out << indent;
        for (size_t i = 0; i < arts.size(); ++i) {
            out << arts[i][row];
            if (i != arts.size() - 1)
                out << " ";
        }
        out << "\n";
    }
}
#endif
