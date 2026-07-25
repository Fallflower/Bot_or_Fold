#include "handType.h"
#include "card.h"
#include <iostream>
#include <cassert>
#include <string>
#include <vector>

// ============================================================================
// Convenience helpers to construct cards
// ============================================================================
inline Card<CARDNUM> C(CARDNUM n, SUIT s = SUIT::CLU) { return Card<CARDNUM>(n, s); }
inline Card<SHORT_CARDNUM> CS(SHORT_CARDNUM n, SUIT s = SUIT::CLU) { return Card<SHORT_CARDNUM>(n, s); }

// ============================================================================
// Counters
// ============================================================================
static int passed = 0, failed = 0;

// ============================================================================
// Check helpers
// ============================================================================
static void check(const std::string& name,
                  HANDRANK expectedRank,
                  const std::vector<CARDNUM>& expectedKeys,
                  const HandType<CARDNUM>& result)
{
    if (result.rank == expectedRank && result.keys == expectedKeys) {
        ++passed;
        std::cout << "  [PASS] " << name << "  →  " << result.to_string() << std::endl;
    } else {
        ++failed;
        std::cout << "  [FAIL] " << name << std::endl;
        std::cout << "         expected: rank=" << expectedRank;
        std::cout << "  keys=[";
        for (size_t i = 0; i < expectedKeys.size(); ++i) {
            if (i) std::cout << ",";
            std::cout << static_cast<int>(expectedKeys[i]);
        }
        std::cout << "]" << std::endl;
        std::cout << "         got:      rank=" << result.rank;
        std::cout << "  keys=[";
        for (size_t i = 0; i < result.keys.size(); ++i) {
            if (i) std::cout << ",";
            std::cout << static_cast<int>(result.keys[i]);
        }
        std::cout << "]   to_string=\"" << result.to_string() << "\"" << std::endl;
    }
}

static void checkShort(const std::string& name,
                       HANDRANK expectedRank,
                       const std::vector<SHORT_CARDNUM>& expectedKeys,
                       const HandType<SHORT_CARDNUM>& result)
{
    if (result.rank == expectedRank && result.keys == expectedKeys) {
        ++passed;
        std::cout << "  [PASS] " << name << "  →  " << result.to_string() << std::endl;
    } else {
        ++failed;
        std::cout << "  [FAIL] " << name << std::endl;
        std::cout << "         expected: rank=" << expectedRank;
        std::cout << "  keys=[";
        for (size_t i = 0; i < expectedKeys.size(); ++i) {
            if (i) std::cout << ",";
            std::cout << static_cast<int>(expectedKeys[i]);
        }
        std::cout << "]" << std::endl;
        std::cout << "         got:      rank=" << result.rank;
        std::cout << "  keys=[";
        for (size_t i = 0; i < result.keys.size(); ++i) {
            if (i) std::cout << ",";
            std::cout << static_cast<int>(result.keys[i]);
        }
        std::cout << "]   to_string=\"" << result.to_string() << "\"" << std::endl;
    }
}

// ============================================================================
// Main
// ============================================================================
int main() {
    using N = CARDNUM;
    constexpr N _2 = N::NUM_2,   _3 = N::NUM_3,   _4  = N::NUM_4;
    constexpr N _5 = N::NUM_5,   _6 = N::NUM_6,   _7  = N::NUM_7;
    constexpr N _8 = N::NUM_8,   _9 = N::NUM_9,   _T  = N::NUM_10;
    constexpr N J  = N::JACK,    Q  = N::QUEEN,    K   = N::KING;
    constexpr N A  = N::ACE;

    using SN = SHORT_CARDNUM;
    constexpr SN s6 = SN::NUM_6,  s7 = SN::NUM_7,  s8 = SN::NUM_8;
    constexpr SN s9 = SN::NUM_9,  sT = SN::NUM_10, sJ = SN::JACK;
    constexpr SN sQ = SN::QUEEN,  sK = SN::KING,   sA = SN::ACE;

    // ========================================================================
    //  STANDARD MODE (CARDNUM) — 测试每种牌型
    // ========================================================================

    std::cout << "\n========== 标准模式 (CARDNUM) ==========" << std::endl;

    // ------------------------------------------------------------------
    //  高牌 HIGH_CARD
    // ------------------------------------------------------------------
    std::cout << "\n--- HIGH_CARD ---" << std::endl;
    {
        // 2 cards
        auto r = HandType<N>::evaluate({C(K,SUIT::SPA), C(_2,SUIT::DIA)});
        check("2 cards: K-high", HIGH_CARD, {K, _2}, r);
    }
    {
        // 5 scattered cards
        auto r = HandType<N>::evaluate({
            C(K,SUIT::SPA), C(_3,SUIT::DIA), C(_7,SUIT::CLU),
            C(_T,SUIT::HEA), C(_4,SUIT::SPA)});
        check("5 cards: K-T-7-4-3", HIGH_CARD, {K, _T, _7, _4, _3}, r);
    }
    {
        // 7 scattered cards
        auto r = HandType<N>::evaluate({
            C(A,SUIT::CLU), C(Q,SUIT::DIA), C(_T,SUIT::HEA),
            C(_8,SUIT::SPA), C(_5,SUIT::CLU), C(_3,SUIT::DIA), C(_2,SUIT::HEA)});
        check("7 cards: A-Q-T-8-5-3-2", HIGH_CARD, {A, Q, _T, _8, _5, _3, _2}, r);
    }
    {
        // 7 cards with near-straight but not quite (gap at 6)
        auto r = HandType<N>::evaluate({
            C(_9,SUIT::CLU), C(_8,SUIT::DIA), C(_7,SUIT::HEA),
            C(_5,SUIT::SPA), C(_4,SUIT::CLU), C(_3,SUIT::DIA), C(_2,SUIT::HEA)});
        check("7 cards: near-straight (gap)", HIGH_CARD, {_9, _8, _7, _5, _4, _3, _2}, r);
    }

    // ------------------------------------------------------------------
    //  一对 ONE_PAIR
    // ------------------------------------------------------------------
    std::cout << "\n--- ONE_PAIR ---" << std::endl;
    {
        // bare pair (2 cards)
        auto r = HandType<N>::evaluate({C(K,SUIT::SPA), C(K,SUIT::HEA)});
        check("2 cards: pair K", ONE_PAIR, {K}, r);
    }
    {
        // pair + 2 kickers
        auto r = HandType<N>::evaluate({
            C(_8,SUIT::SPA), C(_8,SUIT::HEA), C(A,SUIT::CLU), C(_3,SUIT::DIA)});
        check("4 cards: pair 8, kicker A", ONE_PAIR, {_8, A}, r);
    }
    {
        // pair + multiple singles (7 cards, best kicker used)
        auto r = HandType<N>::evaluate({
            C(J,SUIT::SPA), C(J,SUIT::CLU),
            C(A,SUIT::DIA), C(K,SUIT::HEA), C(_9,SUIT::SPA),
            C(_5,SUIT::CLU), C(_3,SUIT::DIA)});
        check("7 cards: pair J, kicker A", ONE_PAIR, {J, A}, r);
    }
    {
        // pair in middle, kickers above and below
        auto r = HandType<N>::evaluate({
            C(_7,SUIT::SPA), C(_7,SUIT::DIA),
            C(_2,SUIT::CLU), C(_4,SUIT::HEA), C(_T,SUIT::SPA)});
        check("5 cards: pair 7, kicker T", ONE_PAIR, {_7, _T}, r);
    }

    // ------------------------------------------------------------------
    //  两对 TWO_PAIR  (重点：修复了数组越界 bug)
    // ------------------------------------------------------------------
    std::cout << "\n--- TWO_PAIR ---" << std::endl;
    {
        // exactly 2 pairs, 4 cards, no kicker
        auto r = HandType<N>::evaluate({
            C(_9,SUIT::SPA), C(_9,SUIT::DIA), C(_4,SUIT::CLU), C(_4,SUIT::HEA)});
        check("4 cards: 2pair 9&4, no kicker", TWO_PAIR, {_9, _4}, r);
    }
    {
        // 2 pairs + 1 kicker (5 cards)
        auto r = HandType<N>::evaluate({
            C(Q,SUIT::SPA), C(Q,SUIT::DIA),
            C(_6,SUIT::CLU), C(_6,SUIT::HEA), C(A,SUIT::SPA)});
        check("5 cards: 2pair Q&6, kicker A", TWO_PAIR, {Q, _6, A}, r);
    }
    {
        // 2 pairs + multiple kickers (7 cards) — best kicker picked
        auto r = HandType<N>::evaluate({
            C(_T,SUIT::SPA), C(_T,SUIT::DIA),
            C(_5,SUIT::CLU), C(_5,SUIT::HEA),
            C(A,SUIT::SPA), C(K,SUIT::CLU), C(_3,SUIT::DIA)});
        check("7 cards: 2pair T&5, kicker A", TWO_PAIR, {_T, _5, A}, r);
    }
    {
        // *** 3 pairs, 6 cards — 回归测试：第三对作 kicker ***
        auto r = HandType<N>::evaluate({
            C(_8,SUIT::SPA), C(_8,SUIT::DIA),
            C(_5,SUIT::CLU), C(_5,SUIT::HEA),
            C(_3,SUIT::SPA), C(_3,SUIT::DIA)});
        check("6 cards: 3 pairs 8&5&3 → best two 8&5, kicker 3",
              TWO_PAIR, {_8, _5, _3}, r);
    }
    {
        // *** 3 pairs + kicker, 7 cards — 回归测试 ***
        auto r = HandType<N>::evaluate({
            C(J,SUIT::SPA), C(J,SUIT::DIA),
            C(_9,SUIT::CLU), C(_9,SUIT::HEA),
            C(_4,SUIT::SPA), C(_4,SUIT::DIA),
            C(A,SUIT::CLU)});
        check("7 cards: 3pair J&9&4 + A kicker → 2pair J&9, kicker A",
              TWO_PAIR, {J, _9, A}, r);
    }
    {
        // 3 pairs where kicker is lower than the third pair — kicker wins
        auto r = HandType<N>::evaluate({
            C(K,SUIT::SPA), C(K,SUIT::DIA),
            C(Q,SUIT::CLU), C(Q,SUIT::HEA),
            C(_2,SUIT::SPA), C(_2,SUIT::DIA),
            C(_T,SUIT::CLU)});
        check("7 cards: 3pair K&Q&2 + T kicker → 2pair K&Q, kicker T",
              TWO_PAIR, {K, Q, _T}, r);
    }

    // ------------------------------------------------------------------
    //  三条 THREE_OF_A_KIND  (重点：修复了数组越界 bug)
    // ------------------------------------------------------------------
    std::cout << "\n--- THREE_OF_A_KIND ---" << std::endl;
    {
        // bare trips (3 cards)
        auto r = HandType<N>::evaluate({
            C(_T,SUIT::SPA), C(_T,SUIT::DIA), C(_T,SUIT::HEA)});
        check("3 cards: trips T, no kicker", THREE_OF_A_KIND, {_T}, r);
    }
    {
        // trips + 1 kicker
        auto r = HandType<N>::evaluate({
            C(_5,SUIT::SPA), C(_5,SUIT::DIA), C(_5,SUIT::HEA), C(A,SUIT::CLU)});
        check("4 cards: trips 5, kicker A", THREE_OF_A_KIND, {_5, A}, r);
    }
    {
        // trips + multiple kickers (7 cards) — keys include all high cards
        auto r = HandType<N>::evaluate({
            C(_6,SUIT::SPA), C(_6,SUIT::DIA), C(_6,SUIT::HEA),
            C(A,SUIT::CLU), C(K,SUIT::SPA), C(Q,SUIT::DIA), C(_2,SUIT::HEA)});
        check("7 cards: trips 6, kicker A", THREE_OF_A_KIND, {_6, A, K, Q, _2}, r);
    }
    {
        // trips + multiple singles, no pair → trips, not FH
        auto r = HandType<N>::evaluate({
            C(J,SUIT::SPA), C(J,SUIT::DIA), C(J,SUIT::HEA),
            C(A,SUIT::CLU), C(K,SUIT::SPA), C(_T,SUIT::DIA), C(_2,SUIT::HEA)});
        check("7 cards: trips J + A,K,T,2 → trips J, kicker A",
              THREE_OF_A_KIND, {J, A, K, _T, _2}, r);
    }

    // ------------------------------------------------------------------
    //  顺子 STRAIGHT
    // ------------------------------------------------------------------
    std::cout << "\n--- STRAIGHT ---" << std::endl;
    {
        // normal 5-card straight
        auto r = HandType<N>::evaluate({
            C(_9,SUIT::SPA), C(_8,SUIT::DIA), C(_7,SUIT::CLU),
            C(_6,SUIT::HEA), C(_5,SUIT::SPA)});
        check("5 cards: straight 9-high", STRAIGHT, {_9}, r);
    }
    {
        // Broadway (T-J-Q-K-A)
        auto r = HandType<N>::evaluate({
            C(A,SUIT::SPA), C(K,SUIT::DIA), C(Q,SUIT::CLU),
            C(J,SUIT::HEA), C(_T,SUIT::SPA)});
        check("5 cards: Broadway A-high", STRAIGHT, {A}, r);
    }
    {
        // Ace-low wheel (A-2-3-4-5)
        auto r = HandType<N>::evaluate({
            C(A,SUIT::SPA), C(_2,SUIT::DIA), C(_3,SUIT::CLU),
            C(_4,SUIT::HEA), C(_5,SUIT::SPA)});
        check("5 cards: wheel A-2-3-4-5 (5-high)", STRAIGHT, {_5}, r);
    }
    {
        // 7-card straight (2-3-4-5-6-7-8)
        auto r = HandType<N>::evaluate({
            C(_8,SUIT::SPA), C(_7,SUIT::DIA), C(_6,SUIT::CLU),
            C(_5,SUIT::HEA), C(_4,SUIT::SPA), C(_3,SUIT::DIA), C(_2,SUIT::CLU)});
        check("7 cards: straight 8-high", STRAIGHT, {_8}, r);
    }
    {
        // *** 6 cards with a straight + gap ***
        auto r = HandType<N>::evaluate({
            C(_T,SUIT::SPA), C(_9,SUIT::DIA), C(_8,SUIT::CLU),
            C(_7,SUIT::HEA), C(_6,SUIT::SPA), C(_2,SUIT::DIA)});
        check("6 cards: straight T-high + extra 2", STRAIGHT, {_T}, r);
    }

    // ------------------------------------------------------------------
    //  同花 FLUSH
    // ------------------------------------------------------------------
    std::cout << "\n--- FLUSH ---" << std::endl;
    {
        auto r = HandType<N>::evaluate({
            C(A,SUIT::SPA), C(J,SUIT::SPA), C(_9,SUIT::SPA),
            C(_6,SUIT::SPA), C(_3,SUIT::SPA)});
        check("5 cards: flush A-high", FLUSH, {A, J, _9, _6, _3}, r);
    }
    {
        // 6-card flush
        auto r = HandType<N>::evaluate({
            C(K,SUIT::HEA), C(Q,SUIT::HEA), C(_T,SUIT::HEA),
            C(_8,SUIT::HEA), C(_5,SUIT::HEA), C(_2,SUIT::HEA)});
        check("6 cards: flush K-high", FLUSH, {K, Q, _T, _8, _5, _2}, r);
    }
    {
        // 7-card flush
        auto r = HandType<N>::evaluate({
            C(A,SUIT::DIA), C(Q,SUIT::DIA), C(_T,SUIT::DIA),
            C(_8,SUIT::DIA), C(_6,SUIT::DIA), C(_4,SUIT::DIA), C(_2,SUIT::DIA)});
        check("7 cards: flush A-high", FLUSH, {A, Q, _T, _8, _6, _4, _2}, r);
    }
    {
        // flush cards mixed with non-flush cards (7 cards, only 5 suited)
        auto r = HandType<N>::evaluate({
            C(A,SUIT::SPA), C(K,SUIT::SPA), C(Q,SUIT::SPA),
            C(J,SUIT::SPA), C(_9,SUIT::SPA),
            C(_8,SUIT::DIA), C(_7,SUIT::CLU)});
        check("7 cards: 5-flush A-high + 2 offsuit", FLUSH, {A, K, Q, J, _9}, r);
    }

    // ------------------------------------------------------------------
    //  葫芦 FULL_HOUSE  (重点：修复了双三条的 bug)
    // ------------------------------------------------------------------
    std::cout << "\n--- FULL_HOUSE ---" << std::endl;
    {
        // trips + pair (standard)
        auto r = HandType<N>::evaluate({
            C(K,SUIT::SPA), C(K,SUIT::DIA), C(K,SUIT::HEA),
            C(_T,SUIT::CLU), C(_T,SUIT::SPA)});
        check("5 cards: FH K over T", FULL_HOUSE, {K, _T}, r);
    }
    {
        // trips + pair, with extra singleton
        auto r = HandType<N>::evaluate({
            C(_8,SUIT::SPA), C(_8,SUIT::DIA), C(_8,SUIT::HEA),
            C(_4,SUIT::CLU), C(_4,SUIT::SPA), C(A,SUIT::DIA)});
        check("6 cards: FH 8 over 4 + A kicker(ignored)", FULL_HOUSE, {_8, _4}, r);
    }
    {
        // trips + 2 pairs → picks higher pair for FH
        auto r = HandType<N>::evaluate({
            C(Q,SUIT::SPA), C(Q,SUIT::DIA), C(Q,SUIT::HEA),
            C(_9,SUIT::CLU), C(_9,SUIT::SPA),
            C(_5,SUIT::DIA), C(_5,SUIT::HEA)});
        check("7 cards: trips Q + pairs 9&5 → FH Q over 9",
              FULL_HOUSE, {Q, _9}, r);
    }
    {
        // *** 两个三条 (6 cards) — 回归测试 ***
        auto r = HandType<N>::evaluate({
            C(_7,SUIT::SPA), C(_7,SUIT::DIA), C(_7,SUIT::HEA),
            C(_4,SUIT::CLU), C(_4,SUIT::SPA), C(_4,SUIT::DIA)});
        check("6 cards: two trips 7&4 → FH 7 over 4", FULL_HOUSE, {_7, _4}, r);
    }
    {
        // *** 两个三条 + 一张散牌 (7 cards) ***
        auto r = HandType<N>::evaluate({
            C(J,SUIT::SPA), C(J,SUIT::DIA), C(J,SUIT::HEA),
            C(_8,SUIT::CLU), C(_8,SUIT::SPA), C(_8,SUIT::DIA),
            C(A,SUIT::HEA)});
        check("7 cards: two trips J&8 + A → FH J over 8",
              FULL_HOUSE, {J, _8}, r);
    }

    // ------------------------------------------------------------------
    //  四条 FOUR_OF_A_KIND
    // ------------------------------------------------------------------
    std::cout << "\n--- FOUR_OF_A_KIND ---" << std::endl;
    {
        // bare quads (4 cards)
        auto r = HandType<N>::evaluate({
            C(_6,SUIT::SPA), C(_6,SUIT::DIA), C(_6,SUIT::HEA), C(_6,SUIT::CLU)});
        check("4 cards: quads 6, no kicker", FOUR_OF_A_KIND, {_6, _2}, r);  // quaKicker defaults to kFirstRank
    }
    {
        // quads + 1 kicker
        auto r = HandType<N>::evaluate({
            C(_3,SUIT::SPA), C(_3,SUIT::DIA), C(_3,SUIT::HEA), C(_3,SUIT::CLU),
            C(A,SUIT::SPA)});
        check("5 cards: quads 3, kicker A", FOUR_OF_A_KIND, {_3, A}, r);
    }
    {
        // quads + pair (the pair is better kicker than singles)
        auto r = HandType<N>::evaluate({
            C(_5,SUIT::SPA), C(_5,SUIT::DIA), C(_5,SUIT::HEA), C(_5,SUIT::CLU),
            C(K,SUIT::SPA), C(K,SUIT::DIA), C(_2,SUIT::HEA)});
        check("7 cards: quads 5, kicker K (from pair)", FOUR_OF_A_KIND, {_5, K}, r);
    }
    {
        // quads + trips (trips provides kicker)
        auto r = HandType<N>::evaluate({
            C(_9,SUIT::SPA), C(_9,SUIT::DIA), C(_9,SUIT::HEA), C(_9,SUIT::CLU),
            C(_2,SUIT::SPA), C(_2,SUIT::DIA), C(_2,SUIT::HEA)});
        check("7 cards: quads 9 + trips 2 → kicker 2", FOUR_OF_A_KIND, {_9, _2}, r);
    }

    // ------------------------------------------------------------------
    //  同花顺 STRAIGHT_FLUSH
    // ------------------------------------------------------------------
    std::cout << "\n--- STRAIGHT_FLUSH ---" << std::endl;
    {
        // Royal flush
        auto r = HandType<N>::evaluate({
            C(A,SUIT::SPA), C(K,SUIT::SPA), C(Q,SUIT::SPA),
            C(J,SUIT::SPA), C(_T,SUIT::SPA)});
        check("5 cards: Royal Flush", STRAIGHT_FLUSH, {A}, r);
    }
    {
        // normal straight flush
        auto r = HandType<N>::evaluate({
            C(_9,SUIT::HEA), C(_8,SUIT::HEA), C(_7,SUIT::HEA),
            C(_6,SUIT::HEA), C(_5,SUIT::HEA)});
        check("5 cards: straight flush 9-high", STRAIGHT_FLUSH, {_9}, r);
    }
    {
        // Ace-low straight flush (wheel)
        auto r = HandType<N>::evaluate({
            C(A,SUIT::DIA), C(_2,SUIT::DIA), C(_3,SUIT::DIA),
            C(_4,SUIT::DIA), C(_5,SUIT::DIA)});
        check("5 cards: SF wheel 5-high", STRAIGHT_FLUSH, {_5}, r);
    }
    {
        // straight flush with extra suited cards (7 cards)
        auto r = HandType<N>::evaluate({
            C(_8,SUIT::CLU), C(_7,SUIT::CLU), C(_6,SUIT::CLU),
            C(_5,SUIT::CLU), C(_4,SUIT::CLU),
            C(K,SUIT::CLU), C(_2,SUIT::CLU)});
        check("7 cards: SF 8-high in 7-flush", STRAIGHT_FLUSH, {_8}, r);
    }

    // ========================================================================
    //  SHORT DECK MODE (SHORT_CARDNUM) — 短牌模式
    // ========================================================================

    std::cout << "\n========== 短牌模式 (SHORT_CARDNUM) ==========" << std::endl;

    // ------------------------------------------------------------------
    //  高牌
    // ------------------------------------------------------------------
    std::cout << "\n--- HIGH_CARD (short) ---" << std::endl;
    {
        auto r = HandType<SN>::evaluate({
            CS(sK,SUIT::SPA), CS(s7,SUIT::DIA), CS(s9,SUIT::CLU),
            CS(s6,SUIT::HEA), CS(sT,SUIT::SPA)});
        checkShort("5 cards: K-T-9-7-6", HIGH_CARD, {sK, sT, s9, s7, s6}, r);
    }
    {
        auto r = HandType<SN>::evaluate({
            CS(sA,SUIT::CLU), CS(sQ,SUIT::DIA), CS(sJ,SUIT::HEA),
            CS(s8,SUIT::SPA), CS(s7,SUIT::CLU), CS(s6,SUIT::DIA)});
        checkShort("6 cards: A-Q-J-8-7-6", HIGH_CARD, {sA, sQ, sJ, s8, s7, s6}, r);
    }

    // ------------------------------------------------------------------
    //  一对
    // ------------------------------------------------------------------
    std::cout << "\n--- ONE_PAIR (short) ---" << std::endl;
    {
        auto r = HandType<SN>::evaluate({
            CS(sA,SUIT::SPA), CS(sA,SUIT::HEA), CS(sK,SUIT::CLU)});
        checkShort("3 cards: pair A, kicker K", ONE_PAIR, {sA, sK}, r);
    }
    {
        auto r = HandType<SN>::evaluate({
            CS(s9,SUIT::SPA), CS(s9,SUIT::DIA),
            CS(sA,SUIT::CLU), CS(sK,SUIT::HEA), CS(s7,SUIT::SPA)});
        checkShort("5 cards: pair 9, kicker A", ONE_PAIR, {s9, sA}, r);
    }

    // ------------------------------------------------------------------
    //  两对
    // ------------------------------------------------------------------
    std::cout << "\n--- TWO_PAIR (short) ---" << std::endl;
    {
        auto r = HandType<SN>::evaluate({
            CS(sK,SUIT::SPA), CS(sK,SUIT::DIA),
            CS(s8,SUIT::CLU), CS(s8,SUIT::HEA), CS(sA,SUIT::SPA)});
        checkShort("5 cards: 2pair K&8, kicker A", TWO_PAIR, {sK, s8, sA}, r);
    }
    {
        // *** 3 pairs, 6 cards — regression for short deck ***
        auto r = HandType<SN>::evaluate({
            CS(sJ,SUIT::SPA), CS(sJ,SUIT::DIA),
            CS(s9,SUIT::CLU), CS(s9,SUIT::HEA),
            CS(s7,SUIT::SPA), CS(s7,SUIT::DIA)});
        checkShort("6 cards: 3pairs J&9&7 → 2pair J&9, kicker 7",
                   TWO_PAIR, {sJ, s9, s7}, r);
    }
    {
        // 3 pairs + kicker, 7 cards
        auto r = HandType<SN>::evaluate({
            CS(sQ,SUIT::SPA), CS(sQ,SUIT::DIA),
            CS(sT,SUIT::CLU), CS(sT,SUIT::HEA),
            CS(s6,SUIT::SPA), CS(s6,SUIT::DIA),
            CS(sA,SUIT::CLU)});
        checkShort("7 cards: 3pair Q&T&6 + A → 2pair Q&T, kicker A",
                   TWO_PAIR, {sQ, sT, sA}, r);
    }

    // ------------------------------------------------------------------
    //  三条
    // ------------------------------------------------------------------
    std::cout << "\n--- THREE_OF_A_KIND (short) ---" << std::endl;
    {
        auto r = HandType<SN>::evaluate({
            CS(s8,SUIT::SPA), CS(s8,SUIT::DIA), CS(s8,SUIT::HEA)});
        checkShort("3 cards: trips 8, no kicker", THREE_OF_A_KIND, {s8}, r);
    }
    {
        auto r = HandType<SN>::evaluate({
            CS(sT,SUIT::SPA), CS(sT,SUIT::DIA), CS(sT,SUIT::HEA),
            CS(sA,SUIT::CLU), CS(sK,SUIT::SPA)});
        checkShort("5 cards: trips T, kicker A", THREE_OF_A_KIND, {sT, sA, sK}, r);
    }

    // ------------------------------------------------------------------
    //  顺子
    // ------------------------------------------------------------------
    std::cout << "\n--- STRAIGHT (short) ---" << std::endl;
    {
        // 6-7-8-9-T
        auto r = HandType<SN>::evaluate({
            CS(sT,SUIT::SPA), CS(s9,SUIT::DIA), CS(s8,SUIT::CLU),
            CS(s7,SUIT::HEA), CS(s6,SUIT::SPA)});
        checkShort("5 cards: straight T-high", STRAIGHT, {sT}, r);
    }
    {
        // Broadway short: T-J-Q-K-A
        auto r = HandType<SN>::evaluate({
            CS(sA,SUIT::SPA), CS(sK,SUIT::DIA), CS(sQ,SUIT::CLU),
            CS(sJ,SUIT::HEA), CS(sT,SUIT::SPA)});
        checkShort("5 cards: Broadway A-high", STRAIGHT, {sA}, r);
    }
    {
        // Ace-low wheel: A-6-7-8-9 (short deck wheel)
        auto r = HandType<SN>::evaluate({
            CS(sA,SUIT::SPA), CS(s6,SUIT::DIA), CS(s7,SUIT::CLU),
            CS(s8,SUIT::HEA), CS(s9,SUIT::SPA)});
        checkShort("5 cards: short wheel A-6-7-8-9 (9-high)", STRAIGHT, {s9}, r);
    }
    {
        // 7 cards straight: 6-7-8-9-T-J-Q
        auto r = HandType<SN>::evaluate({
            CS(sQ,SUIT::SPA), CS(sJ,SUIT::DIA), CS(sT,SUIT::CLU),
            CS(s9,SUIT::HEA), CS(s8,SUIT::SPA), CS(s7,SUIT::DIA), CS(s6,SUIT::CLU)});
        checkShort("7 cards: straight Q-high", STRAIGHT, {sQ}, r);
    }

    // ------------------------------------------------------------------
    //  同花 (短牌中同花 > 葫芦)
    // ------------------------------------------------------------------
    std::cout << "\n--- FLUSH (short) ---" << std::endl;
    {
        auto r = HandType<SN>::evaluate({
            CS(sA,SUIT::HEA), CS(sK,SUIT::HEA), CS(sT,SUIT::HEA),
            CS(s8,SUIT::HEA), CS(s7,SUIT::HEA)});
        checkShort("5 cards: flush A-high", FLUSH, {sA, sK, sT, s8, s7}, r);
    }
    {
        // 7 cards flush
        auto r = HandType<SN>::evaluate({
            CS(sQ,SUIT::DIA), CS(sJ,SUIT::DIA), CS(sT,SUIT::DIA),
            CS(s9,SUIT::DIA), CS(s7,SUIT::DIA), CS(s6,SUIT::DIA)});
        checkShort("6 cards: flush Q-high", FLUSH, {sQ, sJ, sT, s9, s7, s6}, r);
    }

    // ------------------------------------------------------------------
    //  葫芦
    // ------------------------------------------------------------------
    std::cout << "\n--- FULL_HOUSE (short) ---" << std::endl;
    {
        auto r = HandType<SN>::evaluate({
            CS(sA,SUIT::SPA), CS(sA,SUIT::DIA), CS(sA,SUIT::HEA),
            CS(sK,SUIT::CLU), CS(sK,SUIT::SPA)});
        checkShort("5 cards: FH A over K", FULL_HOUSE, {sA, sK}, r);
    }
    {
        // *** 两个三条 — regression ***
        auto r = HandType<SN>::evaluate({
            CS(sT,SUIT::SPA), CS(sT,SUIT::DIA), CS(sT,SUIT::HEA),
            CS(s8,SUIT::CLU), CS(s8,SUIT::SPA), CS(s8,SUIT::DIA)});
        checkShort("6 cards: two trips T&8 → FH T over 8", FULL_HOUSE, {sT, s8}, r);
    }
    {
        // trips + 2 pairs → picks higher pair
        auto r = HandType<SN>::evaluate({
            CS(sQ,SUIT::SPA), CS(sQ,SUIT::DIA), CS(sQ,SUIT::HEA),
            CS(sJ,SUIT::CLU), CS(sJ,SUIT::SPA),
            CS(s7,SUIT::DIA), CS(s7,SUIT::HEA)});
        checkShort("7 cards: trips Q + pairs J&7 → FH Q over J",
                   FULL_HOUSE, {sQ, sJ}, r);
    }

    // ------------------------------------------------------------------
    //  四条
    // ------------------------------------------------------------------
    std::cout << "\n--- FOUR_OF_A_KIND (short) ---" << std::endl;
    {
        auto r = HandType<SN>::evaluate({
            CS(s9,SUIT::SPA), CS(s9,SUIT::DIA), CS(s9,SUIT::HEA), CS(s9,SUIT::CLU),
            CS(sA,SUIT::SPA)});
        checkShort("5 cards: quads 9, kicker A", FOUR_OF_A_KIND, {s9, sA}, r);
    }

    // ------------------------------------------------------------------
    //  同花顺
    // ------------------------------------------------------------------
    std::cout << "\n--- STRAIGHT_FLUSH (short) ---" << std::endl;
    {
        // Royal
        auto r = HandType<SN>::evaluate({
            CS(sA,SUIT::SPA), CS(sK,SUIT::SPA), CS(sQ,SUIT::SPA),
            CS(sJ,SUIT::SPA), CS(sT,SUIT::SPA)});
        checkShort("5 cards: Royal Flush", STRAIGHT_FLUSH, {sA}, r);
    }
    {
        // Normal SF: 7-8-9-T-J
        auto r = HandType<SN>::evaluate({
            CS(sJ,SUIT::HEA), CS(sT,SUIT::HEA), CS(s9,SUIT::HEA),
            CS(s8,SUIT::HEA), CS(s7,SUIT::HEA)});
        checkShort("5 cards: SF J-high", STRAIGHT_FLUSH, {sJ}, r);
    }
    {
        // Ace-low SF: A-6-7-8-9
        auto r = HandType<SN>::evaluate({
            CS(sA,SUIT::DIA), CS(s6,SUIT::DIA), CS(s7,SUIT::DIA),
            CS(s8,SUIT::DIA), CS(s9,SUIT::DIA)});
        checkShort("5 cards: SF short wheel 9-high", STRAIGHT_FLUSH, {s9}, r);
    }

    // ========================================================================
    //  边界测试 — 2~7 张牌的所有关键组合
    // ========================================================================

    std::cout << "\n========== 边界测试 (Edge Cases) ==========" << std::endl;

    // --- 2 cards ---
    std::cout << "\n--- 2 cards ---" << std::endl;
    {
        auto r = HandType<N>::evaluate({C(_2,SUIT::SPA), C(_3,SUIT::DIA)});
        check("2 cards: 3-2 high", HIGH_CARD, {_3, _2}, r);
    }

    // --- 3 cards ---
    std::cout << "\n--- 3 cards ---" << std::endl;
    {
        auto r = HandType<N>::evaluate({
            C(_2,SUIT::SPA), C(_2,SUIT::DIA), C(_2,SUIT::HEA)});
        check("3 cards: trips 2", THREE_OF_A_KIND, {_2}, r);
    }
    {
        auto r = HandType<N>::evaluate({
            C(A,SUIT::SPA), C(_2,SUIT::DIA), C(_3,SUIT::CLU)});
        check("3 cards: A-3-2 high", HIGH_CARD, {A, _3, _2}, r);
    }

    // --- 6 cards special ---
    std::cout << "\n--- 6 cards special ---" << std::endl;
    {
        // 6 cards: 3 pairs exactly (no kicker)
        auto r = HandType<N>::evaluate({
            C(A,SUIT::SPA), C(A,SUIT::DIA),
            C(K,SUIT::CLU), C(K,SUIT::HEA),
            C(_2,SUIT::SPA), C(_2,SUIT::DIA)});
        check("6 cards: 3 pairs A&K&2 → 2pair A&K, kicker 2",
              TWO_PAIR, {A, K, _2}, r);
    }
    {
        // 6 cards: 3 pairs with two trips-esque... no, just 3 distinct pairs
        auto r = HandType<N>::evaluate({
            C(Q,SUIT::SPA), C(Q,SUIT::DIA),
            C(J,SUIT::CLU), C(J,SUIT::HEA),
            C(_T,SUIT::SPA), C(_T,SUIT::DIA)});
        check("6 cards: 3 pairs Q&J&T → 2pair Q&J, kicker T",
              TWO_PAIR, {Q, J, _T}, r);
    }
    {
        // 6 cards: two trips
        auto r = HandType<N>::evaluate({
            C(_8,SUIT::SPA), C(_8,SUIT::DIA), C(_8,SUIT::HEA),
            C(_5,SUIT::CLU), C(_5,SUIT::SPA), C(_5,SUIT::DIA)});
        check("6 cards: two trips 8&5 → FH 8 over 5", FULL_HOUSE, {_8, _5}, r);
    }
    {
        // 6 cards: quads + pair
        auto r = HandType<N>::evaluate({
            C(_7,SUIT::SPA), C(_7,SUIT::DIA), C(_7,SUIT::HEA), C(_7,SUIT::CLU),
            C(A,SUIT::SPA), C(A,SUIT::DIA)});
        check("6 cards: quads 7 + pair A → kicker A", FOUR_OF_A_KIND, {_7, A}, r);
    }

    // --- 7 cards special ---
    std::cout << "\n--- 7 cards special ---" << std::endl;
    {
        // 7 cards: flush + straight (straight flush should win)
        auto r = HandType<N>::evaluate({
            C(_9,SUIT::HEA), C(_8,SUIT::HEA), C(_7,SUIT::HEA),
            C(_6,SUIT::HEA), C(_5,SUIT::HEA),
            C(A,SUIT::SPA), C(K,SUIT::DIA)});
        check("7 cards: SF 9-high beating A-high flush", STRAIGHT_FLUSH, {_9}, r);
    }
    {
        // 7 cards: full house vs flush (FH wins in standard)
        auto r = HandType<N>::evaluate({
            C(K,SUIT::SPA), C(K,SUIT::DIA), C(K,SUIT::HEA),
            C(Q,SUIT::CLU), C(Q,SUIT::SPA),
            C(_T,SUIT::HEA), C(_7,SUIT::HEA)});
        check("7 cards: FH K over Q (flush not possible, 3h)", FULL_HOUSE, {K, Q}, r);
    }
    {
        // 7 cards: two trips + kicker → FH
        auto r = HandType<N>::evaluate({
            C(_T,SUIT::SPA), C(_T,SUIT::DIA), C(_T,SUIT::HEA),
            C(_9,SUIT::CLU), C(_9,SUIT::SPA), C(_9,SUIT::DIA),
            C(A,SUIT::HEA)});
        check("7 cards: two trips T&9 + A → FH T over 9", FULL_HOUSE, {_T, _9}, r);
    }
    {
        // 7 cards: straight + pairs → best 5 is straight
        auto r = HandType<N>::evaluate({
            C(J,SUIT::SPA), C(_T,SUIT::DIA), C(_9,SUIT::CLU),
            C(_8,SUIT::HEA), C(_7,SUIT::SPA),
            C(_7,SUIT::DIA), C(_T,SUIT::HEA)});
        check("7 cards: straight J-high with pair 7&T", STRAIGHT, {J}, r);
    }
    {
        // 7 cards: quads + trips → quads with trips kicker
        auto r = HandType<N>::evaluate({
            C(_6,SUIT::SPA), C(_6,SUIT::DIA), C(_6,SUIT::HEA), C(_6,SUIT::CLU),
            C(_4,SUIT::SPA), C(_4,SUIT::DIA), C(_4,SUIT::HEA)});
        check("7 cards: quads 6 + trips 4 → kicker 4", FOUR_OF_A_KIND, {_6, _4}, r);
    }

    // --- 7 cards: sf over flush ---
    {
        // Full 7-card flush containing a straight flush
        auto r = HandType<N>::evaluate({
            C(J,SUIT::CLU), C(_T,SUIT::CLU), C(_9,SUIT::CLU),
            C(_8,SUIT::CLU), C(_7,SUIT::CLU),
            C(_4,SUIT::CLU), C(_3,SUIT::CLU)});
        check("7 cards: 7-flush with SF J-high", STRAIGHT_FLUSH, {J}, r);
    }

    // ========================================================================
    //  排名比较测试 (compareHandType)
    // ========================================================================

    std::cout << "\n========== 排名比较 (compareHandType) ==========" << std::endl;

    // 验证标准模式：同花顺 > 四条 > 葫芦 > 同花 > 顺子 > 三条 > 两对 > 一对 > 高牌
    {
        auto sf = HandType<N>::evaluate({C(_9,SUIT::SPA),C(_8,SUIT::SPA),C(_7,SUIT::SPA),C(_6,SUIT::SPA),C(_5,SUIT::SPA)});
        auto qd = HandType<N>::evaluate({C(A,SUIT::SPA),C(A,SUIT::DIA),C(A,SUIT::HEA),C(A,SUIT::CLU),C(K,SUIT::SPA)});
        auto fh = HandType<N>::evaluate({C(K,SUIT::SPA),C(K,SUIT::DIA),C(K,SUIT::HEA),C(Q,SUIT::CLU),C(Q,SUIT::SPA)});
        auto fl = HandType<N>::evaluate({C(A,SUIT::HEA),C(J,SUIT::HEA),C(_9,SUIT::HEA),C(_6,SUIT::HEA),C(_3,SUIT::HEA)});
        auto st = HandType<N>::evaluate({C(_9,SUIT::SPA),C(_8,SUIT::DIA),C(_7,SUIT::CLU),C(_6,SUIT::HEA),C(_5,SUIT::SPA)});
        auto tk = HandType<N>::evaluate({C(_T,SUIT::SPA),C(_T,SUIT::DIA),C(_T,SUIT::HEA),C(A,SUIT::CLU),C(K,SUIT::SPA)});
        auto tp = HandType<N>::evaluate({C(Q,SUIT::SPA),C(Q,SUIT::DIA),C(_6,SUIT::CLU),C(_6,SUIT::HEA),C(A,SUIT::SPA)});
        auto op = HandType<N>::evaluate({C(K,SUIT::SPA),C(K,SUIT::HEA),C(A,SUIT::CLU),C(_3,SUIT::DIA)});
        auto hc = HandType<N>::evaluate({C(A,SUIT::SPA),C(K,SUIT::DIA),C(Q,SUIT::CLU),C(J,SUIT::HEA),C(_9,SUIT::SPA)});

        auto cmpAll = [&](const auto& a, const auto& b, const char* aname, const char* bname) {
            int r = HandType<N>::compareHandType(a, b);
            if (r == 1) {
                ++passed;
                std::cout << "  [PASS] " << aname << " > " << bname << std::endl;
            } else {
                ++failed;
                std::cout << "  [FAIL] " << aname << " vs " << bname
                          << ": expected 1, got " << r << std::endl;
            }
        };

        cmpAll(sf, qd, "StraightFlush", "Quads");
        cmpAll(qd, fh, "Quads", "FullHouse");
        cmpAll(fh, fl, "FullHouse", "Flush");
        cmpAll(fl, st, "Flush", "Straight");
        cmpAll(st, tk, "Straight", "Trips");
        cmpAll(tk, tp, "Trips", "TwoPair");
        cmpAll(tp, op, "TwoPair", "OnePair");
        cmpAll(op, hc, "OnePair", "HighCard");
    }

    // 验证短牌模式：同花 > 葫芦 (key difference)
    {
        auto sf_short = HandType<SN>::evaluate({
            CS(sJ,SUIT::SPA),CS(sT,SUIT::SPA),CS(s9,SUIT::SPA),CS(s8,SUIT::SPA),CS(s7,SUIT::SPA)});
        auto qd_short = HandType<SN>::evaluate({
            CS(sA,SUIT::SPA),CS(sA,SUIT::DIA),CS(sA,SUIT::HEA),CS(sA,SUIT::CLU),CS(sK,SUIT::SPA)});
        auto fl_short = HandType<SN>::evaluate({
            CS(sA,SUIT::HEA),CS(sK,SUIT::HEA),CS(sT,SUIT::HEA),CS(s8,SUIT::HEA),CS(s7,SUIT::HEA)});
        auto fh_short = HandType<SN>::evaluate({
            CS(sK,SUIT::SPA),CS(sK,SUIT::DIA),CS(sK,SUIT::HEA),CS(sQ,SUIT::CLU),CS(sQ,SUIT::SPA)});

        // Standard mode: FH > Flush
        auto fh_std = HandType<N>::evaluate({
            C(K,SUIT::SPA),C(K,SUIT::DIA),C(K,SUIT::HEA),C(Q,SUIT::CLU),C(Q,SUIT::SPA)});
        auto fl_std = HandType<N>::evaluate({
            C(A,SUIT::HEA),C(K,SUIT::HEA),C(_T,SUIT::HEA),C(_8,SUIT::HEA),C(_7,SUIT::HEA)});

        int rStandard = HandType<N>::compareHandType(fh_std, fl_std);
        if (rStandard == 1) {
            ++passed;
            std::cout << "  [PASS] Standard: FullHouse > Flush" << std::endl;
        } else {
            ++failed;
            std::cout << "  [FAIL] Standard: FullHouse vs Flush, expected 1 got "
                      << rStandard << std::endl;
        }

        // Short deck mode: Flush > FH
        int rShort = HandType<SN>::compareHandType(fl_short, fh_short);
        if (rShort == 1) {
            ++passed;
            std::cout << "  [PASS] Short: Flush > FullHouse" << std::endl;
        } else {
            ++failed;
            std::cout << "  [FAIL] Short: Flush vs FullHouse, expected 1 got "
                      << rShort << std::endl;
        }

        // Cross-check: SF still > quads in short
        int rSF = HandType<SN>::compareHandType(sf_short, qd_short);
        if (rSF == 1) {
            ++passed;
            std::cout << "  [PASS] Short: StraightFlush > Quads" << std::endl;
        } else {
            ++failed;
            std::cout << "  [FAIL] Short: StraightFlush vs Quads, expected 1 got "
                      << rSF << std::endl;
        }
    }

    // same-rank comparison: kicker decides
    {
        auto tp1 = HandType<N>::evaluate({
            C(Q,SUIT::SPA),C(Q,SUIT::DIA),C(J,SUIT::CLU),C(J,SUIT::HEA),C(A,SUIT::SPA)});
        auto tp2 = HandType<N>::evaluate({
            C(Q,SUIT::SPA),C(Q,SUIT::HEA),C(J,SUIT::DIA),C(J,SUIT::CLU),C(K,SUIT::SPA)});
        int r = HandType<N>::compareHandType(tp1, tp2);
        if (r == 1) {
            ++passed;
            std::cout << "  [PASS] 2pair Q&J A-kicker > 2pair Q&J K-kicker" << std::endl;
        } else {
            ++failed;
            std::cout << "  [FAIL] 2pair kicker comparison, expected 1 got " << r << std::endl;
        }

        // equal hands
        int rEq = HandType<N>::compareHandType(tp1, tp1);
        if (rEq == 0) {
            ++passed;
            std::cout << "  [PASS] equal hands → compareHandType returns 0" << std::endl;
        } else {
            ++failed;
            std::cout << "  [FAIL] equal hands, expected 0 got " << rEq << std::endl;
        }
    }

    // ========================================================================
    //  RESULT
    // ========================================================================
    std::cout << "\n==============================================" << std::endl;
    std::cout << "  TOTAL: " << (passed + failed)
              << "  |  PASSED: " << passed
              << "  |  FAILED: " << failed << std::endl;
    std::cout << "==============================================" << std::endl;

    return failed == 0 ? 0 : 1;
}
