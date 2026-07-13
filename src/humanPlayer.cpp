#include "humanPlayer.h"
#include "assistant.h"

template<typename NumT>
ACTION HumanPlayer<NumT>::makeAction(const gameInfo<NumT>& info, int &betAmount)
{
    // 根据legalActions动态生成菜单
    std::cout << "==========================" << std::endl;
    char i = '1';
    for (auto act : info.legalActions) {
        switch (act) {
        case FOLD:
            std::cout << "=== Fold           [" << i << "] ===" << std::endl; break;
        case CALL:
            if (info.chipsToCall > 0) std::cout << "=== Call  " << std::right << std::setw(6) << info.chipsToCall << "   [" << i << "] ===" << std::endl;
            else std::cout << "=== Check          [" << i << "] ===" << std::endl;
            break;
        case RAISE:
            std::cout << "=== Raise          [" << i << "] ===" << std::endl; break;
        }
        i++;
    }
    std::cout << "=== QUIT         [Esc] ===" << std::endl;
    std::cout << "==========================" << std::endl;

    // 构建选择字符串
    std::string choices;
    for (char j = '1'; j < i; j++) choices += j;
    choices += "\x1b";

    int k = Choice("Please Choose: ", choices.c_str());
    if (k == '\x1b') exit(0);

    int idx = k - '1';

    switch (info.legalActions[idx]) {
    case FOLD:
        return FOLD;
    case CALL:
        return CALL;
    case RAISE: {
        std::cout << "Bet Amount: " << std::flush;
        std::cin >> betAmount;
        return RAISE;
    }
    }
    return FOLD;
}

template class HumanPlayer<CARDNUM>;
template class HumanPlayer<SHORT_CARDNUM>;
