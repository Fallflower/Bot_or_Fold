#include "game.h"
#include "assistant.h"
#include <cassert>

int main() {
    Position position(2, 1);
    Game<CARDNUM> game(position, 200, HumanPlayer<CARDNUM>("Human", 200), 0);

    const TableSnapshot initial = game.snapshot();
    assert(initial.players.size() == 2);
    assert(initial.publicCards.size() == 5);
    assert(initial.players[0].human);
    assert(initial.players[0].cards[0].visible);
    assert(!initial.players[0].handDescription.empty());
    assert(!initial.players[1].cards[0].visible);
    assert(initial.players[1].cards[0].rank == -1);
    assert(initial.players[1].handDescription.empty());

    const DecisionRequest firstDecision = game.decisionRequest();
    assert(firstDecision.playerIndex == 0);
    assert(firstDecision.human);
    assert(firstDecision.maxRaise == initial.players[0].chips);

    bool rejectedInvalidRaise = false;
    try {
        game.submitAction({RAISE, firstDecision.chipsToCall});
    } catch (const Error&) {
        rejectedInvalidRaise = true;
    }
    assert(rejectedInvalidRaise);

    while (!game.isEnd()) {
        const DecisionRequest decision = game.decisionRequest();
        assert(decision.playerIndex >= 0);
        game.submitAction({CALL, 0});
    }

    const TableSnapshot showdown = game.snapshot();
    assert(showdown.roundEnded);
    assert(showdown.players[1].cards[0].visible);
    assert(!showdown.players[1].handDescription.empty());

    const RoundResult firstSettlement = game.settleRound();
    const TableSnapshot settled = game.snapshot();
    const RoundResult secondSettlement = game.settleRound();
    const TableSnapshot settledAgain = game.snapshot();
    assert(firstSettlement.settled);
    assert(secondSettlement.settled);
    assert(settled.roundSettled);
    assert(settled.players[0].chips == settledAgain.players[0].chips);
    assert(settled.players[1].chips == settledAgain.players[1].chips);

    game.nextRound();
    const TableSnapshot nextRound = game.snapshot();
    assert(!nextRound.roundEnded);
    assert(!nextRound.roundSettled);
    assert(!nextRound.players[1].cards[0].visible);

    Position botFirstPosition(2, 1);
    Game<CARDNUM> botFirstGame(
        botFirstPosition, 200, HumanPlayer<CARDNUM>("Human", 200), 1);
    assert(!botFirstGame.isHumanTurn());
    const int botIndex = botFirstGame.decisionRequest().playerIndex;
    botFirstGame.advanceBot();
    const TableSnapshot afterBot = botFirstGame.snapshot();
    assert(afterBot.players[botIndex].hasLastAction);
    assert(afterBot.players[botIndex].lastAction.id == botIndex);
    return 0;
}
