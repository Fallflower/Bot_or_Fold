#include "botPlayer.h"
#include "gameLog.h"
#include <algorithm>
#include <chrono>
#include <random>

static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
static std::uniform_int_distribution<int> dist100(0, 99);
static int pct() { return dist100(rng); }

// ---- Preflop hand scoring ----
template<typename NumT>
int BotPlayer<NumT>::calcPreflopScore(const gameInfo<NumT>& info) const {
    const auto& cards = info.handCards;
    if (cards.size() < 2) return 0;

    int r1 = static_cast<int>(cards[0].getNum());
    int r2 = static_cast<int>(cards[1].getNum());
    if (r1 < r2) std::swap(r1, r2);
    int score = r1 + r2;

    if (r1 == r2) {
        score += 8;                         // pair bonus
    } else {
        if (cards[0].getSuit() == cards[1].getSuit()) score += 2;  // suited bonus
        if (r1 - r2 <= 2) score += 1;       // connected / 1-gap
    }
    return score;
}

template<typename NumT>
HandTier BotPlayer<NumT>::getTier(int score) const {
    if (score >= 24) return TIER_PREMIUM;
    if (score >= 17) return TIER_STRONG;
    if (score >= 11) return TIER_PLAYABLE;
    if (score >= 7)  return TIER_MARGINAL;
    return TIER_TRASH;
}

// ============================================================
//                   Entry point
// ============================================================
template<typename NumT>
ACTION BotPlayer<NumT>::makeAction(const gameInfo<NumT>& info, int &betAmount) {
    if (g_log) g_log->write(this->getName());
    if (info.stateCode == 0) {
        if (g_log) g_log->writeLine(" 进入翻牌前决策");
        return actPreflop(info, betAmount);
    }
    if (g_log) g_log->writeLine(" 进入翻牌后决策");
    return actPostflop(info, betAmount);
}

// ============================================================
//                   Preflop decision tree
// ============================================================
template<typename NumT>
ACTION BotPlayer<NumT>::actPreflop(const gameInfo<NumT>& info, int &betAmount) {
    // ---- Abstract information ----
    int score = calcPreflopScore(info);
    HandTier tier = getTier(score);
    int ctc = info.chipsToCall;
    bool lp = info.positionStr == " C O " || info.positionStr == "  D  ";
    bool sb = info.positionStr == " S B ";
    bool bb = info.positionStr == " B B ";

    // ---- Scenario A: can check for free (BB with no raise) ----
    if (ctc == 0) {
        if (g_log) g_log->writeLine("\t[决策] 翻牌前-场景A：可免费过牌，当前tier=" + std::to_string(tier));
        if (tier >= TIER_STRONG) {
            if (g_log) g_log->writeLine("\t\t[决策] → 手牌强（tier≥STRONG），加注逼价值");
            betAmount = info.pot + 4;
            return RAISE;
        }
        if (g_log) g_log->writeLine("\t\t[决策] → 手牌弱，过牌");
        return CHECK;
    }

    // ---- Scenario B: facing blinds (raiseCount == 0, ctc > 0) ----
    if (info.raiseCount == 0) {
        if (g_log) g_log->writeLine("\t[决策] 翻牌前-场景B：面对盲注（无人加注），ctc=" + std::to_string(ctc) + " tier=" + std::to_string(tier));
        if (tier >= TIER_PREMIUM) {
            if (g_log) g_log->writeLine("\t\t[决策] → 顶级牌（PREMIUM），4倍加注" + std::string(lp ? "（有利位置追加2）" : ""));
            betAmount = ctc * 4 + (lp ? 2 : 0);

            return RAISE;
        }
        if (tier >= TIER_STRONG) {
            if (lp) {
                if (g_log) g_log->writeLine("\t\t[决策] → 强牌+有利位置，3倍加注+2");
                betAmount = ctc * 3 + 2;
                return RAISE;
            }
            if (g_log) g_log->writeLine("\t\t[决策] → 强牌+不利位置，平跟");
            return CALL;
        }
        if (tier >= TIER_PLAYABLE) {
            if (bb) {
                if (g_log) g_log->writeLine("\t\t[决策] → 可玩牌+大盲，过牌");
                return CHECK;
            }
            if (sb) {
                if (g_log) g_log->writeLine("\t\t[决策] → 可玩牌+小盲，补齐");
                return CALL;
            }
            if (lp && pct() < 35) {
                if (g_log) g_log->writeLine("\t\t[决策] → 可玩牌+有利位置+35%偷鸡，加注");
                betAmount = ctc * 3 + 2;
                return RAISE;
            }
            if (g_log) g_log->writeLine("\t\t[决策] → 可玩牌但位置/概率不合适，弃牌");
            return FOLD;
        }
        if (tier >= TIER_MARGINAL) {
            if (bb && pct() < 45) {
                if (g_log) g_log->writeLine("\t\t[决策] → 边缘牌+大盲+45%概率，过牌");
                return CHECK;
            }
            if (sb && pct() < 25) {
                if (g_log) g_log->writeLine("\t\t[决策] → 边缘牌+小盲+25%概率，补齐");
                return CALL;
            }
        }
        if (g_log) g_log->writeLine("\t\t[决策] → 牌力不足，弃牌");
        return FOLD;
    }

    // ---- Scenario C: facing ONE raise (raiseCount == 1) ----
    if (info.raiseCount == 1) {
        int potOdds = (info.pot + ctc > 0) ? 100 * ctc / (info.pot + ctc) : 0;
        if (g_log) g_log->writeLine("\t[决策] 翻牌前-场景C：面对一次加注，potOdds=" + std::to_string(potOdds) + "% tier=" + std::to_string(tier));

        if (tier >= TIER_PREMIUM) {
            if (g_log) g_log->writeLine("\t\t[决策] → 顶级牌，3倍反加");
            betAmount = ctc * 3;
            return RAISE;
        }
        if (tier >= TIER_STRONG && potOdds < 40) {
            if (g_log) g_log->writeLine("\t\t[决策] → 强牌且底池赔率合适（<40%），平跟");
            return CALL;
        }
        if (tier >= TIER_PLAYABLE && potOdds < 30 && (lp || bb)) {
            if (g_log) g_log->writeLine("\t\t[决策] → 可玩牌+好赔率+位置佳，平跟");
            return CALL;
        }
        if (g_log) g_log->writeLine("\t\t[决策] → 牌力/赔率不足，弃牌");
        return FOLD;
    }

    // ---- Scenario D: facing 3-bet or more (raiseCount >= 2) ----
    if (g_log) g_log->writeLine("\t[决策] 翻牌前-场景D：面对3-bet及以上，tier=" + std::to_string(tier));
    if (tier >= TIER_PREMIUM && pct() < 50) {
        if (g_log) g_log->writeLine("\t\t[决策] → 顶级牌+50%概率，平跟慢打");
        return CALL;
    }
    if (g_log) g_log->writeLine("\t\t[决策] → 牌力不足/运气不佳，弃牌");
    return FOLD;
}

// ============================================================
//                   Postflop decision tree
// ============================================================
template<typename NumT>
ACTION BotPlayer<NumT>::actPostflop(const gameInfo<NumT>& info, int &betAmount) {
    int ctc = info.chipsToCall;
    int pot = info.pot;
    double wr = info.winRate;
    bool lp = info.positionStr == " C O " || info.positionStr == "  D  ";

    // ---- Checked to me ----
    if (ctc == 0) {
        if (g_log) g_log->writeLine("\t[决策] 翻牌后-场景：无人下注，wr=" + std::to_string((int)wr) + "%");
        if (wr > 80) {
            if (g_log) g_log->writeLine("\t\t[决策] → 胜率>80%，重注75%底池");
            int betSz = std::max((int)(pot * 0.75), 6);
            betAmount = betSz; return RAISE;
        }
        if (wr > 60) {
            if (g_log) g_log->writeLine("\t\t[决策] → 胜率>60%，中注60%底池");
            int betSz = std::max((int)(pot * 0.6), 6);
            betAmount = betSz; return RAISE;
        }
        if (wr > 40) {
            if (lp && pct() < 30) {
                if (g_log) g_log->writeLine("\t\t[决策] → 胜率>40%+有利位置+30%概率，轻注40%");
                int betSz = std::max((int)(pot * 0.4), 6);
                betAmount = betSz; return RAISE;
            }
            if (g_log) g_log->writeLine("\t\t[决策] → 胜率>40%但条件不足，过牌");
            return CHECK;
        }
        if (wr > 25) {
            if (pct() < 12) {
                if (g_log) g_log->writeLine("\t\t[决策] → 胜率>25%+12%偷鸡，诈唬50%");
                int betSz = std::max((int)(pot * 0.5), 6);
                betAmount = betSz; return RAISE;
            }
            if (g_log) g_log->writeLine("\t\t[决策] → 胜率>25%但没偷鸡，过牌");
            return CHECK;
        }
        if (pct() < 8) {
            if (g_log) g_log->writeLine("\t\t[决策] → 胜率低+8%纯诈唬");
            int betSz = std::max((int)(pot * 0.6), 6);
            betAmount = betSz; return RAISE;
        }
        if (g_log) g_log->writeLine("\t\t[决策] → 胜率低+没诈唬，过牌");
        return CHECK;
    }

    // ---- Facing a bet ----
    int potOdds = (pot + ctc > 0) ? 100 * ctc / (pot + ctc) : 0;
    if (g_log) g_log->writeLine("\t[决策] 翻牌后-场景：面对下注，wr=" + std::to_string((int)wr) + "% potOdds=" + std::to_string(potOdds) + "%");

    if (wr > 60) {
        if (pct() < 70) {
            if (g_log) g_log->writeLine("\t\t[决策] → 胜率>60%+70%概率，反加诈唬");
            betAmount = ctc * 3 + (int)(pot * 0.5);
            return RAISE;
        }
        if (g_log) g_log->writeLine("\t\t[决策] → 胜率>60%但运气不佳，平跟");
        return CALL;
    }

    if (wr > potOdds + 5) {
        if (g_log) g_log->writeLine("\t\t[决策] → 胜率高于底池赔率+5%，正期望平跟");
        return CALL;
    }

    if (wr > potOdds - 8) {
        if (pct() < 10 && lp) {
            if (g_log) g_log->writeLine("\t\t[决策] → 胜率略低于赔率+有利位置+10%概率，诈唬反加");
            betAmount = ctc * 3;
            return RAISE;
        }
        if (g_log) g_log->writeLine("\t\t[决策] → 胜率略低于赔率但接近，平跟");
        return CALL;
    }

    if (pct() < 6) {
        if (g_log) g_log->writeLine("\t\t[决策] → 胜率差+6%纯诈唬");
        betAmount = ctc * 3;
        return RAISE;

    }
    if (g_log) g_log->writeLine("\t\t[决策] → 无利可图，弃牌");
    return FOLD;
}

template class BotPlayer<CARDNUM>;
template class BotPlayer<SHORT_CARDNUM>;
