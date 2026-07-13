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
    // r1 >= r2 according to deck::deal

    bool isPair = (r1 == r2);
    bool suited = (cards[0].getSuit() == cards[1].getSuit());
    // Chen Formula:
    // Step 1: Score highest card only
    double score;
    if (r1 == 12)      score = 10;     // A
    else if (r1 == 11) score = 8;      // K
    else if (r1 == 10) score = 7;      // Q
    else if (r1 == 9)  score = 6;      // J
    else               score = (r1 + 2) * 0.5;  // 10 to 2: half of card value

    // Step 2: Pairs — multiply by 2, minimum 5
    if (isPair) {
        score *= 2;
        if (score < 5) score = 5;
        return static_cast<int>(score + 0.5);
    }

    // Step 3: +2 if suited
    if (suited) score += 2;

    // Step 4 & 5: Gap penalty and Straight bonus
    int gap = r1 - r2 - 1;
    if (gap == 0)        score += 1;
    else if (gap == 2)   score -= 2;
    else if (gap == 3)   score -= 4;
    else if (gap >= 4)   score -= 5;

    // Step 6: Round half point scores up
    return static_cast<int>(score + 0.5);
}

template<typename NumT>
HandTier BotPlayer<NumT>::getTier(int score) const {
    if (score >= 13) return TIER_PREMIUM;   // AA, KK, QQ, AKs
    if (score >= 8)  return TIER_STRONG;    // above AJo
    if (score >= 5)  return TIER_PLAYABLE;  // above 43s, 53s, A2o, 
    if (score >= 3)  return TIER_MARGINAL;
    return TIER_TRASH;
}

/* ============================================================
 *                   Entry point
 * ============================================================
 * Fold/Call操作不用动betAmount
 * Raise操作需要设置betAmount为加注金额
*/
template<typename NumT>
ACTION BotPlayer<NumT>::makeAction(const gameInfo<NumT>& info, int &betAmount) {
    if (g_log) g_log->write(this->getName());
    bool mw = info.remainPlayerNum > 2;
    switch (info.stateCode) {
    case 0:
        if (g_log) g_log->writeLine(" 进入翻前决策");
        return actPreflop(info, betAmount, mw);
    case 1:
        if (g_log) g_log->writeLine(" 进入翻牌决策");
        return actFlop(info, betAmount, mw);
    case 2:
        if (g_log) g_log->writeLine(" 进入转牌决策");
        return actTurn(info, betAmount, mw);
    case 3:
        if (g_log) g_log->writeLine(" 进入河牌决策");
        return actRiver(info, betAmount, mw);
    default:
        return FOLD;
    }
}

// ============================================================
//                   Preflop decision tree
// ============================================================
template<typename NumT>
ACTION BotPlayer<NumT>::actPreflop(const gameInfo<NumT>& info, int &betAmount, bool mw) {
    // ---- Abstract information ----
    int score = calcPreflopScore(info);
    HandTier tier = getTier(score);
    int ctc = info.chipsToCall;
    bool lp = info.positionStr == " C O " || info.positionStr == "  D  ";
    bool sb = info.positionStr == " S B ";
    bool bb = info.positionStr == " B B ";

    // ---- Scenario A: can check for free (BB with no raise) ----
    if (ctc == 0) {
        if (g_log) g_log->writeLine("\t[决策] 翻前-场景A：可免费过牌，当前tier=" + std::to_string(tier));
        if (tier >= TIER_PREMIUM) {
            if (g_log) g_log->writeLine("\t\t[决策] → 手牌顶级（tier≥PREMIUM），加注隔离");
            betAmount = info.pot + 4;
            return RAISE;
        }
        if (tier >= TIER_STRONG && pct() < 30) {
            if (g_log) g_log->writeLine("\t\t[决策] → 手牌强（tier≥STRONG），30%频率加注隔离");
            betAmount = info.pot + 4;
            return RAISE;
        }
        if (g_log) g_log->writeLine("\t\t[决策] → 手牌弱，过牌");
        return CALL;
    }

    // ---- Scenario B: facing blinds (raiseCount == 0, ctc > 0) ----
    if (info.raiseCount == 0) {
        if (g_log) g_log->writeLine("\t[决策] 翻前-场景B：面对盲注（无人加注），ctc=" + std::to_string(ctc) + " tier=" + std::to_string(tier) + " 多人底池=" + (mw ? "是" : "否"));
        if (tier >= TIER_PREMIUM) {
            if (g_log) g_log->writeLine("\t\t[决策] → 顶级牌（PREMIUM），" + std::string(lp ? "4倍" : "3倍") + "加注");
            betAmount = ctc * (3 + (lp ? 1 : 0)) + 2;
            return RAISE;
        }
        if (tier >= TIER_STRONG) {
            if (lp) {
                if (g_log) g_log->writeLine("\t\t[决策] → 强牌+有利位置，3倍加注+2");
                betAmount = ctc * 3 + 2;
                return RAISE;
            } else {
                if (pct() < 30) {
                    if (g_log) g_log->writeLine("\t\t[决策] → 强牌+不利位置，30%频率加注");
                    betAmount = ctc * 3 + 2;
                    return RAISE;
                }
            }
            if (g_log) g_log->writeLine("\t\t[决策] → 强牌+不利位置，平跟");
            return CALL;
        }
        if (tier >= TIER_PLAYABLE) {
            if (bb) {
                if (g_log) g_log->writeLine("\t\t[决策] → 可玩牌+大盲，过牌");
                return CALL;
            }
            if (sb) {
                if (mw && pct() < 85) {
                    if (g_log) g_log->writeLine("\t\t[决策] → 可玩牌+小盲+多人池+85%弃牌");
                    return FOLD;
                }
                if (g_log) g_log->writeLine("\t\t[决策] → 可玩牌+小盲，补齐");
                return CALL;
            }
            if (lp) {
                int stealPct = mw ? 20 : 35;
                if (pct() < stealPct) {
                    if (g_log) g_log->writeLine("\t\t[决策] → 可玩牌+有利位置+" + std::to_string(stealPct) + "%偷鸡，加注");
                    betAmount = ctc * 3 + 2;
                    return RAISE;
                }
            }
            if (g_log) g_log->writeLine("\t\t[决策] → 可玩牌但位置/概率不合适，弃牌");
            return FOLD;
        }
        if (tier >= TIER_MARGINAL) {
            if (bb) {
                int defPct = mw ? 30 : 45;
                if (pct() < defPct) {
                    if (g_log) g_log->writeLine("\t\t[决策] → 边缘牌+大盲+" + std::to_string(defPct) + "%概率，过牌");
                    return CALL;
                }
            }
            if (sb) {
                int callPct = mw ? 15 : 55;
                if (pct() < callPct) {
                    if (g_log) g_log->writeLine("\t\t[决策] → 边缘牌+小盲+" + std::to_string(callPct) + "%概率，补齐");
                    return CALL;
                }
            }
        }
        if (g_log) g_log->writeLine("\t\t[决策] → 牌力不足，弃牌");
        return FOLD;
    }

    // ---- Scenario C: facing ONE raise (raiseCount == 1) ----
    if (info.raiseCount == 1) {
        if (g_log) g_log->writeLine("\t[决策] 翻前-场景C：面对一次加注，tier=" + std::to_string(tier) + " 多人底池=" + (mw ? "是" : "否"));

        if (tier >= TIER_PREMIUM) {
            if (g_log) g_log->writeLine("\t\t[决策] → 顶级牌，3倍反加");
            betAmount = ctc * 3 + 2;
            return RAISE;
        }
        if (tier >= TIER_STRONG) {
            int raiseFreq = mw ? 15 : 40;
            if (pct() < raiseFreq) {
                if (g_log) g_log->writeLine("\t\t[决策] → 强牌, " + std::to_string(raiseFreq) + "%频率加注");
                betAmount = ctc * 3 + 2;
                return RAISE;
            }
            return CALL;
        }
        if (tier >= TIER_PLAYABLE) {
            int callFreq = mw ? 30 : 70;
            if (pct() < callFreq) {
                if (g_log) g_log->writeLine("\t\t[决策] → 可玩牌" + std::to_string(callFreq) + "%频率平跟");
                return CALL;
            }
        }
        if (g_log) g_log->writeLine("\t\t[决策] → 牌力不足，弃牌");
        return FOLD;
    }

    // ---- Scenario D: facing 3-bet or more (raiseCount >= 2) ----
    if (g_log) g_log->writeLine("\t[决策] 翻前-场景D：面对3-bet及以上，tier=" + std::to_string(tier) + " 多人底池=" + (mw ? "是" : "否"));
    if (tier >= TIER_PREMIUM) {
        int raiseFreq = mw ? 80 : 50;
        if (pct() < raiseFreq) {
            if (g_log) g_log->writeLine("\t\t[决策] → 顶级牌+" + std::to_string(raiseFreq) + "%频率，3倍反加");
            betAmount = info.chipsToCall * 3 + 2;
            return RAISE;
        }
        if (g_log) g_log->writeLine("\t\t[决策] → 顶级牌，平跟");
        return CALL;
    }
    if (tier >= TIER_STRONG) {
        int callFreq = mw ? 20 : 60;
        if (pct() < callFreq) {
            if (g_log) g_log->writeLine("\t\t[决策] → 强牌+" + std::to_string(callFreq) + "%频率，平跟买牌");
            return CALL;
        }
    }
    if (g_log) g_log->writeLine("\t\t[决策] → 牌力不足/考虑频率因素，最终弃牌");
    return FOLD;
}

// ============================================================
//                   Flop decision tree
// ============================================================
template<typename NumT>
ACTION BotPlayer<NumT>::actFlop(const gameInfo<NumT>& info, int &betAmount, bool mw) {
    int ctc = info.chipsToCall;
    int pot = info.pot;
    double wr = info.winRate;

    if (g_log) g_log->writeLine("\t[决策] 翻牌：ctc=" + std::to_string(ctc) + " wr=" + std::to_string((int)wr) + "% 多人底池=" + (mw ? "是" : "否"));

    if (ctc == 0) {
        if (mw) {
            if (wr > 70) {
                if (g_log) g_log->writeLine("\t\t[决策] → wr>70% 多人池，价值下注50%");
                betAmount = std::max((int)(pot * 0.5), 6);
                return RAISE;
            }
            if (pct() < 6) {
                if (g_log) g_log->writeLine("\t\t[决策] → 6%概率半诈唬40%");
                betAmount = std::max((int)(pot * 0.4), 6);
                return RAISE;
            }
            if (g_log) g_log->writeLine("\t\t[决策] → 过牌");
            return CALL;
        } else {
            if (wr > 55) {
                if (g_log) g_log->writeLine("\t\t[决策] → wr>55%，价值下注60%");
                betAmount = std::max((int)(pot * 0.6), 6);
                return RAISE;
            }
            if (wr > 40) {
                if (pct() < 30) {
                    if (g_log) g_log->writeLine("\t\t[决策] → wr>40%+30%持续下注45%");
                    betAmount = std::max((int)(pot * 0.45), 6);
                    return RAISE;
                }
                if (g_log) g_log->writeLine("\t\t[决策] → wr>40%+概率不足，过牌");
                return CALL;
            }
            if (wr > 25 && pct() < 12) {
                if (g_log) g_log->writeLine("\t\t[决策] → wr>25%+12%半诈唬50%");
                betAmount = std::max((int)(pot * 0.5), 6);
                return RAISE;
            }
            if (pct() < 8) {
                if (g_log) g_log->writeLine("\t\t[决策] → 8%纯诈唬60%");
                betAmount = std::max((int)(pot * 0.6), 6);
                return RAISE;
            }
            if (g_log) g_log->writeLine("\t\t[决策] → 过牌");
            return CALL;
        }
    }

    // ---- Facing a bet ----
    int potOdds = (pot + ctc > 0) ? 100 * ctc / (pot + ctc) : 0;
    if (g_log) g_log->writeLine("\t[决策] 翻牌-面对下注: potOdds=" + std::to_string(potOdds) + "%");

    if (mw) {
        if (wr > 70) {
            if (pct() < 50) {
                if (g_log) g_log->writeLine("\t\t[决策] → wr>70%+50%概率，价值加注");
                betAmount = ctc * 3 + (int)(pot * 0.5);
                return RAISE;
            }
            if (g_log) g_log->writeLine("\t\t[决策] → wr>70%+概率未中，平跟");
            return CALL;
        }
        if (wr > potOdds + 12) {
            if (g_log) g_log->writeLine("\t\t[决策] → 赔率好（+12%），平跟");
            return CALL;
        }
        if (wr > potOdds + 5 && pct() < 15) {
            if (g_log) g_log->writeLine("\t\t[决策] → 略低于高赔率门槛+15%概率，诈唬加注(增加raise频率)");
            betAmount = ctc * 3;
            return RAISE;
        }
        if (pct() < 3) {
            if (g_log) g_log->writeLine("\t\t[决策] → 3%纯诈唬");
            betAmount = ctc * 3;
            return RAISE;
        }
        if (g_log) g_log->writeLine("\t\t[决策] → 多人池防守不足，弃牌");
        return FOLD;
    } else {
        if (wr > 60) {
            if (pct() < 70) {
                if (g_log) g_log->writeLine("\t\t[决策] → wr>60%+70%概率，反加");
                betAmount = ctc * 3 + (int)(pot * 0.5);
                return RAISE;
            }
            if (g_log) g_log->writeLine("\t\t[决策] → wr>60%+概率未中，平跟");
            return CALL;
        }
        if (wr > potOdds + 5) {
            if (g_log) g_log->writeLine("\t\t[决策] → 正期望赔率，平跟");
            return CALL;
        }
        if (wr > potOdds - 8 && pct() < 10) {
            if (g_log) g_log->writeLine("\t\t[决策] → 略低赔率+10%诈唬加注");
            betAmount = ctc * 3;
            return RAISE;
        }
        if (pct() < 6) {
            if (g_log) g_log->writeLine("\t\t[决策] → 6%纯诈唬");
            betAmount = ctc * 3;
            return RAISE;
        }
        if (g_log) g_log->writeLine("\t\t[决策] → 无利可图，弃牌");
        return FOLD;
    }
}

// ============================================================
//                   Turn decision tree
// ============================================================
template<typename NumT>
ACTION BotPlayer<NumT>::actTurn(const gameInfo<NumT>& info, int &betAmount, bool mw) {
    int ctc = info.chipsToCall;
    int pot = info.pot;
    double wr = info.winRate;

    if (g_log) g_log->writeLine("\t[决策] 转牌：ctc=" + std::to_string(ctc) + " wr=" + std::to_string((int)wr) + "% 多人底池=" + (mw ? "是" : "否"));

    if (ctc == 0) {
        if (mw) {
            if (wr > 75) {
                if (g_log) g_log->writeLine("\t\t[决策] → wr>75% 多人池，价值下注60%");
                betAmount = std::max((int)(pot * 0.6), 6);
                return RAISE;
            }
            if (pct() < 4) {
                if (g_log) g_log->writeLine("\t\t[决策] → 4%概率诈唬50%");
                betAmount = std::max((int)(pot * 0.5), 6);
                return RAISE;
            }
            if (g_log) g_log->writeLine("\t\t[决策] → 过牌");
            return CALL;
        } else {
            if (wr > 60) {
                if (g_log) g_log->writeLine("\t\t[决策] → wr>60%，价值下注65%");
                betAmount = std::max((int)(pot * 0.65), 6);
                return RAISE;
            }
            if (wr > 50) {
                if (pct() < 20) {
                    if (g_log) g_log->writeLine("\t\t[决策] → wr>50%+20%持续下注50%");
                    betAmount = std::max((int)(pot * 0.5), 6);
                    return RAISE;
                }
                return CALL;
            }
            if (pct() < 8) {
                if (g_log) g_log->writeLine("\t\t[决策] → 8%诈唬55%");
                betAmount = std::max((int)(pot * 0.55), 6);
                return RAISE;
            }
            if (pct() < 5) {
                if (g_log) g_log->writeLine("\t\t[决策] → 5%纯诈唬60%");
                betAmount = std::max((int)(pot * 0.6), 6);
                return RAISE;
            }
            if (g_log) g_log->writeLine("\t\t[决策] → 过牌");
            return CALL;
        }
    }

    // ---- Facing a bet on Turn ----
    int potOdds = (pot + ctc > 0) ? 100 * ctc / (pot + ctc) : 0;
    if (g_log) g_log->writeLine("\t[决策] 转牌-面对下注: potOdds=" + std::to_string(potOdds) + "%");

    if (mw) {
        if (wr > 75) {
            if (pct() < 40) {
                if (g_log) g_log->writeLine("\t\t[决策] → wr>75%+40%概率，价值加注");
                betAmount = ctc * 3 + (int)(pot * 0.5);
                return RAISE;
            }
            return CALL;
        }
        if (wr > potOdds + 10) {
            if (g_log) g_log->writeLine("\t\t[决策] → 赔率好（+10%），平跟");
            return CALL;
        }
        if (wr > potOdds + 3 && pct() < 12) {
            if (g_log) g_log->writeLine("\t\t[决策] → 略低高门槛+12%概率，诈唬加注(增加raise频率)");
            betAmount = ctc * 3;
            return RAISE;
        }
        if (pct() < 2) {
            if (g_log) g_log->writeLine("\t\t[决策] → 2%纯诈唬");
            betAmount = ctc * 3;
            return RAISE;
        }
        if (g_log) g_log->writeLine("\t\t[决策] → 多人池无利可图，弃牌");
        return FOLD;
    } else {
        if (wr > 60) {
            if (pct() < 65) {
                if (g_log) g_log->writeLine("\t\t[决策] → wr>60%+65%概率，反加");
                betAmount = ctc * 3 + (int)(pot * 0.5);
                return RAISE;
            }
            return CALL;
        }
        if (wr > potOdds + 3) {
            if (g_log) g_log->writeLine("\t\t[决策] → 正期望赔率+3%，平跟");
            return CALL;
        }
        if (wr > potOdds - 5 && pct() < 8) {
            if (g_log) g_log->writeLine("\t\t[决策] → 略低赔率+8%诈唬加注");
            betAmount = ctc * 3;
            return RAISE;
        }
        if (pct() < 5) {
            if (g_log) g_log->writeLine("\t\t[决策] → 5%纯诈唬");
            betAmount = ctc * 3;
            return RAISE;
        }
        if (g_log) g_log->writeLine("\t\t[决策] → 无利可图，弃牌");
        return FOLD;
    }
}

// ============================================================
//                   River decision tree
// ============================================================
template<typename NumT>
ACTION BotPlayer<NumT>::actRiver(const gameInfo<NumT>& info, int &betAmount, bool mw) {
    int ctc = info.chipsToCall;
    int pot = info.pot;
    double wr = info.winRate;

    if (g_log) g_log->writeLine("\t[决策] 河牌：ctc=" + std::to_string(ctc) + " wr=" + std::to_string((int)wr) + "% 多人底池=" + (mw ? "是" : "否"));

    if (ctc == 0) {
        if (mw) {
            if (wr > 80) {
                if (g_log) g_log->writeLine("\t\t[决策] → wr>80% 多人池，价值下注70%");
                betAmount = std::max((int)(pot * 0.7), 6);
                return RAISE;
            }
            if (wr > 60 && pct() < 15) {
                if (g_log) g_log->writeLine("\t\t[决策] → wr>60%+15%薄价值下注50%");
                betAmount = std::max((int)(pot * 0.5), 6);
                return RAISE;
            }
            if (pct() < 2) {
                if (g_log) g_log->writeLine("\t\t[决策] → 2%纯诈唬30%");
                betAmount = std::max((int)(pot * 0.3), 6);
                return RAISE;
            }
            if (g_log) g_log->writeLine("\t\t[决策] → 过牌");
            return CALL;
        } else {
            if (wr > 65) {
                if (g_log) g_log->writeLine("\t\t[决策] → wr>65%，价值下注70%");
                betAmount = std::max((int)(pot * 0.7), 6);
                return RAISE;
            }
            if (wr > 50) {
                if (pct() < 25) {
                    if (g_log) g_log->writeLine("\t\t[决策] → wr>50%+25%薄价值下注55%");
                    betAmount = std::max((int)(pot * 0.55), 6);
                    return RAISE;
                }
                return CALL;
            }
            if (pct() < 5) {
                if (g_log) g_log->writeLine("\t\t[决策] → 5%纯诈唬40%");
                betAmount = std::max((int)(pot * 0.4), 6);
                return RAISE;
            }
            if (g_log) g_log->writeLine("\t\t[决策] → 过牌");
            return CALL;
        }
    }

    // ---- Facing a bet on River ----
    int potOdds = (pot + ctc > 0) ? 100 * ctc / (pot + ctc) : 0;
    if (g_log) g_log->writeLine("\t[决策] 河牌-面对下注: potOdds=" + std::to_string(potOdds) + "%");

    if (mw) {
        if (wr > 80) {
            if (pct() < 30) {
                if (g_log) g_log->writeLine("\t\t[决策] → wr>80%+30%概率，价值加注");
                betAmount = ctc * 3 + (int)(pot * 0.5);
                return RAISE;
            }
            return CALL;
        }
        if (wr > potOdds + 8) {
            if (g_log) g_log->writeLine("\t\t[决策] → 赔率足够好（+8%），平跟");
            return CALL;
        }
        if (wr > potOdds + 2 && pct() < 10) {
            if (g_log) g_log->writeLine("\t\t[决策] → 接近赔率+10%概率，诈唬加注(增加raise频率)");
            betAmount = ctc * 3;
            return RAISE;
        }
        if (pct() < 1) {
            if (g_log) g_log->writeLine("\t\t[决策] → 1%纯诈唬");
            betAmount = ctc * 3;
            return RAISE;
        }
        if (g_log) g_log->writeLine("\t\t[决策] → 多人池河牌无利可图，弃牌");
        return FOLD;
    } else {
        if (wr > 60) {
            if (pct() < 60) {
                if (g_log) g_log->writeLine("\t\t[决策] → wr>60%+60%概率，反加");
                betAmount = ctc * 3 + (int)(pot * 0.5);
                return RAISE;
            }
            return CALL;
        }
        if (wr > potOdds) {
            if (g_log) g_log->writeLine("\t\t[决策] → 正期望赔率，平跟");
            return CALL;
        }
        if (wr > potOdds - 5 && pct() < 6) {
            if (g_log) g_log->writeLine("\t\t[决策] → 略低赔率+6%诈唬加注");
            betAmount = ctc * 3;
            return RAISE;
        }
        if (pct() < 4) {
            if (g_log) g_log->writeLine("\t\t[决策] → 4%纯诈唬");
            betAmount = ctc * 3;
            return RAISE;
        }
        if (g_log) g_log->writeLine("\t\t[决策] → 河牌无利可图，弃牌");
        return FOLD;
    }
}

template class BotPlayer<CARDNUM>;
template class BotPlayer<SHORT_CARDNUM>;
