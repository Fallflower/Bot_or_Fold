#ifndef __POSITION_H__
#define __POSITION_H__
#include<iostream>
#include<string>
#include<vector>

/**
 * 根据 playerNum 初始化位置序列，完成后不会改动
 * 通过 dealer 建立逻辑索引到位置序列索引的映射。
 * 修改 dealer 达到位置轮换的目的。
 * pn=2 -> {SB, BB}
 * pn=3 -> {SB, BB, D}
 * pn=4 -> {SB, BB, UTG, D}
 * pn=5 -> {SB, BB, UTG, CO, D}
 * pn=6 -> {SB, BB, UTG, HJ, CO, D}
 * pn=7 -> {SB, BB, UTG, MP, HJ, CO, D}
 * pn>7 -> {SB, BB, UTG, UTG+1, ... , MP, HJ, CO, D}
 */
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

    bool isBlind(int pi) const;
    bool isLatePosition(int pi) const;

    std::ostream& output(std::ostream& out) const;
};

std::ostream &operator<<(std::ostream &out, const Position &p);
#endif