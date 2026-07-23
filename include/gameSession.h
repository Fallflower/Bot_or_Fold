#ifndef HOLDEM_GAME_SESSION_H
#define HOLDEM_GAME_SESSION_H

#include "game.h"
#include <string>

class IGameSession {
public:
    virtual ~IGameSession() = default;

    virtual TableSnapshot snapshot() const = 0;
    virtual void submitAction(const ActionCommand& command) = 0;
    virtual void advanceBot() = 0;
    virtual RoundResult settleRound() = 0;
    virtual void nextRound() = 0;
};

template<typename NumT>
class GameSession final : public IGameSession {
public:
    GameSession(const Position& position, int initialChips,
                const std::string& playerName, int humanPlayerIndex)
        : game_(position, initialChips,
                HumanPlayer<NumT>(playerName, initialChips), humanPlayerIndex) {}

    TableSnapshot snapshot() const override { return game_.snapshot(); }

    void submitAction(const ActionCommand& command) override {
        game_.submitAction(command);
    }

    void advanceBot() override { game_.advanceBot(); }

    RoundResult settleRound() override { return game_.settleRound(); }

    void nextRound() override { game_.nextRound(); }

private:
    Game<NumT> game_;
};

#endif
