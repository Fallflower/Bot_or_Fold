#include "gameController.h"
#include "gameSession.h"
#include <mutex>
#include <stdexcept>
#include <utility>

namespace {

ControllerState stateFrom(const TableSnapshot& table) {
    if (table.roundEnded) return ControllerState::RoundFinished;
    if (table.awaitingHumanAction) return ControllerState::WaitingForHuman;
    return ControllerState::BotThinking;
}

void validateConfig(const GameConfig& config) {
    if (config.playerName.empty())
        throw std::invalid_argument("Player name must not be empty.");
    if (config.playerCount < 2)
        throw std::invalid_argument("Player count must be at least 2.");
    if (config.initialChips < 2)
        throw std::invalid_argument("Initial chips must cover the big blind.");
    if (config.humanPlayerIndex < 0 || config.humanPlayerIndex >= config.playerCount)
        throw std::invalid_argument("Human player position is out of range.");
}

std::unique_ptr<IGameSession> makeSession(const GameConfig& config) {
    Position position(config.playerCount, config.playerCount - 1);
    switch (config.mode) {
    case GameMode::Standard:
        return std::make_unique<GameSession<CARDNUM>>(
            position, config.initialChips, config.playerName, config.humanPlayerIndex);
    case GameMode::ShortDeck:
        return std::make_unique<GameSession<SHORT_CARDNUM>>(
            position, config.initialChips, config.playerName, config.humanPlayerIndex);
    }
    throw std::invalid_argument("Unsupported game mode.");
}

} // namespace

struct GameController::Impl {
    mutable std::mutex viewMutex;
    std::mutex commandMutex;
    std::unique_ptr<IGameSession> session;
    ControllerView published;
    ControllerState recoveryState = ControllerState::Setup;

    ControllerView view() const {
        std::lock_guard<std::mutex> lock(viewMutex);
        return published;
    }

    void publishTable(TableSnapshot table, RoundResult roundResult = {}) {
        std::lock_guard<std::mutex> lock(viewMutex);
        published.hasGame = true;
        published.table = std::move(table);
        published.roundResult = std::move(roundResult);
        published.state = stateFrom(published.table);
        published.errorMessage.clear();
        recoveryState = published.state;
    }

    bool fail(const std::string& message) {
        std::lock_guard<std::mutex> lock(viewMutex);
        if (published.state != ControllerState::Error)
            recoveryState = published.state;
        published.state = ControllerState::Error;
        published.errorMessage = message;
        return false;
    }

    bool hasSession() const {
        std::lock_guard<std::mutex> lock(viewMutex);
        return published.hasGame;
    }
};

GameController::GameController() : impl_(std::make_unique<Impl>()) {}

GameController::~GameController() = default;

bool GameController::startGame(const GameConfig& config) {
    std::lock_guard<std::mutex> commandLock(impl_->commandMutex);
    try {
        validateConfig(config);
        auto session = makeSession(config);
        TableSnapshot table = session->snapshot();
        impl_->session = std::move(session);
        {
            std::lock_guard<std::mutex> viewLock(impl_->viewMutex);
            impl_->published = {};
            impl_->published.config = config;
        }
        impl_->publishTable(std::move(table));
        return true;
    } catch (const std::exception& error) {
        return impl_->fail(error.what());
    }
}

bool GameController::submitHumanAction(const ActionCommand& command) {
    std::lock_guard<std::mutex> commandLock(impl_->commandMutex);
    if (!impl_->hasSession()) return impl_->fail("No game is active.");

    const ControllerView current = impl_->view();
    if (current.state == ControllerState::Error)
        return false;
    if (!current.table.awaitingHumanAction)
        return impl_->fail("The game is not waiting for a human action.");

    try {
        impl_->session->submitAction(command);
        impl_->publishTable(impl_->session->snapshot());
        return true;
    } catch (const std::exception& error) {
        return impl_->fail(error.what());
    }
}

bool GameController::advanceBot() {
    std::lock_guard<std::mutex> commandLock(impl_->commandMutex);
    if (!impl_->hasSession()) return impl_->fail("No game is active.");

    const ControllerView current = impl_->view();
    if (current.state == ControllerState::Error)
        return false;
    if (current.table.roundEnded)
        return impl_->fail("The round has already ended.");
    if (current.table.awaitingHumanAction)
        return impl_->fail("The game is waiting for a human action.");

    try {
        impl_->session->advanceBot();
        impl_->publishTable(impl_->session->snapshot());
        return true;
    } catch (const std::exception& error) {
        return impl_->fail(error.what());
    }
}

bool GameController::settleRound() {
    std::lock_guard<std::mutex> commandLock(impl_->commandMutex);
    if (!impl_->hasSession()) return impl_->fail("No game is active.");

    const ControllerView current = impl_->view();
    if (current.state == ControllerState::Error)
        return false;
    if (!current.table.roundEnded)
        return impl_->fail("The round has not ended.");

    try {
        RoundResult result = impl_->session->settleRound();
        impl_->publishTable(impl_->session->snapshot(), std::move(result));
        return true;
    } catch (const std::exception& error) {
        return impl_->fail(error.what());
    }
}

bool GameController::nextRound() {
    std::lock_guard<std::mutex> commandLock(impl_->commandMutex);
    if (!impl_->hasSession()) return impl_->fail("No game is active.");

    const ControllerView current = impl_->view();
    if (current.state == ControllerState::Error)
        return false;
    if (!current.table.roundSettled)
        return impl_->fail("The current round must be settled first.");

    try {
        impl_->session->nextRound();
        impl_->publishTable(impl_->session->snapshot());
        return true;
    } catch (const std::exception& error) {
        return impl_->fail(error.what());
    }
}

void GameController::returnToSetup() {
    std::lock_guard<std::mutex> commandLock(impl_->commandMutex);
    impl_->session.reset();
    std::lock_guard<std::mutex> viewLock(impl_->viewMutex);
    impl_->published = {};
    impl_->recoveryState = ControllerState::Setup;
}

void GameController::dismissError() {
    std::lock_guard<std::mutex> viewLock(impl_->viewMutex);
    if (impl_->published.state != ControllerState::Error) return;
    impl_->published.state = impl_->recoveryState;
    impl_->published.errorMessage.clear();
}

ControllerView GameController::view() const {
    return impl_->view();
}
