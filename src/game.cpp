#include "game.h"
#include "assistant.h"
#include <algorithm>
#include <thread>
#include <random>
#include <type_traits>
#include "ranker_data.h"
#include "ranker_data_short.h"

const std::string stateStr[] = {"preflop", "flop", "turn", "river", "end"};


template <class ElemType >
inline void Swap(ElemType &e1, ElemType &e2)
// 操作结果: 交换e1, e2之值
{
	ElemType temp;		// 临时变量
	// 循环赋值实现交换e1, e2
	temp = e1;	e1 = e2;  e2 = temp;
}

template<typename NumT>
void Game<NumT>::init_game() {
    chips = new int*[playerNum];
    for (int i = 0; i < playerNum; i++)
        chips[i] = new int[4]{0};

    ftag = new bool[playerNum];
    for (int i = 0; i < playerNum; i++)
        ftag[i] = false;

    ctag = new bool[playerNum];
    for (int i = 0; i < playerNum; i++)
        ctag[i] = false;

    atag = new bool[playerNum];
    for (int i = 0; i < playerNum; i++)
        atag[i] = false;

    active = (dealer + 3) % playerNum;
}

template<typename NumT>
void Game<NumT>::reset_tags() {
    for (int i = 0; i < playerNum; i++) {
        for (int j = 0; j < 4; j++)
            chips[i][j] = 0;
        ftag[i] = false;
        ctag[i] = false;
        atag[i] = false;
    }
}

template<typename NumT>
void Game<NumT>::init_players(const HumanPlayer& p, const int& c) {
    for (int i = 1; i < playerNum; i++)
        players.push_back(std::make_unique<BotPlayer>("BotPlayer"+std::to_string(i), c));
    players.insert(players.begin() + hpi, std::make_unique<HumanPlayer>(p));
}

template<typename NumT>
void Game<NumT>::init_blinds() {
    int sb = pos.find(" S B ");
    int bb = pos.find(" B B ");
    chips[sb][0] = 1;
    chips[bb][0] = 2;
    players[sb]->decChips(1);
    players[bb]->decChips(2);
    lastBet = bb;
    commit[0]=2;commit[1]=0;commit[2]=0;commit[3]=0;
}

template<typename NumT>
void Game<NumT>::checkState() {
    int i, n1 = 0, n2 = 0;
    for (i = 0; i < playerNum; i++)
    {
        if (!ftag[i] && !atag[i]) n1++;    // 还在牌局中的玩家数
        if (!ftag[i] && !atag[i] && !ctag[i]) n2++;  // 还在牌局中且没有check的玩家数
    }
    if (n1 < 1) { stateCode = 4; deck_.setShow(stateCode); return; }   // 当没有玩家时，游戏结束
    if (n1 == 1 && n2 == 0) { stateCode = 4; deck_.setShow(stateCode); return; }   // 当只剩下一个非Allin玩家且非fold玩家，且check状态时，游戏结束
    if (n1 > 1 && n2 == 0) {   // 当只剩多个非Allin玩家且非fold玩家，且全部check状态时，进入下一阶段
        stateCode++;
        deck_.setShow(stateCode);
        lastBet = -1;   // 清空lasetBet指针
        for (int j = 0; j < playerNum; j++) // 清空check tags
            if (!ftag[j])
                ctag[j] = false;
        // 移动active指针到庄位后一位
        active = dealer;
    }
    step();
}


template<typename NumT>
const std::vector<Card<NumT>> Game<NumT>::getHands(const int& k) const {
    std::vector<Card<NumT>> temp;
    int sc = stateCode > 3 ? 3 : stateCode;
    if (sc) {
        const auto& pub = deck_.getPubCards();
        temp.assign(pub.begin(), pub.end() - 3 + sc);
    }
    temp.insert(temp.end(), hands[k].begin(), hands[k].end());
    return temp;
}

template<typename NumT>
const std::vector<Card<NumT>> Game<NumT>::getFinalHands(const int& k) const {
    std::vector<Card<NumT>> temp;
    const auto& pub = deck_.getPubCards();
    temp.assign(pub.begin(), pub.end());
    temp.insert(temp.end(), hands[k].begin(), hands[k].end());
    return temp;
}

template<typename NumT>
double Game<NumT>::calcEquity(const int& pi, const int& simulations) const {
    std::vector<double> win(playerNum, 0.0);

    std::vector<Card<NumT>> knownPubCards = getKnownPubCards();
    int left_n = 5 - knownPubCards.size();
    Deck<NumT> simDeck(getHands(pi));   // 构造一个牌堆，不含玩家pi的手牌和已知的公共牌
    for (int i = 0; i < simulations; i++) {
        Deck<NumT> tempDeck = simDeck;
        tempDeck.shuffle();
        std::vector<std::vector<Card<NumT>>> simHands;
        tempDeck.deal(playerNum, simHands);
        std::vector<Card<NumT>> pub_cards(knownPubCards.begin(), knownPubCards.end());
        if (left_n > 0) {   // 拼接已知的公共牌和随机发的公共牌
            const auto remain_pub = tempDeck.getFrontN(left_n);
            pub_cards.insert(pub_cards.end(), remain_pub.begin(), remain_pub.end());
        }
        simHands.insert(simHands.begin() + pi, hands[pi]); // 将玩家pi的手牌插入模拟手牌正确的位置
        auto winners = checkWinner(simHands, pub_cards);
        double share = 1.0 / winners.size();
        for (auto j : winners)
            win[j] += share;
    }
    for (int i = 0; i < playerNum; i++)
        win[i] = 100.0 * win[i] / simulations;
    return win[pi];
}

template<typename NumT>
std::vector<double> Game<NumT>::calcWinRate(const int& simulations) const {
    std::vector<double> win(playerNum, 0.0);
    if (stateCode < 3) {
        int known_pub_cards_num = (stateCode) ? stateCode + 2 : 0;
        std::vector<Card<NumT>> deck_remain = deck_.remainingDeck(playerNum, known_pub_cards_num);

        int num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) num_threads = 4;
        int per_thread = simulations / num_threads;

        std::vector<std::vector<double>> local_win(num_threads, std::vector<double>(playerNum, 0.0));
        std::vector<std::thread> threads;

        for (int t = 0; t < num_threads; t++) {
            threads.emplace_back([&, t]() {
                std::mt19937 engin(std::random_device{}());
                int start = t * per_thread;
                int end = (t == num_threads - 1) ? simulations : start + per_thread;

                for (int i = start; i < end; i++) {
                    auto tmp = deck_remain;
                    std::shuffle(tmp.begin(), tmp.end(), engin);
                    std::vector<Card<NumT>> board(tmp.begin(), tmp.begin() + 5 - known_pub_cards_num);
                    const auto& pub = deck_.getPubCards();
                    board.insert(board.end(), pub.begin(), pub.begin() + known_pub_cards_num);
                    auto winners = checkWinner(hands, board);
                    double share = 1.0 / winners.size();
                    for (auto j : winners)
                        local_win[t][j] += share;
                }
            });
        }

        for (auto& t : threads) t.join();

        for (int t = 0; t < num_threads; t++)
            for (int i = 0; i < playerNum; i++)
                win[i] += local_win[t][i];
    }

    if (stateCode >= 3) {
        auto winners = checkWinner(hands, deck_.getPubCards());
        int n = winners.size();
        if (n == 1) win[winners[0]] = 1.0 * simulations;
    }

    for (int i = 0; i < playerNum; i++)
        win[i] = 100.0 * win[i] / simulations;
    return win;
}

template<typename NumT>
std::vector<int> Game<NumT>::checkWinner(const std::vector<std::vector<Card<NumT>>>& simHands, const std::vector<Card<NumT>>& publicCards) const {
    std::vector<int> res;
    int bestRank = INT_MAX;
    for (int i = 0; i < playerNum; i++) {
        if (!ftag[i]) {
            std::vector<Card<NumT>> handCards = simHands[i];
            handCards.insert(handCards.end(), publicCards.begin(), publicCards.end());
            int rank = advancedEvaluate(handCards);
            if (rank == INT_MAX) throw Error(4, "System Error: look up table failed");
            if (rank < bestRank) {
                res.clear();
                bestRank = rank;
                res.push_back(i);
            } else if (rank == bestRank) {
                res.push_back(i);
            }
        }
    }
    return res;
}

template<typename NumT>
Game<NumT>::Game(int pn, int d): playerNum(pn), dealer(d), stateCode(0) {
    init_game();
    deck_.shuffle();
    deck_.deal(playerNum, hands);
}

template<typename NumT>
Game<NumT>::Game(const Position& p,const int& c, const HumanPlayer& hp, const int& hppi)
: playerNum(p.getPlayerNum()), inic(c), hpi(hppi), dealer(p.getDealer()), stateCode(0), pos(p) {
    init_game();
    init_players(hp, c);
    init_blinds();
    deck_.shuffle();
    deck_.deal(playerNum, hands);
    hands[hpi][0].show = 1;
    hands[hpi][1].show = 1;
    if constexpr (std::is_same_v<NumT, CARDNUM>) {
        if (!initAdvancedRanker(ranker_bin, ranker_bin_len))
            throw Error(101, "System Error: failed to load embedded ranker data");
    } else if constexpr (std::is_same_v<NumT, SHORT_CARDNUM>) {
        if (!initAdvancedRanker(ranker_short_bin, ranker_short_bin_len))
            throw Error(101, "System Error: failed to load embedded ranker data");
    }
}

template<typename NumT>
Game<NumT>::~Game() {
    for (int i = 0; i < playerNum; i++)
        delete[] chips[i];
    delete[] chips;
    delete[] ftag;
    delete[] ctag;
    delete[] atag;
}


template<typename NumT>
void Game<NumT>::show(std::ostream& out) const {
    out << "================================================================" << std::endl;
    out << "  Public: " << std::endl;
    out << "\t\t\t";
    for (auto c : deck_.getPubCards()) out << c.toString() << " ";
    out << std::endl;
    out << "   State:  " << stateStr[stateCode] << std::endl;
    out << "     Pot:  " << getPot() << std::endl;
    out << "----------------------------------------------------------------" << std::endl;
    auto win_rate = calcWinRate(20000);
    for (int i = 0; i < playerNum; i++) {
        if (i == active) out << " *";
        else out << "  ";
        // 玩家名
        out << (i==hpi ? "HP"+std::to_string(i) : "BP"+std::to_string(i));
        out << " (" << pos[i] << ")";
        // 后手筹码
        out << std::right << std::setw(5) << players[i]->getChips() << " BB:\t";
        for (int j = 0; j < 2; j++)
            out << hands[i][j].toString() << ' ';
        // 总投入筹码
        out << "\t" << getPlayerCommited(i);
        if (ftag[i])
            out << "\t(fold)\t" << HandType<NumT>::evaluate(getHands(i));
        else
            out << "\t" << std::fixed << std::setprecision(2) << win_rate[i] << "%\t" << HandType<NumT>::evaluate(getHands(i));
        out << std::endl;
    }
    out << "================================================================" << std::endl;
}

template<typename NumT>
void Game<NumT>::showPlayerView(std::ostream& out) const {
    out << "================================================================\n";
    out << "  Public: \n";
    printCards(out, deck_.getPubCards(), "\t    ");
    out << "   State:  " << stateStr[stateCode] << "\n";
    out << "     Pot:  " << getPot() << "\n";
    out << "----------------------------------------------------------------\n";

    for (int i = 0; i < playerNum; i++) {
        // active标记
        out << (i == active ? " *" : "  ");
        //玩家名：固定宽度
        out << std::left << std::setw(12) << players[i]->getName();
        out << " (" << pos[i] << ")";
        // 后手筹码
        out << std::right << std::setw(5) << players[i]->getChips() << " :\t";
        // 手牌
        for (int j = 0; j < 2; j++)
            out << hands[i][j] << ' ';
        // 筹码
        out << std::right << std::setw(5) << getPlayerCommited(i) << "\t";

        // 动作
        const actInfo aif = players[i]->getLastAction();
        if (aif.id > -1 && aif.stateCode == stateCode)
            out << aif;
        else if (ftag[i])
            out << std::left << std::setw(14) << "(fold)  .";
        else
            out << std::left << std::setw(14) << ".";
        out << "\n";
    }

    out << "================================================================" << std::endl;
    if (active != hpi)
        out << players[active]->getName() << " is thinking..." << std::endl;
}

template<typename NumT>
int Game<NumT>::getPot() const {
    int temp = 0;
    for (int i = 0; i < playerNum; i++)
        for (int j = 0; j < 4; j++)
            temp += chips[i][j];
    return temp;
}

template<typename NumT>
void Game<NumT>::fold() {
    if (hpi == active) {    // 唯一的人类玩家选择弃牌，游戏结束
        std::cout << "You folded. Better luck next time!" << std::endl;
        stateCode = 4;
        deck_.setShow(stateCode);
        ftag[hpi] = 1;
        return;
    }
    ftag[active] = 1;
    checkState();
}

template<typename NumT>
void Game<NumT>::call(const int& amount) {
    chips[active][stateCode] += amount;
    ctag[active] = true;
    if (players[active]->getChips() == 0)
        atag[active] = true;
    checkState();
}

template<typename NumT>
void Game<NumT>::bet(const int& chip) {
    if (chips[active][stateCode] + chip <= commit[stateCode])
        throw Error(1, "System Error: Invalid betting scale.");
    for (int i = 0; i < playerNum; i++)
        if (!atag[i])       // 只有非all-in玩家才会被清空check tag
            ctag[i] = false;// 加注将清空其他人的check tag
    ctag[active] = true;    // 加注将自己标记成为check tag
    chips[active][stateCode] += chip;
    commit[stateCode] = chips[active][stateCode];
    lastBet = active;
    if (players[active]->getChips() == 0)
        atag[active] = true;
    step();
}

template<typename NumT>
void Game<NumT>::toAct() { // 玩家筹码修改在Player的makeAction中处理

    int chipsToCall = getChipsToCall();
    int playerChips = players[active]->getChips();

    // 计算还有几个非fold非allin的活跃玩家
    int activePlayers = 0;
    for (int i = 0; i < playerNum; i++)
        if (!ftag[i] && !atag[i]) activePlayers++;
    bool canRaise = activePlayers >= 2;  // 有对手才能下注/加注

    // 由Game判断合法操作列表
    std::vector<ACTION> legalActions;
    if (chipsToCall == 0) {
        legalActions.push_back(CHECK);
        if (canRaise) legalActions.push_back(RAISE);
    } else if (chipsToCall < playerChips) {
        legalActions = {FOLD, CALL};
        if (canRaise) legalActions.push_back(RAISE);
    } else {
        legalActions = {FOLD, CALL};  // CALL = 全下跟注
    }

    std::string pubCardsStr = "";
    for (auto c : getKnownPubCards())
        pubCardsStr += c.toString() + " ";

    gameInfo info {
        .playerNum = playerNum,
        .remainPlayerNum = activePlayers,
        .stateCode = stateCode,
        .pot = getPot(),
        .chipsToCall = chipsToCall,
        .playerCommited = getPlayerCommited(active),
        .winRate = calcEquity(active, 12288),
        .positionStr = pos[active],
        .handCardsStr = hands[active][0].toString() + " " + hands[active][1].toString(),
        .publicCardsStr = pubCardsStr,
        .handTypeStr = HandType<NumT>::evaluate(getHands(active)).to_string(),
        .legalActions = legalActions
    };
    int betAmount = 0;
    ACTION action = players[active]->makeAction(info, betAmount);
    players[active]->addActionHistory(actInfo{0, stateCode, action, betAmount});
    if (g_log) {
        std::string actStr = action2str(action);
        if (action == RAISE)
            actStr += " " + std::to_string(betAmount);
        g_log->writeLine("  [" + std::string(stateStr[stateCode]) + "] " + players[active]->getName() + ": " + actStr);
    }
    switch (action) {
    case FOLD:
        fold(); break;
    case CHECK:
        call(0); break;
    case CALL:
        call(std::min(chipsToCall, playerChips)); break;
    case RAISE:
        bet(betAmount); break;
    }
}

template<typename NumT>
void Game<NumT>::afterEnd() {
    if (!isEnd()) return;
    auto winners = checkWinner(hands, deck_.getPubCards());
    int pot = getPot();
    int share = pot / winners.size();
    for (size_t i = 0; i < winners.size(); i++)
        players[winners[i]]->addChips(share);

    // 终端输出：玩家关心的结果
    std::cout << "\nGame Over! Final Results:" << std::endl;
    show();
    for (size_t i = 0; i < winners.size(); i++)
        std::cout << players[winners[i]]->getName() << " won " << share << " chips" << std::endl;
    if (players[hpi]->getChips() == 0) {
        players[hpi]->setChips(inic);
        std::cout << "Unfortunately, you lost all chips. Chips Topped up." << std::endl;
    }

    // 日志文件：详细上帝视角
    if (g_log) {
        g_log->writeLine("\nGame Over! Final Results:");
        show(g_log->stream());
        for (size_t i = 0; i < winners.size(); i++)
            g_log->writeLine(std::string(players[winners[i]]->getName()) + " won " + std::to_string(share) + " chips");
        if (players[hpi]->getChips() == 0)
            g_log->writeLine("Unfortunately, you lost all chips. Chips Topped up.");
    }
}

template<typename NumT>
void Game<NumT>::nextRound() {
    if (g_log) {
        g_log->writeLine("================================================================");
        g_log->writeLine("--- New Round ---");
        g_log->writeLine("================================================================");
    }
    stateCode = 0;
    dealer = (dealer + 1) % playerNum;
    active = (dealer + 3) % playerNum;
    reset_tags();
    pos.step();
    for (int i = 0; i < playerNum; i++) {
        players[i]->addActionHistory(actInfo{-1, -1, CHECK, 0});
        if (i != hpi && players[i]->getChips() < inic) players[i]->setChips(inic); // 每轮给人机补筹码
    }
    init_blinds();
    deck_.reset();
    deck_.shuffle();
    deck_.deal(playerNum, hands);
    hands[hpi][0].show = 1;
    hands[hpi][1].show = 1;
}

// 显式实例化
template class Game<CARDNUM>;
template class Game<SHORT_CARDNUM>;
