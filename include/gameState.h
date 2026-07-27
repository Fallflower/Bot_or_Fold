#ifndef HOLDEM_GAME_STATE_H
#define HOLDEM_GAME_STATE_H

#include "player.h"
#include <optional>
#include <string>
#include <vector>

struct ActionCommand {
    ACTION action = FOLD;
    // Chips paid by this action. Only used for RAISE.
    int amount = 0;
};

struct DecisionRequest {
    int playerIndex = -1;
    bool human = false;
    int chipsToCall = 0;
    int minRaise = 0;
    int maxRaise = 0;
    std::vector<ACTION> legalActions;
};

struct CardSnapshot {
    int rank = -1;
    int suit = -1;
    bool visible = false;
};

struct PlayerSnapshot {
    int index = -1;
    std::string name;
    std::string position;
    int chips = 0;
    int committed = 0;
    bool folded = false;
    bool allIn = false;
    bool active = false;
    bool human = false;
    bool hasLastAction = false;
    std::optional<HandTypeDisplayData> handType;
    actInfo lastAction;
    std::vector<CardSnapshot> cards;
};

struct SidePot {
    int amount = 0;
    std::vector<int> eligiblePlayers;
};

struct TableSnapshot {
    int stateCode = 0;
    std::string state;
    int pot = 0;
    int dealerIndex = -1;
    int activePlayerIndex = -1;
    int humanPlayerIndex = -1;
    bool roundEnded = false;
    bool roundSettled = false;
    bool awaitingHumanAction = false;
    DecisionRequest decision;
    std::vector<CardSnapshot> publicCards;
    std::vector<PlayerSnapshot> players;
    std::vector<SidePot> sidePots;
};

struct PlayerAward {
    int playerIndex = -1;
    int amount = 0;
};

struct PotResult {
    int amount = 0;
    std::vector<int> eligiblePlayers;
    std::vector<int> winners;
    std::vector<PlayerAward> awards;
};

struct RoundResult {
    bool settled = false;
    bool humanToppedUp = false;
    std::vector<PotResult> pots;
};

#endif
