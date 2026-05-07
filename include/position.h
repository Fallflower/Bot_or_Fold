#ifndef __POSITION_H__
#define __POSITION_H__
#include<iostream>
#include<string>
#include<vector>
class Position
{
private:
    std::vector<std::string> poss;
    int dealer;
    int playerNum;

public:
    Position(int pn = 3, int di = 1);
    int find(const std::string&) const;
    int getDealer() const { return dealer; }
    int getPlayerNum() const { return playerNum; }

    void step();

    std::string operator[](int pi) const;

    std::ostream& output(std::ostream& out) const;
};

std::ostream &operator<<(std::ostream &out, const Position &p);
#endif