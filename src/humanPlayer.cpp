#include "humanPlayer.h"
#include "assistant.h"

template<typename NumT>
ACTION HumanPlayer<NumT>::makeAction(const gameInfo<NumT>& info, int &betAmount)
{
    // 根据legalActions动态生成菜单
    std::cout << "==========================" << std::endl;
    int i = 1;
    for (auto act : info.legalActions) {
        switch (act) {
        case FOLD:
            std::cout << "=== Fold           [" << i << "] ===" << std::endl; break;
        case CHECK:
            std::cout << "=== Check          [" << i << "] ===" << std::endl; break;
        case CALL:
            std::cout << "=== Call  " << std::right << std::setw(5) << info.chipsToCall << " BB [" << i << "] ===" << std::endl; break;
        case RAISE:
            std::cout << "=== Raise          [" << i << "] ===" << std::endl; break;
        }
        i++;
    }
    std::cout << "=== QUIT         [Esc] ===" << std::endl;
    std::cout << "==========================" << std::endl;

    // 构建选择字符串
    std::string choices;
    for (int j = 1; j < i; j++) choices += std::to_string(j)[0];
    choices += "\x1b";

    int k = Choice("Please Choose:", choices.c_str());
    if (k == '\x1b') exit(0);

    int idx = k - '1';
    if (idx < 0 || idx >= (int)info.legalActions.size())
        throw Error(3, "System Error: Invalid action selected.");

    switch (info.legalActions[idx]) {
    case FOLD:
        return FOLD;
    case CHECK:
        return CHECK;
    case CALL: {
        // 后手不够叫则全下跟注，否则正常跟注
        if (info.chipsToCall >= this->chips) {
            betAmount = this->chips;
            this->setChips(0);
        } else {
            this->decChips(info.chipsToCall);
        }
        return CALL;
    }
    case RAISE: {
        int minRaise = (info.chipsToCall > 0) ? info.chipsToCall * 2 : 1;
        std::cout << "Bet Amount (min " << minRaise << "): " << std::flush;
        std::cin >> betAmount;
        if (betAmount < minRaise)
            throw Error(2, "User Error: Invalid betting scale.");
        if (betAmount >= this->chips) {
            betAmount = this->chips;
            this->setChips(0);
        } else {
            this->decChips(betAmount);
        }
        return RAISE;
    }
    }
    return FOLD;
}

template class HumanPlayer<CARDNUM>;
template class HumanPlayer<SHORT_CARDNUM>;
