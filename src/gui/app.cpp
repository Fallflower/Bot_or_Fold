#include "eui_neo.h"
#include "gameController.h"

#include <algorithm>
#include <string>

namespace app {
namespace {

constexpr eui::Color kBackground{0.055f, 0.070f, 0.065f, 1.0f};
constexpr eui::Color kSurface{0.095f, 0.110f, 0.105f, 1.0f};
constexpr eui::Color kSurfaceRaised{0.135f, 0.150f, 0.142f, 1.0f};
constexpr eui::Color kBorder{0.25f, 0.29f, 0.27f, 1.0f};
constexpr eui::Color kText{0.94f, 0.96f, 0.95f, 1.0f};
constexpr eui::Color kMuted{0.62f, 0.68f, 0.65f, 1.0f};
constexpr eui::Color kGreen{0.12f, 0.55f, 0.34f, 1.0f};
constexpr eui::Color kGreenHover{0.15f, 0.64f, 0.40f, 1.0f};
constexpr eui::Color kGreenPressed{0.09f, 0.45f, 0.28f, 1.0f};
constexpr eui::Color kRed{0.66f, 0.20f, 0.22f, 1.0f};
constexpr eui::Color kRedHover{0.76f, 0.25f, 0.27f, 1.0f};
constexpr eui::Color kRedPressed{0.55f, 0.15f, 0.17f, 1.0f};
constexpr eui::Color kGold{0.82f, 0.60f, 0.17f, 1.0f};
constexpr eui::Color kGoldHover{0.91f, 0.68f, 0.22f, 1.0f};
constexpr eui::Color kGoldPressed{0.70f, 0.49f, 0.12f, 1.0f};

struct UiState {
    std::string playerName = "Player";
    int playerCount = 6;
    int initialChips = 200;
    int humanPlayerIndex = 5;
    int mode = 0;
    int raiseAmount = 0;
    int decisionPlayer = -1;
    int decisionStreet = -1;
    int decisionToCall = -1;
    bool botTaskPending = false;
};

GameController controller;
UiState state;

void text(eui::Ui& ui, const std::string& id, const std::string& value,
          float width, float height, float fontSize,
          const eui::Color& color = kText,
          eui::HorizontalAlign align = eui::HorizontalAlign::Left) {
    ui.text(id)
        .size(width, height)
        .text(value)
        .fontSize(fontSize)
        .lineHeight(fontSize + 4.0f)
        .color(color)
        .horizontalAlign(align)
        .verticalAlign(eui::VerticalAlign::Center)
        .build();
}

bool hasAction(const DecisionRequest& decision, ACTION action) {
    return std::find(decision.legalActions.begin(), decision.legalActions.end(), action)
        != decision.legalActions.end();
}

std::string actionLabel(const PlayerSnapshot& player) {
    if (player.folded) return "Folded";
    if (!player.hasLastAction) return player.allIn ? "All-in" : "Waiting";
    if (player.lastAction.act == CALL && player.lastAction.betAmount == 0)
        return "Check";

    std::string label = action2str(player.lastAction.act);
    if (player.lastAction.act != FOLD)
        label += " " + std::to_string(player.lastAction.betAmount);
    if (player.allIn) label += " / All-in";
    return label;
}

void syncRaiseAmount(const ControllerView& view) {
    if (!view.hasGame || view.table.roundEnded) return;
    const DecisionRequest& decision = view.table.decision;
    const bool changed = state.decisionPlayer != decision.playerIndex
        || state.decisionStreet != view.table.stateCode
        || state.decisionToCall != decision.chipsToCall;
    if (!changed) return;

    state.decisionPlayer = decision.playerIndex;
    state.decisionStreet = view.table.stateCode;
    state.decisionToCall = decision.chipsToCall;
    state.raiseAmount = decision.minRaise > 0 ? decision.minRaise : 0;
}

void scheduleBot(const ControllerView& view) {
    if (view.state != ControllerState::BotThinking || state.botTaskPending) return;

    state.botTaskPending = true;
    const bool accepted = app::async::restart(
        "holdem.bot.advance",
        [] { return controller.advanceBot(); },
        [](const app::async::Result<bool>&) { state.botTaskPending = false; });
    if (!accepted) state.botTaskPending = false;
}

void setupRow(eui::Ui& ui, const std::string& id, const std::string& label,
              float width, const std::function<void(float)>& composeControl) {
    const float labelWidth = 150.0f;
    const float controlWidth = width - labelWidth - 14.0f;
    ui.row(id)
        .size(width, 44.0f)
        .gap(14.0f)
        .alignItems(eui::Align::CENTER)
        .content([&] {
            text(ui, id + ".label", label, labelWidth, 42.0f, 15.0f, kMuted);
            composeControl(controlWidth);
        })
        .build();
}

void composeSetup(eui::Ui& ui, const eui::Screen& screen) {
    const float panelWidth = std::min(560.0f, std::max(360.0f, screen.width - 32.0f));
    const float panelHeight = std::min(610.0f, std::max(540.0f, screen.height - 32.0f));
    const float innerWidth = panelWidth - 48.0f;

    ui.stack("setup.root")
        .size(screen.width, screen.height)
        .align(eui::Align::CENTER, eui::Align::CENTER)
        .content([&] {
            ui.rect("setup.background").size(screen.width, screen.height).color(kBackground).build();
            ui.stack("setup.panel")
                .size(panelWidth, panelHeight)
                .content([&] {
                    ui.rect("setup.panel.background")
                        .size(panelWidth, panelHeight)
                        .color(kSurface)
                        .radius(8.0f)
                        .border(1.0f, kBorder)
                        .shadow(28.0f, 0.0f, 10.0f, {0.0f, 0.0f, 0.0f, 0.34f})
                        .build();
                    ui.column("setup.form")
                        .size(panelWidth, panelHeight)
                        .padding(24.0f)
                        .gap(12.0f)
                        .content([&] {
                            text(ui, "setup.title", "Bot or Fold", innerWidth, 42.0f, 30.0f);
                            text(ui, "setup.subtitle", "New table", innerWidth, 24.0f, 14.0f, kMuted);

                            setupRow(ui, "setup.name", "Player name", innerWidth, [&](float width) {
                                components::input(ui, "setup.name.input")
                                    .size(width, 42.0f)
                                    .value(state.playerName)
                                    .placeholder("Player")
                                    .onChange([](const std::string& value) { state.playerName = value; })
                                    .build();
                            });
                            setupRow(ui, "setup.mode", "Game mode", innerWidth, [&](float width) {
                                components::segmented(ui, "setup.mode.segmented")
                                    .size(width, 40.0f)
                                    .items({"Standard", "Short Deck"})
                                    .selected(state.mode)
                                    .onChange([](int value) { state.mode = value; })
                                    .build();
                            });
                            setupRow(ui, "setup.players", "Players", innerWidth, [&](float width) {
                                components::stepper(ui, "setup.players.stepper")
                                    .size(width, 40.0f)
                                    .value(state.playerCount)
                                    .min(2).max(9)
                                    .onChange([](long long value) {
                                        state.playerCount = static_cast<int>(value);
                                        state.humanPlayerIndex = std::min(
                                            state.humanPlayerIndex, state.playerCount - 1);
                                    })
                                    .build();
                            });
                            setupRow(ui, "setup.chips", "Starting chips", innerWidth, [&](float width) {
                                components::stepper(ui, "setup.chips.stepper")
                                    .size(width, 40.0f)
                                    .value(state.initialChips)
                                    .step(50).min(50).max(5000)
                                    .onChange([](long long value) {
                                        state.initialChips = static_cast<int>(value);
                                    })
                                    .build();
                            });
                            setupRow(ui, "setup.seat", "Seat index", innerWidth, [&](float width) {
                                components::stepper(ui, "setup.seat.stepper")
                                    .size(width, 40.0f)
                                    .value(state.humanPlayerIndex)
                                    .min(0).max(state.playerCount - 1)
                                    .onChange([](long long value) {
                                        state.humanPlayerIndex = static_cast<int>(value);
                                    })
                                    .build();
                            });

                            ui.rect("setup.divider").size(innerWidth, 1.0f).color(kBorder).build();
                            components::button(ui, "setup.start")
                                .size(innerWidth, 48.0f)
                                .text("Start game")
                                .radius(6.0f)
                                .colors(kGreen, kGreenHover, kGreenPressed)
                                .onClick([] {
                                    GameConfig config;
                                    config.playerName = state.playerName;
                                    config.playerCount = state.playerCount;
                                    config.initialChips = state.initialChips;
                                    config.humanPlayerIndex = state.humanPlayerIndex;
                                    config.mode = state.mode == 0
                                        ? GameMode::Standard : GameMode::ShortDeck;
                                    controller.startGame(config);
                                    state.decisionPlayer = -1;
                                })
                                .build();
                        })
                        .build();
                })
                .build();
        })
        .build();
}

void playerRow(eui::Ui& ui, const PlayerSnapshot& player, float width, int row) {
    const std::string id = "game.player." + std::to_string(player.index);
    const eui::Color background = player.active
        ? eui::Color{0.12f, 0.28f, 0.20f, 1.0f}
        : (row % 2 == 0 ? kSurface : kSurfaceRaised);

    ui.stack(id)
        .size(width, 36.0f)
        .content([&] {
            ui.rect(id + ".background")
                .size(width, 36.0f)
                .color(background)
                .radius(5.0f)
                .border(player.active ? 1.0f : 0.0f, player.active ? kGreen : kBorder)
                .build();
            ui.row(id + ".content")
                .size(width, 36.0f)
                .padding(8.0f)
                .gap(8.0f)
                .alignItems(eui::Align::CENTER)
                .content([&] {
                    text(ui, id + ".name", player.name, width * 0.23f, 30.0f, 14.0f);
                    text(ui, id + ".position", player.position, width * 0.13f, 30.0f, 13.0f, kMuted);
                    text(ui, id + ".chips", std::to_string(player.chips) + " chips",
                         width * 0.18f, 30.0f, 13.0f);
                    text(ui, id + ".committed", "In " + std::to_string(player.committed),
                         width * 0.15f, 30.0f, 13.0f, kMuted);
                    text(ui, id + ".action", actionLabel(player), width * 0.23f, 30.0f,
                         13.0f, player.folded ? kRedHover : kText,
                         eui::HorizontalAlign::Right);
                })
                .build();
        })
        .build();
}

void humanActions(eui::Ui& ui, const ControllerView& view, float width) {
    const DecisionRequest& decision = view.table.decision;
    const bool canFold = hasAction(decision, FOLD);
    const bool canCall = hasAction(decision, CALL);
    const bool canRaise = hasAction(decision, RAISE);
    const float raiseControlWidth = std::max(210.0f, width - 430.0f);

    ui.row("game.actions")
        .size(width, 46.0f)
        .gap(10.0f)
        .alignItems(eui::Align::CENTER)
        .content([&] {
            components::button(ui, "game.action.fold")
                .size(110.0f, 44.0f).text("Fold").radius(6.0f)
                .colors(kRed, kRedHover, kRedPressed).disabled(!canFold)
                .onClick([] { controller.submitHumanAction({FOLD, 0}); }).build();
            components::button(ui, "game.action.call")
                .size(150.0f, 44.0f)
                .text(decision.chipsToCall == 0
                    ? "Check" : "Call " + std::to_string(decision.chipsToCall))
                .radius(6.0f).colors(kGreen, kGreenHover, kGreenPressed)
                .disabled(!canCall)
                .onClick([] { controller.submitHumanAction({CALL, 0}); }).build();
            components::stepper(ui, "game.action.raise.amount")
                .size(raiseControlWidth, 42.0f)
                .value(state.raiseAmount)
                .min(canRaise ? decision.minRaise : 0)
                .max(canRaise ? decision.maxRaise : 0)
                .onChange([](long long value) { state.raiseAmount = static_cast<int>(value); })
                .build();
            components::button(ui, "game.action.raise")
                .size(140.0f, 44.0f)
                .text(state.raiseAmount == decision.maxRaise ? "All-in" : "Raise")
                .radius(6.0f).colors(kGold, kGoldHover, kGoldPressed)
                .disabled(!canRaise)
                .onClick([] { controller.submitHumanAction({RAISE, state.raiseAmount}); }).build();
        })
        .build();
}

void roundActions(eui::Ui& ui, const ControllerView& view, float width) {
    ui.row("game.round.actions")
        .size(width, 46.0f)
        .gap(10.0f)
        .content([&] {
            if (!view.table.roundSettled) {
                components::button(ui, "game.round.settle")
                    .size(220.0f, 44.0f).text("Show result").radius(6.0f)
                    .colors(kGold, kGoldHover, kGoldPressed)
                    .onClick([] { controller.settleRound(); }).build();
            } else {
                components::button(ui, "game.round.next")
                    .size(220.0f, 44.0f).text("Next round").radius(6.0f)
                    .colors(kGreen, kGreenHover, kGreenPressed)
                    .onClick([] {
                        controller.nextRound();
                        state.decisionPlayer = -1;
                    }).build();
            }
            components::button(ui, "game.round.setup")
                .size(180.0f, 44.0f).text("Table setup").radius(6.0f)
                .theme(components::theme::dark(), false)
                .onClick([] { controller.returnToSetup(); }).build();
        })
        .build();
}

void composeGame(eui::Ui& ui, const eui::Screen& screen, const ControllerView& view) {
    syncRaiseAmount(view);
    const float shellWidth = std::min(1040.0f, std::max(760.0f, screen.width - 32.0f));
    const float shellHeight = std::min(690.0f, std::max(580.0f, screen.height - 32.0f));
    const float innerWidth = shellWidth - 40.0f;

    ui.stack("game.root")
        .size(screen.width, screen.height)
        .align(eui::Align::CENTER, eui::Align::CENTER)
        .content([&] {
            ui.rect("game.background")
                .size(screen.width, screen.height)
                .color({0.035f, 0.12f, 0.075f, 1.0f})
                .build();
            ui.column("game.shell")
                .size(shellWidth, shellHeight)
                .padding(20.0f)
                .gap(10.0f)
                .content([&] {
                    ui.row("game.header")
                        .size(innerWidth, 48.0f)
                        .alignItems(eui::Align::CENTER)
                        .content([&] {
                            text(ui, "game.title", "Bot or Fold", innerWidth * 0.45f, 44.0f, 26.0f);
                            text(ui, "game.street", view.table.state, innerWidth * 0.20f,
                                 44.0f, 15.0f, kMuted, eui::HorizontalAlign::Center);
                            text(ui, "game.pot", "Pot " + std::to_string(view.table.pot),
                                 innerWidth * 0.35f, 44.0f, 20.0f, kGold,
                                 eui::HorizontalAlign::Right);
                        }).build();
                    ui.rect("game.header.divider").size(innerWidth, 1.0f).color(kBorder).build();
                    text(ui, "game.status",
                         view.state == ControllerState::BotThinking ? "Bot is thinking..." :
                         view.state == ControllerState::WaitingForHuman ? "Your action" : "Round complete",
                         innerWidth, 26.0f, 14.0f,
                         view.state == ControllerState::BotThinking ? kGold : kText);
                    ui.column("game.players")
                        .size(innerWidth, static_cast<float>(view.table.players.size()) * 40.0f)
                        .gap(4.0f)
                        .content([&] {
                            for (size_t i = 0; i < view.table.players.size(); ++i)
                                playerRow(ui, view.table.players[i], innerWidth, static_cast<int>(i));
                        }).build();

                    const PlayerSnapshot& human = view.table.players[view.table.humanPlayerIndex];
                    text(ui, "game.hand", human.handDescription.empty()
                             ? "Your hand" : "Your hand: " + human.handDescription,
                         innerWidth, 28.0f, 14.0f, kMuted);
                    ui.rect("game.actions.divider").size(innerWidth, 1.0f).color(kBorder).build();

                    if (view.state == ControllerState::WaitingForHuman)
                        humanActions(ui, view, innerWidth);
                    else if (view.state == ControllerState::RoundFinished)
                        roundActions(ui, view, innerWidth);
                    else
                        text(ui, "game.bot.wait", "Calculating next action", innerWidth,
                             46.0f, 15.0f, kMuted, eui::HorizontalAlign::Center);
                }).build();
        }).build();

    scheduleBot(view);
}

void composeError(eui::Ui& ui, const eui::Screen& screen, const ControllerView& view) {
    if (view.state != ControllerState::Error) return;
    ui.stack("error.overlay")
        .size(screen.width, screen.height)
        .align(eui::Align::CENTER, eui::Align::CENTER)
        .zIndex(1000)
        .content([&] {
            ui.rect("error.scrim").size(screen.width, screen.height)
                .color({0.0f, 0.0f, 0.0f, 0.68f}).build();
            ui.stack("error.panel")
                .size(440.0f, 190.0f)
                .content([&] {
                    ui.rect("error.panel.background")
                        .size(440.0f, 190.0f).color(kSurface).radius(8.0f)
                        .border(1.0f, kRed).build();
                    ui.column("error.content")
                        .size(440.0f, 190.0f).padding(20.0f).gap(12.0f)
                        .content([&] {
                            text(ui, "error.title", "Action failed", 400.0f, 28.0f, 20.0f, kRedHover);
                            ui.text("error.message").size(400.0f, 62.0f)
                                .text(view.errorMessage).fontSize(14.0f).lineHeight(19.0f)
                                .wrap(true).color(kText).build();
                            components::button(ui, "error.dismiss")
                                .size(150.0f, 40.0f).text("Dismiss").radius(6.0f)
                                .colors(kRed, kRedHover, kRedPressed)
                                .onClick([] { controller.dismissError(); }).build();
                        }).build();
                }).build();
        }).build();
}

} // namespace

const DslAppConfig& dslAppConfig() {
    static const DslAppConfig config = DslAppConfig{}
        .title("Bot or Fold")
        .pageId("holdem_gui")
        .clearColor(kBackground)
        .windowSize(1100, 720)
        .fps(90.0)
        .showDebugStatsInTitle(false);
    return config;
}

void compose(eui::Ui& ui, const eui::Screen& screen) {
    const ControllerView view = controller.view();
    if (view.hasGame) composeGame(ui, screen, view);
    else composeSetup(ui, screen);
    composeError(ui, screen, view);
}

} // namespace app
