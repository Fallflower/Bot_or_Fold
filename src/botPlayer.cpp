#include "botPlayer.h"
#include <algorithm>
#include <cstdlib>
#include <random>

std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
std::uniform_int_distribution<int> intDist(0, 99);

int getRandInt() {
    return intDist(gen);
}

ACTION BotPlayer::makeAction(const int& chipsToCall, int &betAmount) {
    int pot = betAmount;                    // 传入时是底池大小
    // 计算odd并化为百分数
    double potOdds = 100.0 * chipsToCall / (pot + chipsToCall);

    int r = getRandInt();

    // 无人下注时偶尔诈唬，牌力够强时主动找价值
    if (chipsToCall == 0) {
        if (r < 8 || equity > 60) {
            betAmount = (int)(pot * 0.7);
            if (betAmount >= chips) {
                betAmount = chips;
                setChips(0);
                return ALLIN;
            }
            decChips(betAmount);
            return BET;
        }
        return CHECK;
    }
    if (equity > 60) {
        // 强牌: 80%加注, 20%跟注
        if (chips <= chipsToCall) { // 筹码不足果断allin
            betAmount = chips;
            setChips(0);
            return ALLINTOCALL;
        }
        if (r < 80) {
            betAmount = chipsToCall * 3;
            if (betAmount >= chips) {
                betAmount = chips;
                setChips(0);
                return ALLIN;
            }
            decChips(betAmount);
            return RAISE;
        }
        decChips(chipsToCall);
        return CALL;
    }

    if (equity > potOdds + 5) {
        // 有利可图: 跟注为主, 15%半诈唬
        if (chips <= chipsToCall) return FOLD; // 筹码不足果断fold
        if (r < 15) {
            betAmount = chipsToCall * 3;
            if (betAmount >= chips) {
                betAmount = chips;
                setChips(0);
                return ALLIN;
            }
            decChips(betAmount);
            return RAISE;
        }
        if (chips <= chipsToCall) {
            betAmount = chips;
            setChips(0);
            return ALLINTOCALL;
        }
        decChips(chipsToCall);
        return CALL;
    }

    if (equity < potOdds) {
        // 不好摊牌，偶尔bluff
        if (chips <= chipsToCall) return FOLD; // 筹码不足果断fold
        if (r < 10) {
            betAmount = chipsToCall * 4;
            if (betAmount >= chips) {
                betAmount = chips;
                setChips(0);
                return ALLIN;
            }
            decChips(betAmount);
            return RAISE;
        }
    }
    return FOLD;
}
