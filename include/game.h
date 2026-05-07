#ifndef __GAME_H__
#define __GAME_H__
#include "handType.h"
#include "handType_.h"
#include "deck.h"
#include "position.h"
#include "humanPlayer.h"
#include "botPlayer.h"
#include <memory>

extern const std::string stateStr[];

class Game
{
private:
    std::vector<std::vector<Card>> hands;
    Deck deck_;

    int playerNum;
    std::vector<std::unique_ptr<Player>> players;
    int hpi;        // humanPlayerIndex
    int dealer;
    int stateCode;  // 0, 1, 2, 3, 4
    int commit[4] = {2, 0, 0, 0};  // Chips commitment of each round (2 means big blind)
    Position pos;

    int active;     // index: active player
    int lastBet;    // index: the last player who bet
    int **chips;    // [playerNum, 4] chips commitment of each player at each round
    bool *ftag;     // fold tags
    bool *ctag;     // check tags
    bool *atag;     // all-in tags

    void init_game();
    void reset_game();
    void init_players(const HumanPlayer&, const int&);
    void checkState();

    int getPlayerCommited(const int& pi) const {
        int sum = 0;
        for (int i = 0; i < 4; i++) sum += chips[pi][i];
        return sum;
    }

    void step() {   // move "active"
        active = (active + 1) % playerNum;
        while (ftag[active] || atag[active]) active = (active + 1) % playerNum;
    }

    std::vector<Card> getHands(const int&) const;   // 3/4/5 + 2
    std::vector<Card> getFinalHands(const int&) const;  // 5 + 2

    double calcEquity(const int&, const int& = 12288) const;  // 计算玩家视角的胜率，返回百分数
    std::vector<double> calcWinRate(const int& simulations = 12288) const;  // 返回上帝视角所有玩家胜率百分数
    std::vector<int> checkWinner(const std::vector<std::vector<Card>>& hands, const std::vector<Card>& public_cards) const; // 判断特定输入的玩家手牌组合下的赢家，用于蒙特卡洛模拟
    std::vector<int> checkWinner(std::vector<Card> public_cards) const; // 判断特定输入的公共牌下的赢家，用于蒙特卡洛模拟
public:
    Game(int pn = 3, int d = 0);
    Game(const Position& posInfo, const int& initialChips, const HumanPlayer& humanPlayer, const int &humanPlayerPosIndex);
    ~Game();

    void show() const;
    void showPlayerView() const;
    int getPot() const;
    int getChipsToCall() const { return commit[stateCode] - chips[active][stateCode]; }
    int getState() const { return stateCode; }
    void fold();
    void call();     
    void bet(const int&);
    void allin(const int&);
    void allinToCall(const int&);

    void toAct();
    void afterEnd();
    void nextRound();

    bool isEnd() const { return stateCode == 4; }
    Player* getPlayer(const int& pi) const { return players[pi].get(); }
    Player* getActPlayer() const { return players[active].get(); }
    std::vector<int> checkWinner() const;

    Position getPosiInfo() const { return pos; }

};
#endif