#include "humanPlayer.h"
#include "assistant.h"

int HumanPlayer::showActionMenu(const int& chipsToCall) const
{
    int action_type = 0;
	std::cout << "==========================" << std::endl;
    if(chipsToCall == 0) {
	    std::cout << "=== Check          [1] ===" << std::endl;
    	std::cout << "=== Bet            [2] ===" << std::endl;
    	std::cout << "=== Allin          [3] ===" << std::endl;
        action_type = 1;
    } else if ( chipsToCall < chips) {
        std::cout << "=== Fold           [1] ===" << std::endl;
        std::cout << "=== Call  " << std::right << std::setw(5) << chipsToCall << " BB [2] ===" << std::endl;
    	std::cout << "=== Raise          [3] ===" << std::endl;
    	std::cout << "=== Allin          [4] ===" << std::endl;
        action_type = 2;
    } else {
        std::cout << "=== Fold           [1] ===" << std::endl;
        std::cout << "=== Allin to Call  [2] ===" << std::endl;
        action_type = 3;
    }
	std::cout << "=== QUIT         [Esc] ===" << std::endl;
	std::cout << "==========================" << std::endl;
    return action_type;
}

ACTION HumanPlayer::makeAction(const int& chipsToCall, int &betAmount)
{
    int k, ba;
    switch (showActionMenu(chipsToCall))
    {
    case 1:
        k = Choice("Please Choose:", "123\x1b");
        switch (k) // 注意，所有的bet和raise都需要输入金额，统一到最后处理
        {
        case '1':
            return CHECK;
        case '3':
            betAmount = chips;
            setChips(0);
            return ALLIN;
        case '\x1b':
            exit(0);
        default:
            break;
        }
        break;
    case 2:
        k = Choice("Please Choose:", "1234\x1b");
        switch (k)
        {
        case '1':
            return FOLD;
        case '2':
            decChips(chipsToCall);
            return CALL;
        case '4':
            betAmount = chips;
            setChips(0);
            return ALLIN;
        case '\x1b':
            exit(0);
        default:
            break;
        }
        break;
    case 3:
        k = Choice("Please Choose:", "12\x1b");
        switch (k)
        {
        case '1':
            return CHECK;
        case '2':
            betAmount = chips;
            setChips(0);
            return ALLINTOCALL;
        case '\x1b':
            exit(0);
        default:
            break;
        }
        break;
    case 0:
        throw Error(3, "System Error: Invalid 'chipsToCall' value.");
    default:
        break;
    }

    std::cout << "Bet Amount: " << std::flush;
    std::cin >> ba;
    if (ba <= chipsToCall || ba > chips)
        throw Error(2, "User Error: Invalid betting scale.");
    betAmount = ba;
    if (ba >= chips) {setChips(0); betAmount=chips; return ALLIN;}
    decChips(betAmount);
    if (chipsToCall > 0) return RAISE;
    return BET;
}
