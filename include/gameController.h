#ifndef HOLDEM_GAME_CONTROLLER_H
#define HOLDEM_GAME_CONTROLLER_H

#include "gameState.h"
#include <memory>
#include <string>

enum class GameMode {
    Standard,
    ShortDeck
};

struct GameConfig {
    std::string playerName;
    int playerCount = 2;
    int initialChips = 200;
    int humanPlayerIndex = 0;
    GameMode mode = GameMode::Standard;
};

enum class ControllerState {
    Setup,
    BotThinking,
    WaitingForHuman,
    RoundFinished,
    Error
};

struct ControllerView {
    ControllerState state = ControllerState::Setup;
    bool hasGame = false;
    GameConfig config;
    TableSnapshot table;
    RoundResult roundResult;
    std::string errorMessage;
};

class GameController {
public:
    GameController();
    ~GameController();

    GameController(const GameController&) = delete;
    GameController& operator=(const GameController&) = delete;

    bool startGame(const GameConfig& config);
    bool submitHumanAction(const ActionCommand& command);
    bool advanceBot();
    bool settleRound();
    bool nextRound();

    void returnToSetup();
    void dismissError();
    // Returns the last published copy without reading the live game session.
    ControllerView view() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

#endif
