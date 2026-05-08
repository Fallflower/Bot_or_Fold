#include "game.h"
#include "assistant.h"
#include "cliMenu.h"
#include <cstdlib>

extern advancedHandType g_advancedRanker;

int main(int argc, char* argv[]) {
    setConsoleToUTF8();

    g_advancedRanker.load("./resources/card5_dic_zipped.bin");

    std::string name = (argc > 1) ? argv[1] : "";
    int pn = (argc > 2) ? std::atoi(argv[2]) : 0;
    int chips = (argc > 3) ? std::atoi(argv[3]) : 0;
    int hppi = (argc > 4) ? std::atoi(argv[4]) : -1;

    if (name.empty()) {
        std::cout << " Player Name:" << std::flush;
        std::cin >> name;
    }
    if (!pn) {
        std::cout << " Players Num:" << std::flush;
        std::cin >> pn;
    }
    if (!chips) {
        std::cout << " Initial Chips:" << std::flush;
        std::cin >> chips;
    }
    Position pos(pn, pn-1);
    if (hppi < 0) {
        showPositionMenu(pos);
        std::cin >> hppi;
    }

    Game g(pos, chips, HumanPlayer(name, chips), hppi);
    while (1) {
        while (!g.isEnd()) {
            clearScreen();
            g.showPlayerView();
            g.toAct();
        }
        clearScreen();
        g.showPlayerView();
        g.afterEnd();

        ifContinueMenu();
        int k = 0;
        k = Choice("Please Choose:", "12\x1b");
        switch (k)
        {
        case '1':
            g.nextRound();
            break;
        default: exit(0);    
        }
    }
    return 0;
}