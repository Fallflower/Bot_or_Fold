#include "game.h"
#include "assistant.h"
#include "cliMenu.h"
#include "ranker_data.h"
#include <cstdlib>

int main(int argc, char* argv[]) {
    setConsoleToUTF8();

    if (!initAdvancedRanker(ranker_bin, ranker_bin_len)) {
        std::cerr << "Fatal: failed to load embedded ranker data" << std::endl;
        return 1;
    }

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
    try {
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
    } catch (const Error& e) {
        std::cerr << e.what() << std::endl;
        g.show();
        exit(static_cast<int>(e.code()));
    }
    return 0;
}