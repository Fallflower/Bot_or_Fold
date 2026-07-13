#include "game.h"
#include "assistant.h"
#include "cliMenu.h"
#include <cstdlib>

template<typename NumT>
void runGame(Position& pos, int chips, const std::string& name, int hppi) {
    GameLog log("game_log.txt");
    g_log = &log;

    Game<NumT> g(pos, chips, HumanPlayer<NumT>(name, chips), hppi);

    try {
        int flag = 1;
        while (flag) {
            g.show(log.stream());
            while (!g.isEnd()) {
                clearScreen();
                g.showPlayerView();
                g.toAct();
            }
            clearScreen();
            g.showPlayerView();
            g.afterEnd();
            log.flush();

            ifContinueMenu();
            int k = 0;
            k = Choice("Please Choose:", "12\x1b");
            switch (k)
            {
            case '1':
                g.nextRound();
                break;
            default:
                log.writeLine("=== Game Ended by User ===");
                flag = 0;
                break;
            }
        }
    } catch (const Error& e) {
        std::cerr << e.what() << std::endl;
        g.show();
        log.writeLine(std::string("=== Error: ===\n") + e.what() + "\n=== End Er ===");
        exit(static_cast<int>(e.code()));
    }
}

int main(int argc, char* argv[]) {
    setConsoleToUTF8();

    std::string name = (argc > 1) ? argv[1] : "";
    int pn = (argc > 2) ? std::atoi(argv[2]) : 0;
    int chips = (argc > 3) ? std::atoi(argv[3]) : 0;
    int hppi = (argc > 4) ? std::atoi(argv[4]) : -1;
    int mode = (argc > 5) ? std::atoi(argv[5]) : 0;

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
    if (!mode) {
        showGameModeMenu();
        int k;
        k = Choice("Please Choose:", "12\x1b");
        switch (k)
        {
        case '1': mode = 1; break;
        case '2': mode = 2; break;
        default: exit(0);
        }
    }

    switch (mode) {
    case 1:
        runGame<CARDNUM>(pos, chips, name, hppi);
        break;
    case 2:
        runGame<SHORT_CARDNUM>(pos, chips, name, hppi);
        break;
    default:
        exit(0);
    }
    return 0;
}