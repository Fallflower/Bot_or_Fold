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

    bool ifContinue = true;
    Game g(pos, chips, HumanPlayer(name, chips), hppi);
    while (ifContinue ) {
        while (!g.isEnd()) {
            clearScreen();
            g.showPlayerView();
            g.toAct();
        }
        clearScreen();
        g.showPlayerView();
        g.afterEnd();

        std::cout << "\nNext round? (y/n): " << std::flush;
        std::string ans;
        std::cin >> ans;
        if (ans != "y" && ans != "Y" && ans != "yes") {
            ifContinue = false;
        }
    }
    return 0;
}