#ifndef __GAME_H__
#define __GAME_H__
#include "handType.h"
#include "handType_.h"
#include "deck.h"
#include "position.h"
#include "humanPlayer.h"
#include "botPlayer.h"
#include "gameLog.h"
#include "gameState.h"
#include <memory>
#include <iostream>

extern const std::string stateStr[];

template<typename NumT = CARDNUM>
class Game
{
private:
    std::vector<std::vector<Card<NumT>>> hands;
    Deck<NumT> deck_;

    int playerNum;
    std::vector<std::unique_ptr<Player<NumT>>> players;
    int inic;       // initial chips
    int hpi;        // humanPlayerIndex
    int dealer;
    int stateCode;  // 0, 1, 2, 3, 4
    int commit[4];  // Chips commitment of each round (2 means big blind)
    Position pos;

    int active;     // index: active player
    int lastBet;    // index: the last player who bet
    int **chips;    // [playerNum, 4] chips commitment of each player at each round
    bool *ftag;     // fold tags
    bool *ctag;     // check tags
    bool *atag;     // all-in tags
    int raiseCount; // 当前轮的加注次数（0=none\blind, 1=open, 2=3-bet, ...)
    bool roundSettled = false;
    RoundResult roundResult_;

    void init_game();
    void reset_tags();
    void init_players(const HumanPlayer<NumT>&, const int&);
    void init_blinds();
    void checkState();
    void applyAction(const ActionCommand& command);
    void fold();
    void call(const int&);
    void bet(const int&);

    int getPlayerCommited(const int& pi) const {
        int sum = 0;
        for (int i = 0; i < 4; i++) sum += chips[pi][i];
        return sum;
    }

    void step() {   // move "active"
        active = (active + 1) % playerNum;
        while (ftag[active] || atag[active]) active = (active + 1) % playerNum;
    }

    const std::vector<Card<NumT>> getHands(const int&) const;   // 3/4/5 + 2
    const std::vector<Card<NumT>> getFinalHands(const int&) const;  // 5 + 2
    const std::vector<Card<NumT>> getKnownPubCards() const {        // 3/4/5
        std::vector<Card<NumT>> temp;
        for (auto c:deck_.getPubCards())
            if (c.show) temp.push_back(c);
        return temp;
    }

    double calcEquity(const int&, const int& = 12288) const;  // 计算玩家视角的胜率，返回百分数
    std::vector<double> calcWinRate(const int& simulations = 12288) const;  // 返回上帝视角所有玩家胜率百分数
    std::vector<int> checkWinner(const std::vector<std::vector<Card<NumT>>>& hands, const std::vector<Card<NumT>>& publicCards) const; // 判断特定输入的玩家手牌组合下的赢家
    std::vector<int> getWinners(const std::vector<int>& eligiblePlayers) const;  // 计算 eligiblePlayers 中的赢家
public:
    Game(int pn = 3, int d = 0);
    Game(const Position& posInfo, const int& initialChips, const HumanPlayer<NumT>& humanPlayer, const int &humanPlayerPosIndex);
    ~Game();

    void show(std::ostream& out = std::cout) const;
    void showPlayerView(std::ostream& out = std::cout) const;
    int getPot() const;
    int getChipsToCall() const { return isEnd() ? 0 : commit[stateCode] - chips[active][stateCode]; }
    int getState() const { return stateCode; }

    gameInfo<NumT> currentDecision() const;
    DecisionRequest decisionRequest() const;
    void submitAction(const ActionCommand& command);
    void advanceBot();
    bool isHumanTurn() const { return !isEnd() && active == hpi; }
    TableSnapshot snapshot() const;
    RoundResult settleRound();

    void toAct();
    void afterEnd();
    void nextRound();

    bool isEnd() const { return stateCode > 3; }
    Position getPosiInfo() const { return pos; }
    std::vector<SidePot> calculateSidePots() const;  // 计算所有边池（含主池）

};
#endif
