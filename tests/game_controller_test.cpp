#include "gameController.h"
#include <cassert>

int main() {
    GameController controller;
    assert(controller.view().state == ControllerState::Setup);

    GameConfig invalid;
    invalid.playerName = "";
    assert(!controller.startGame(invalid));
    assert(controller.view().state == ControllerState::Error);
    controller.dismissError();
    assert(controller.view().state == ControllerState::Setup);

    GameConfig standard;
    standard.playerName = "Human";
    standard.playerCount = 2;
    standard.initialChips = 200;
    standard.humanPlayerIndex = 0;
    standard.mode = GameMode::Standard;
    assert(controller.startGame(standard));

    ControllerView current = controller.view();
    assert(current.hasGame);
    assert(current.state == ControllerState::WaitingForHuman);
    assert(current.table.players.size() == 2);
    assert(current.table.players[0].cards[0].visible);
    assert(!current.table.players[1].cards[0].visible);

    assert(!controller.submitHumanAction({RAISE, current.table.decision.chipsToCall}));
    assert(controller.view().state == ControllerState::Error);
    controller.dismissError();
    assert(controller.view().state == ControllerState::WaitingForHuman);

    int turns = 0;
    while (controller.view().state != ControllerState::RoundFinished && turns++ < 64) {
        current = controller.view();
        if (current.state == ControllerState::WaitingForHuman) {
            assert(controller.submitHumanAction({CALL, 0}));
        } else if (current.state == ControllerState::BotThinking) {
            assert(controller.advanceBot());
        } else {
            assert(false);
        }
    }
    assert(turns < 64);
    assert(controller.settleRound());
    assert(controller.view().table.roundSettled);
    assert(controller.settleRound());
    assert(controller.nextRound());
    assert(!controller.view().table.roundEnded);

    GameConfig shortDeck = standard;
    shortDeck.humanPlayerIndex = 1;
    shortDeck.mode = GameMode::ShortDeck;
    assert(controller.startGame(shortDeck));
    current = controller.view();
    assert(current.state == ControllerState::BotThinking);
    assert(current.table.players[1].cards[0].visible);
    assert(current.table.players[1].cards[0].rank >= 4);
    assert(!current.table.players[0].cards[0].visible);

    assert(controller.advanceBot());
    current = controller.view();
    assert(current.state == ControllerState::WaitingForHuman
           || current.state == ControllerState::RoundFinished);
    assert(current.table.players[0].hasLastAction);
    controller.returnToSetup();
    current = controller.view();
    assert(current.state == ControllerState::Setup);
    assert(!current.hasGame);
    return 0;
}
