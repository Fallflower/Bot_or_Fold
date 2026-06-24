#include "botPlayer.h"
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

template<typename NumT>
bool BotPlayer<NumT>::isLatePosition(const std::string& pos) const {
    return pos.find("D ") != std::string::npos || pos.find("C O") != std::string::npos;
}

template<typename NumT>
bool BotPlayer<NumT>::isBlind(const std::string& pos) const {
    return pos.find("SB") != std::string::npos || pos.find("BB") != std::string::npos;
}

// ============================================================
//                   Entry point
// ============================================================
template<typename NumT>
ACTION BotPlayer<NumT>::makeAction(const gameInfo<NumT>& info, int &betAmount) {
    if (info.stateCode == 0)
        return actPreflop(info, betAmount);
    return actPostflop(info, betAmount);
}

// ============================================================
//                   Preflop decision tree
// ============================================================
template<typename NumT>
ACTION BotPlayer<NumT>::actPreflop(const gameInfo<NumT>& info, int &betAmount) {
    int score = calcPreflopScore(info);
    HandTier tier = getTier(score);
    int ctc = info.chipsToCall;
    int myChips = this->chips;
    bool lp = isLatePosition(info.positionStr);
    bool sb = info.positionStr.find("SB") != std::string::npos;
    bool bb = info.positionStr.find("BB") != std::string::npos;

    // ---- Scenario A: can check for free (BB with no raise) ----
    if (ctc == 0) {
        if (tier >= TIER_STRONG) {
            int raiseAmt = std::min(info.pot + 4, myChips);
            if (raiseAmt >= myChips) { this->setChips(0); betAmount = myChips; return RAISE; }
            this->decChips(raiseAmt); betAmount = raiseAmt; return RAISE;
        }
        return CHECK;
    }

    // ---- Scenario B: facing blinds (raiseCount == 0, ctc > 0) ----
    if (info.raiseCount == 0) {
        if (tier >= TIER_PREMIUM) {
            int raiseAmt = std::min(ctc * 4 + (lp ? 2 : 0), myChips);
            if (raiseAmt >= myChips) { this->setChips(0); betAmount = myChips; return RAISE; }
            this->decChips(raiseAmt); betAmount = raiseAmt; return RAISE;
        }
        if (tier >= TIER_STRONG) {
            if (lp) {
                int raiseAmt = std::min(ctc * 3 + 2, myChips);
                if (raiseAmt >= myChips) { this->setChips(0); betAmount = myChips; return RAISE; }
                this->decChips(raiseAmt); betAmount = raiseAmt; return RAISE;
            }
            if (ctc >= myChips) { this->setChips(0); return CALL; }
            this->decChips(ctc); return CALL;
        }
        if (tier >= TIER_PLAYABLE) {
            if (bb) return CHECK;
            if (sb) { this->decChips(ctc); return CALL; }
            if (lp && pct() < 35) {
                int raiseAmt = std::min(ctc * 3 + 2, myChips);
                if (raiseAmt >= myChips) { this->setChips(0); betAmount = myChips; return RAISE; }
                this->decChips(raiseAmt); betAmount = raiseAmt; return RAISE;
            }
            return FOLD;
        }
        if (tier >= TIER_MARGINAL) {
            if (bb && pct() < 45) return CHECK;
            if (sb && pct() < 25) { this->decChips(ctc); return CALL; }
        }
        return FOLD;
    }

    // ---- Scenario C: facing ONE raise (raiseCount == 1) ----
    if (info.raiseCount == 1) {
        int potOdds = (info.pot + ctc > 0) ? 100 * ctc / (info.pot + ctc) : 0;

        if (tier >= TIER_PREMIUM) {
            int raiseAmt = std::min(ctc * 3, myChips);
            if (raiseAmt >= myChips) { this->setChips(0); betAmount = myChips; return RAISE; }
            this->decChips(raiseAmt); betAmount = raiseAmt; return RAISE;
        }
        if (tier >= TIER_STRONG && potOdds < 40) {
            if (ctc >= myChips) { this->setChips(0); return CALL; }
            this->decChips(ctc); return CALL;
        }
        if (tier >= TIER_PLAYABLE && potOdds < 30 && (lp || bb)) {
            if (ctc >= myChips) { this->setChips(0); return CALL; }
            this->decChips(ctc); return CALL;
        }
        return FOLD;
    }

    // ---- Scenario D: facing 3-bet or more (raiseCount >= 2) ----
    if (tier >= TIER_PREMIUM && pct() < 50) {
        if (ctc >= myChips) { this->setChips(0); return CALL; }
        this->decChips(ctc); return CALL;
    }
    return FOLD;
}

// ============================================================
//                   Postflop decision tree
// ============================================================
template<typename NumT>
ACTION BotPlayer<NumT>::actPostflop(const gameInfo<NumT>& info, int &betAmount) {
    int ctc = info.chipsToCall;
    int pot = info.pot;
    int myChips = this->chips;
    double wr = info.winRate;

    // ---- Checked to me ----
    if (ctc == 0) {
        if (wr > 80) {
            int betSz = std::max((int)(pot * 0.75), 2);
            if (betSz >= myChips) { this->setChips(0); betAmount = myChips; return RAISE; }
            this->decChips(betSz); betAmount = betSz; return RAISE;
        }
        if (wr > 60) {
            int betSz = std::max((int)(pot * 0.6), 2);
            if (betSz >= myChips) { this->setChips(0); betAmount = myChips; return RAISE; }
            this->decChips(betSz); betAmount = betSz; return RAISE;
        }
        if (wr > 40) {
            if (isLatePosition(info.positionStr) && pct() < 30) {
                int betSz = std::max((int)(pot * 0.4), 2);
                if (betSz >= myChips) { this->setChips(0); betAmount = myChips; return RAISE; }
                this->decChips(betSz); betAmount = betSz; return RAISE;
            }
            return CHECK;
        }
        if (wr > 25) {
            if (pct() < 12) {
                int betSz = std::max((int)(pot * 0.5), 2);
                if (betSz >= myChips) { this->setChips(0); betAmount = myChips; return RAISE; }
                this->decChips(betSz); betAmount = betSz; return RAISE;
            }
            return CHECK;
        }
        if (pct() < 8) {
            int betSz = std::max((int)(pot * 0.6), 2);
            if (betSz >= myChips) { this->setChips(0); betAmount = myChips; return RAISE; }
            this->decChips(betSz); betAmount = betSz; return RAISE;
        }
        return CHECK;
    }

    // ---- Facing a bet ----
    int potOdds = (pot + ctc > 0) ? 100 * ctc / (pot + ctc) : 0;

    if (wr > 60) {
        if (pct() < 70) {
            int raiseAmt = std::min(ctc * 3 + (int)(pot * 0.5), myChips);
            if (raiseAmt >= myChips) { this->setChips(0); betAmount = myChips; return RAISE; }
            this->decChips(raiseAmt); betAmount = raiseAmt; return RAISE;
        }
        if (ctc >= myChips) { this->setChips(0); return CALL; }
        this->decChips(ctc); return CALL;
    }

    if (wr > potOdds + 5) {
        if (ctc >= myChips) { this->setChips(0); return CALL; }
        this->decChips(ctc); return CALL;
    }

    if (wr > potOdds - 8) {
        if (pct() < 10 && isLatePosition(info.positionStr)) {
            int raiseAmt = std::min(ctc * 3, myChips);
            if (raiseAmt >= myChips) { this->setChips(0); betAmount = myChips; return RAISE; }
            this->decChips(raiseAmt); betAmount = raiseAmt; return RAISE;
        }
        if (ctc >= myChips) { this->setChips(0); return CALL; }
        this->decChips(ctc); return CALL;
    }

    if (pct() < 6) {
        int raiseAmt = std::min(ctc * 3, myChips);
        if (raiseAmt >= myChips) { this->setChips(0); betAmount = myChips; return RAISE; }
        this->decChips(raiseAmt); betAmount = raiseAmt; return RAISE;
    }
    return FOLD;
}

template class BotPlayer<CARDNUM>;
template class BotPlayer<SHORT_CARDNUM>;
