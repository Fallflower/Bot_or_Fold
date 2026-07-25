#include "eui_neo.h"
#include "gameController.h"
#include "position.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>
#include <string>
#include <vector>

namespace app {
namespace {

constexpr int kSetupWidth = 640;
constexpr int kSetupHeight = 820;
constexpr int kTableWidth = 1920;
constexpr int kTableHeight = 1080;
constexpr float kPi = 3.14159265358979323846f;

constexpr eui::Color kBackground{0.055f, 0.070f, 0.065f, 1.0f};
constexpr eui::Color kSurface{0.095f, 0.110f, 0.105f, 1.0f};
constexpr eui::Color kSurfaceRaised{0.135f, 0.150f, 0.142f, 1.0f};
constexpr eui::Color kBorder{0.25f, 0.29f, 0.27f, 1.0f};
constexpr eui::Color kText{0.94f, 0.96f, 0.95f, 1.0f};
constexpr eui::Color kMuted{0.62f, 0.68f, 0.65f, 1.0f};
constexpr eui::Color kGreen{0.12f, 0.55f, 0.34f, 1.0f};
constexpr eui::Color kGreenHover{0.15f, 0.64f, 0.40f, 1.0f};
constexpr eui::Color kGreenPressed{0.09f, 0.45f, 0.28f, 1.0f};
constexpr eui::Color kFelt{0.035f, 0.30f, 0.17f, 1.0f};
constexpr eui::Color kFeltEdge{0.10f, 0.48f, 0.27f, 1.0f};
constexpr eui::Color kRed{0.66f, 0.20f, 0.22f, 1.0f};
constexpr eui::Color kRedHover{0.76f, 0.25f, 0.27f, 1.0f};
constexpr eui::Color kRedPressed{0.55f, 0.15f, 0.17f, 1.0f};
constexpr eui::Color kGold{0.82f, 0.60f, 0.17f, 1.0f};
constexpr eui::Color kGoldHover{0.91f, 0.68f, 0.22f, 1.0f};
constexpr eui::Color kGoldPressed{0.70f, 0.49f, 0.12f, 1.0f};

// ============================================================================
// 牌桌布局手动调节区
// ============================================================================
// 比例值均基于 EUI 的逻辑尺寸（Screen 或父容器），不是显示器物理像素。
// 调整时建议一次只修改一组：牌桌外框 -> 玩家框 -> 手牌 -> 公共牌。
namespace table_layout {

// 整个牌桌页面占窗口的比例；剩余空间作为四周留白。
constexpr float kShellWidthRatio = 0.96f;
constexpr float kShellHeightRatio = 0.96f;
constexpr float kShellMinWidth = 640.0f;
constexpr float kShellMinHeight = 360.0f;
constexpr float kShellPadding = 14.0f;
constexpr float kHeaderHeight = 44.0f;
constexpr float kSectionGap = 8.0f;
constexpr float kFooterHeight = 48.0f;

// 绿色椭圆桌面相对于可用牌桌区域的尺寸和起点。
constexpr float kTableXRatio = 0.115f;
constexpr float kTableYRatio = 0.115f;
constexpr float kTableWidthRatio = 0.77f;
constexpr float kTableHeightRatio = 0.69f;

// 玩家信息框尺寸。
// 8/9 人桌使用 crowded 宽度，人数较少时使用 normal 宽度。
constexpr int kCrowdedPlayerCount = 8;
constexpr float kCrowdedSeatWidthRatio = 0.145f;
constexpr float kNormalSeatWidthRatio = 0.19f;
constexpr float kSeatMinWidth = 150.0f;
constexpr float kSeatMaxWidth = 300.0f;
// 增大该值会增加整个玩家信息框的高度。
constexpr float kSeatHeightToWidth = 0.56f;

// 玩家围绕桌面的椭圆半径。数值越大，玩家框越靠近窗口边缘。
constexpr float kSeatRadiusXRatio = 0.39f;
constexpr float kSeatRadiusYRatio = 0.34f;
constexpr float kSeatHorizontalEdgeReserve = 0.58f;
constexpr float kSeatVerticalEdgeReserve = 0.60f;
constexpr float kSeatCenterYRatio = 0.47f;

// 玩家手牌尺寸与位置。
// 手牌首先尝试占满玩家框高度，再受最大宽度限制，避免挤入右侧文字区。
constexpr float kCardHeightToWidth = 1.406f;
constexpr float kHoleCardHeightRatio = 0.88f;
constexpr float kHoleCardMaxWidthRatio = 0.34f;
constexpr float kHoleCardXRatio = 0.025f;
constexpr float kHoleCardYRatio = 0.06f;
// 第二张牌相对第一张牌的步进；小于 1.0 会让两张牌轻微重叠。
constexpr float kHoleCardStepRatio = 0.64f;

// 玩家文字信息区的位置、宽度和字号。
// 左移 kPlayerInfoXRatio 或增大 kPlayerInfoWidthRatio 会扩大右侧文字区。
constexpr float kPlayerInfoXRatio = 0.62f;
constexpr float kPlayerInfoWidthRatio = 0.35f;
constexpr float kPlayerNameYRatio = 0.035f;
constexpr float kPlayerNameHeightRatio = 0.15f;
constexpr float kPlayerPositionYRatio = 0.205f;
constexpr float kPlayerPositionHeightRatio = 0.13f;
constexpr float kPlayerStackYRatio = 0.345f;
constexpr float kPlayerStackHeightRatio = 0.13f;
constexpr float kPlayerCommittedYRatio = 0.485f;
constexpr float kPlayerCommittedHeightRatio = 0.13f;
constexpr float kPlayerActionYRatio = 0.68f;
constexpr float kPlayerActionHeightRatio = 0.25f;
constexpr float kPlayerNameFontRatio = 0.068f;
constexpr float kPlayerDetailFontRatio = 0.058f;
constexpr float kPlayerNameFontMin = 14.0f;
constexpr float kPlayerNameFontMax = 22.0f;
constexpr float kPlayerDetailFontMin = 13.0f;
constexpr float kPlayerDetailFontMax = 18.0f;
constexpr float kPlayerFontLineHeightExtra = 3.0f;

// Dealer 圆形标记尺寸及其相对玩家框右上角的偏移。
constexpr float kDealerSizeRatio = 0.105f;
constexpr float kDealerRightOffsetRatio = 0.85f;
constexpr float kDealerTopOffsetRatio = 0.25f;

// 公共牌尺寸。宽度同时受牌桌宽度和高度约束，取两者中较小值。
constexpr float kCommunityCardWidthRatio = 0.066f;
constexpr float kCommunityCardHeightLimitRatio = 0.20f;
constexpr float kCommunityCardGapRatio = 0.14f;
// 数值越大，公共牌整体越靠上。
constexpr float kCommunityVerticalOffsetRatio = 0.68f;

// 底池徽章相对于一张公共牌的大小。
constexpr float kPotBadgeWidthRatio = 2.05f;
constexpr float kPotBadgeHeightRatio = 0.30f;
constexpr float kPotBadgeGapRatio = 0.10f;

} // namespace table_layout

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
    bool resultOpen = false;
};

GameController controller;
UiState state;

void resizeMainWindow(int width, int height) {
    if (GLFWwindow* window = glfwGetCurrentContext())
        glfwSetWindowSize(window, width, height);
}

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

std::string compactPosition(std::string value) {
    value.erase(std::remove_if(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }), value.end());
    return value;
}

std::string selectedPositionName() {
    const Position positions(state.playerCount, state.playerCount - 1);
    return compactPosition(positions[state.humanPlayerIndex]);
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

std::string cardSource(const CardSnapshot& card) {
    if (!card.visible || card.rank < 0 || card.suit < 0)
        return "assets/pokers/back.png";
    return "assets/pokers/" + std::to_string(card.rank * 4 + card.suit) + ".png";
}

void composeCard(eui::Ui& ui, const std::string& id, const CardSnapshot* card,
                 float x, float y, float width, float height, bool faceDownWhenMissing = false) {
    if (!card && !faceDownWhenMissing) {
        ui.rect(id + ".empty")
            .position(x, y).size(width, height)
            .color({0.02f, 0.16f, 0.09f, 0.54f})
            .radius(5.0f).border(1.0f, {0.42f, 0.62f, 0.50f, 0.65f})
            .build();
        return;
    }

    ui.image(id)
        .position(x, y).size(width, height)
        .source(card ? cardSource(*card) : "assets/pokers/back.png")
        .contain().radius(5.0f)
        .build();
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
        .size(width, 46.0f)
        .gap(14.0f)
        .alignItems(eui::Align::CENTER)
        .content([&] {
            text(ui, id + ".label", label, labelWidth, 42.0f, 15.0f, kMuted);
            composeControl(controlWidth);
        })
        .build();
}

void composePositionSelector(eui::Ui& ui, float width) {
    const float buttonWidth = 52.0f;
    const float labelWidth = width - buttonWidth * 2.0f - 16.0f;
    ui.row("setup.position.selector")
        .size(width, 42.0f).gap(8.0f).alignItems(eui::Align::CENTER)
        .content([&] {
            components::button(ui, "setup.position.previous")
                .size(buttonWidth, 40.0f).text("<").radius(6.0f)
                .colors(kSurfaceRaised, kBorder, kSurface)
                .onClick([] {
                    state.humanPlayerIndex = (state.humanPlayerIndex + state.playerCount - 1)
                        % state.playerCount;
                }).build();
            ui.stack("setup.position.name")
                .size(labelWidth, 40.0f).content([&] {
                    ui.rect("setup.position.name.bg")
                        .size(labelWidth, 40.0f).color(kBackground)
                        .radius(6.0f).border(1.0f, kBorder).build();
                    text(ui, "setup.position.name.text", selectedPositionName(),
                         labelWidth, 40.0f, 16.0f, kText, eui::HorizontalAlign::Center);
                }).build();
            components::button(ui, "setup.position.next")
                .size(buttonWidth, 40.0f).text(">").radius(6.0f)
                .colors(kSurfaceRaised, kBorder, kSurface)
                .onClick([] {
                    state.humanPlayerIndex = (state.humanPlayerIndex + 1) % state.playerCount;
                }).build();
        }).build();
}

void composeSetup(eui::Ui& ui, const eui::Screen& screen) {
    const float panelWidth = std::min(590.0f, std::max(430.0f, screen.width - 32.0f));
    const float panelHeight = std::min(750.0f, std::max(650.0f, screen.height - 32.0f));
    const float innerWidth = panelWidth - 48.0f;
    const float panelX = std::max(0.0f, (screen.width - panelWidth) * 0.5f);
    const float panelY = std::max(0.0f, (screen.height - panelHeight) * 0.5f);

    ui.stack("setup.root")
        .size(screen.width, screen.height)
        .content([&] {
            ui.rect("setup.background").size(screen.width, screen.height).color(kBackground).build();
            ui.stack("setup.panel")
                .position(panelX, panelY).size(panelWidth, panelHeight)
                .content([&] {
                    ui.rect("setup.panel.background")
                        .size(panelWidth, panelHeight)
                        .color(kSurface).radius(10.0f).border(1.0f, kBorder)
                        .shadow(28.0f, 0.0f, 10.0f, {0.0f, 0.0f, 0.0f, 0.34f})
                        .build();
                    ui.column("setup.form")
                        .size(panelWidth, panelHeight)
                        .padding(24.0f).gap(14.0f)
                        .content([&] {
                            text(ui, "setup.title", "Bot or Fold", innerWidth, 44.0f, 30.0f);
                            text(ui, "setup.subtitle", "Create a new poker table", innerWidth,
                                 26.0f, 14.0f, kMuted);

                            setupRow(ui, "setup.name", "Player name", innerWidth, [&](float width) {
                                components::input(ui, "setup.name.input")
                                    .size(width, 42.0f).value(state.playerName).placeholder("Player")
                                    .onChange([](const std::string& value) { state.playerName = value; })
                                    .build();
                            });
                            setupRow(ui, "setup.mode", "Game mode", innerWidth, [&](float width) {
                                components::segmented(ui, "setup.mode.segmented")
                                    .size(width, 40.0f).items({"Standard", "Short Deck"})
                                    .selected(state.mode)
                                    .onChange([](int value) { state.mode = value; }).build();
                            });
                            setupRow(ui, "setup.players", "Players", innerWidth, [&](float width) {
                                components::stepper(ui, "setup.players.stepper")
                                    .size(width, 40.0f).value(state.playerCount).min(2).max(9)
                                    .onChange([](long long value) {
                                        state.playerCount = static_cast<int>(value);
                                        state.humanPlayerIndex = std::min(
                                            state.humanPlayerIndex, state.playerCount - 1);
                                    }).build();
                            });
                            setupRow(ui, "setup.chips", "Starting chips", innerWidth, [&](float width) {
                                components::stepper(ui, "setup.chips.stepper")
                                    .size(width, 40.0f).value(state.initialChips)
                                    .step(50).min(50).max(5000)
                                    .onChange([](long long value) {
                                        state.initialChips = static_cast<int>(value);
                                    }).build();
                            });
                            setupRow(ui, "setup.position", "Your position", innerWidth,
                                     [&](float width) { composePositionSelector(ui, width); });

                            ui.rect("setup.divider").size(innerWidth, 1.0f).color(kBorder).build();
                            text(ui, "setup.hint",
                                 "Your position is shown by its poker name (SB, BB, UTG, CO, D).",
                                 innerWidth, 34.0f, 13.0f, kMuted);
                            components::button(ui, "setup.start")
                                .size(innerWidth, 50.0f).text("Start game").radius(7.0f)
                                .colors(kGreen, kGreenHover, kGreenPressed)
                                .onClick([] {
                                    GameConfig config;
                                    config.playerName = state.playerName;
                                    config.playerCount = state.playerCount;
                                    config.initialChips = state.initialChips;
                                    config.humanPlayerIndex = state.humanPlayerIndex;
                                    config.mode = state.mode == 0
                                        ? GameMode::Standard : GameMode::ShortDeck;
                                    if (controller.startGame(config)) {
                                        state.decisionPlayer = -1;
                                        state.resultOpen = false;
                                        resizeMainWindow(kTableWidth, kTableHeight);
                                    }
                                }).build();
                        }).build();
                }).build();
        }).build();
}

void composeSeat(eui::Ui& ui, const PlayerSnapshot& player, bool dealer,
                 float x, float y, float width, float height) {
    const std::string id = "game.seat." + std::to_string(player.index);
    eui::Color background = player.human
        ? eui::Color{0.11f, 0.18f, 0.15f, 0.98f}
        : eui::Color{0.075f, 0.105f, 0.092f, 0.96f};
    if (player.folded) background = {0.10f, 0.10f, 0.10f, 0.82f};

    ui.stack(id)
        .position(x, y).size(width, height)
        .opacity(player.folded ? 0.74f : 1.0f)
        .content([&] {
            ui.rect(id + ".bg")
                .size(width, height).color(background).radius(9.0f)
                .border(player.active ? 2.0f : 1.0f, player.active ? kGold : kBorder)
                .shadow(player.active ? 18.0f : 8.0f, 0.0f, 4.0f,
                        player.active ? eui::Color{0.85f, 0.62f, 0.14f, 0.34f}
                                      : eui::Color{0.0f, 0.0f, 0.0f, 0.24f})
                .build();

            const float cardWidth = std::min(
                height * table_layout::kHoleCardHeightRatio
                    / table_layout::kCardHeightToWidth,
                width * table_layout::kHoleCardMaxWidthRatio);
            const float cardHeight = cardWidth * table_layout::kCardHeightToWidth;
            const float cardX = width * table_layout::kHoleCardXRatio;
            const float cardY = height * table_layout::kHoleCardYRatio;
            const float cardStep = cardWidth * table_layout::kHoleCardStepRatio;
            const CardSnapshot* first = player.cards.empty() ? nullptr : &player.cards[0];
            const CardSnapshot* second = player.cards.size() < 2 ? nullptr : &player.cards[1];
            composeCard(ui, id + ".card.0", first, cardX, cardY,
                        cardWidth, cardHeight, true);
            composeCard(ui, id + ".card.1", second, cardX + cardStep, cardY,
                        cardWidth, cardHeight, true);

            const float infoX = width * table_layout::kPlayerInfoXRatio;
            const float infoWidth = width * table_layout::kPlayerInfoWidthRatio;
            const float nameFont = std::clamp(
                width * table_layout::kPlayerNameFontRatio,
                table_layout::kPlayerNameFontMin,
                table_layout::kPlayerNameFontMax);
            const float detailFont = std::clamp(
                width * table_layout::kPlayerDetailFontRatio,
                table_layout::kPlayerDetailFontMin,
                table_layout::kPlayerDetailFontMax);
            ui.text(id + ".name").position(
                    infoX, height * table_layout::kPlayerNameYRatio)
                .size(infoWidth, height * table_layout::kPlayerNameHeightRatio)
                .text(player.name + (player.human ? " (You)" : ""))
                .fontSize(nameFont)
                .lineHeight(nameFont + table_layout::kPlayerFontLineHeightExtra)
                .color(kText)
                .verticalAlign(eui::VerticalAlign::Center).build();
            ui.text(id + ".position").position(
                    infoX, height * table_layout::kPlayerPositionYRatio)
                .size(infoWidth, height * table_layout::kPlayerPositionHeightRatio)
                .text("Pos  " + compactPosition(player.position))
                .fontSize(detailFont)
                .lineHeight(detailFont + table_layout::kPlayerFontLineHeightExtra)
                .color(kMuted)
                .verticalAlign(eui::VerticalAlign::Center).build();
            ui.text(id + ".stack").position(
                    infoX, height * table_layout::kPlayerStackYRatio)
                .size(infoWidth, height * table_layout::kPlayerStackHeightRatio)
                .text("Stack  " + std::to_string(player.chips))
                .fontSize(detailFont)
                .lineHeight(detailFont + table_layout::kPlayerFontLineHeightExtra)
                .color(kMuted)
                .verticalAlign(eui::VerticalAlign::Center).build();
            ui.text(id + ".committed").position(
                    infoX, height * table_layout::kPlayerCommittedYRatio)
                .size(infoWidth, height * table_layout::kPlayerCommittedHeightRatio)
                .text("Committed " + std::to_string(player.committed))
                .fontSize(detailFont)
                .lineHeight(detailFont + table_layout::kPlayerFontLineHeightExtra)
                .color(kGold)
                .verticalAlign(eui::VerticalAlign::Center).build();
            ui.rect(id + ".action.bg").position(
                    infoX, height * table_layout::kPlayerActionYRatio)
                .size(infoWidth, height * table_layout::kPlayerActionHeightRatio)
                .color({0.035f, 0.065f, 0.052f, 0.88f}).radius(6.0f).build();
            ui.text(id + ".action").position(
                    infoX, height * table_layout::kPlayerActionYRatio)
                .size(infoWidth, height * table_layout::kPlayerActionHeightRatio)
                .text(actionLabel(player)).fontSize(detailFont)
                .lineHeight(detailFont + table_layout::kPlayerFontLineHeightExtra)
                .color(player.folded ? kRedHover : kText)
                .horizontalAlign(eui::HorizontalAlign::Center)
                .verticalAlign(eui::VerticalAlign::Center).build();

            if (dealer) {
                const float dealerSize = width * table_layout::kDealerSizeRatio;
                ui.rect(id + ".dealer.bg").position(
                        width - dealerSize * table_layout::kDealerRightOffsetRatio,
                        -dealerSize * table_layout::kDealerTopOffsetRatio)
                    .size(dealerSize, dealerSize).color(kGold).radius(dealerSize * 0.5f)
                    .border(1.0f, kText).build();
                ui.text(id + ".dealer.text").position(
                        width - dealerSize * table_layout::kDealerRightOffsetRatio,
                        -dealerSize * table_layout::kDealerTopOffsetRatio)
                    .size(dealerSize, dealerSize).text("D")
                    .fontSize(dealerSize * 0.50f).lineHeight(dealerSize * 0.62f)
                    .color(kBackground).horizontalAlign(eui::HorizontalAlign::Center)
                    .verticalAlign(eui::VerticalAlign::Center).build();
            }
        }).build();
}

void composeCommunity(eui::Ui& ui, const TableSnapshot& table,
                      float stageWidth, float stageHeight) {
    const float cardWidth = std::min(
        stageWidth * table_layout::kCommunityCardWidthRatio,
        stageHeight * table_layout::kCommunityCardHeightLimitRatio);
    const float cardHeight = cardWidth * table_layout::kCardHeightToWidth;
    const float cardGap = cardWidth * table_layout::kCommunityCardGapRatio;
    const float cardsWidth = cardWidth * 5.0f + cardGap * 4.0f;
    const float cardsX = (stageWidth - cardsWidth) * 0.5f;
    const float cardsY = stageHeight * 0.5f
        - cardHeight * table_layout::kCommunityVerticalOffsetRatio;

    for (int i = 0; i < 5; ++i) {
        const CardSnapshot* card = i < static_cast<int>(table.publicCards.size())
            && table.publicCards[static_cast<size_t>(i)].visible
            ? &table.publicCards[static_cast<size_t>(i)] : nullptr;
        composeCard(ui, "game.board.card." + std::to_string(i), card,
                    cardsX + static_cast<float>(i) * (cardWidth + cardGap), cardsY,
                    cardWidth, cardHeight);
    }

    const std::string potText = table.sidePots.size() > 1
        ? "Total pot  " + std::to_string(table.pot) + "  |  "
            + std::to_string(table.sidePots.size()) + " pots"
        : "Pot  " + std::to_string(table.pot);
    const float badgeWidth = cardWidth * table_layout::kPotBadgeWidthRatio;
    const float badgeHeight = cardHeight * table_layout::kPotBadgeHeightRatio;
    ui.stack("game.pot.badge")
        .position(stageWidth * 0.5f - badgeWidth * 0.5f,
                  cardsY + cardHeight + cardHeight * table_layout::kPotBadgeGapRatio)
        .size(badgeWidth, badgeHeight)
        .content([&] {
            ui.rect("game.pot.badge.bg").size(badgeWidth, badgeHeight)
                .color({0.025f, 0.11f, 0.065f, 0.92f}).radius(badgeHeight * 0.5f)
                .border(1.0f, kGold).build();
            text(ui, "game.pot.badge.text", potText, badgeWidth, badgeHeight,
                 std::clamp(cardWidth * 0.19f, 13.0f, 19.0f),
                 kGold, eui::HorizontalAlign::Center);
        }).build();
}

void composeTableStage(eui::Ui& ui, const ControllerView& view,
                       float x, float y, float width, float height) {
    ui.stack("game.table.stage")
        .position(x, y).size(width, height)
        .content([&] {
            const float tableX = width * table_layout::kTableXRatio;
            const float tableY = height * table_layout::kTableYRatio;
            const float tableWidth = width * table_layout::kTableWidthRatio;
            const float tableHeight = height * table_layout::kTableHeightRatio;
            ui.rect("game.table.shadow")
                .position(tableX - 10.0f, tableY + 8.0f)
                .size(tableWidth + 20.0f, tableHeight + 18.0f)
                .color({0.025f, 0.055f, 0.038f, 1.0f})
                .radius(tableHeight * 0.5f).build();
            ui.rect("game.table.rail")
                .position(tableX - 6.0f, tableY - 6.0f)
                .size(tableWidth + 12.0f, tableHeight + 12.0f)
                .color(kFeltEdge).radius(tableHeight * 0.5f)
                .border(6.0f, {0.22f, 0.13f, 0.065f, 1.0f}).build();
            ui.rect("game.table.felt")
                .position(tableX, tableY).size(tableWidth, tableHeight)
                .color(kFelt).radius(tableHeight * 0.5f)
                .border(2.0f, {0.20f, 0.64f, 0.37f, 0.58f}).build();

            composeCommunity(ui, view.table, width, height);

            const int count = static_cast<int>(view.table.players.size());
            const float seatWidth = std::clamp(
                width * (count >= table_layout::kCrowdedPlayerCount
                    ? table_layout::kCrowdedSeatWidthRatio
                    : table_layout::kNormalSeatWidthRatio),
                table_layout::kSeatMinWidth,
                table_layout::kSeatMaxWidth);
            const float seatHeight = seatWidth * table_layout::kSeatHeightToWidth;
            const float radiusX = std::min(
                width * table_layout::kSeatRadiusXRatio,
                width * 0.5f - seatWidth * table_layout::kSeatHorizontalEdgeReserve);
            const float radiusY = std::min(
                height * table_layout::kSeatRadiusYRatio,
                height * 0.5f - seatHeight * table_layout::kSeatVerticalEdgeReserve);
            const float centerX = width * 0.5f;
            const float centerY = height * table_layout::kSeatCenterYRatio;
            for (const PlayerSnapshot& player : view.table.players) {
                const int relative = (player.index - view.table.humanPlayerIndex + count) % count;
                const float angle = kPi * 0.5f
                    + 2.0f * kPi * static_cast<float>(relative) / static_cast<float>(count);
                const float seatX = std::clamp(
                    centerX + radiusX * std::cos(angle) - seatWidth * 0.5f,
                    2.0f, width - seatWidth - 2.0f);
                const float seatY = std::clamp(
                    centerY + radiusY * std::sin(angle) - seatHeight * 0.5f,
                    2.0f, height - seatHeight - 2.0f);
                composeSeat(ui, player, player.index == view.table.dealerIndex,
                            seatX, seatY, seatWidth, seatHeight);
            }
        }).build();
}

void humanActions(eui::Ui& ui, const ControllerView& view, float width) {
    const DecisionRequest& decision = view.table.decision;
    const bool canFold = hasAction(decision, FOLD);
    const bool canCall = hasAction(decision, CALL);
    const bool canRaise = hasAction(decision, RAISE);
    const float raiseControlWidth = std::max(180.0f, width - 570.0f);

    ui.row("game.actions")
        .size(width, 48.0f).gap(10.0f).alignItems(eui::Align::CENTER)
        .content([&] {
            text(ui, "game.action.prompt", "Your action", 100.0f, 44.0f, 14.0f, kGold);
            components::button(ui, "game.action.fold")
                .size(100.0f, 44.0f).text("Fold").radius(6.0f)
                .colors(kRed, kRedHover, kRedPressed).disabled(!canFold)
                .onClick([] { controller.submitHumanAction({FOLD, 0}); }).build();
            components::button(ui, "game.action.call")
                .size(145.0f, 44.0f)
                .text(decision.chipsToCall == 0
                    ? "Check" : "Call " + std::to_string(decision.chipsToCall))
                .radius(6.0f).colors(kGreen, kGreenHover, kGreenPressed)
                .disabled(!canCall)
                .onClick([] { controller.submitHumanAction({CALL, 0}); }).build();
            components::stepper(ui, "game.action.raise.amount")
                .size(raiseControlWidth, 42.0f).value(state.raiseAmount)
                .min(canRaise ? decision.minRaise : 0)
                .max(canRaise ? decision.maxRaise : 0)
                .onChange([](long long value) { state.raiseAmount = static_cast<int>(value); })
                .build();
            components::button(ui, "game.action.raise")
                .size(145.0f, 44.0f)
                .text(state.raiseAmount == decision.maxRaise ? "All-in" : "Raise")
                .radius(6.0f).colors(kGold, kGoldHover, kGoldPressed)
                .disabled(!canRaise)
                .onClick([] { controller.submitHumanAction({RAISE, state.raiseAmount}); }).build();
        }).build();
}

void returnToSetup() {
    controller.returnToSetup();
    state.resultOpen = false;
    state.botTaskPending = false;
    resizeMainWindow(kSetupWidth, kSetupHeight);
}

void nextRound() {
    if (controller.nextRound()) {
        state.resultOpen = false;
        state.decisionPlayer = -1;
    }
}

void roundActions(eui::Ui& ui, const ControllerView& view, float width) {
    ui.row("game.round.actions")
        .size(width, 48.0f).gap(10.0f).alignItems(eui::Align::CENTER)
        .content([&] {
            text(ui, "game.round.done", view.table.roundSettled ? "Result ready" : "Round complete",
                 130.0f, 44.0f, 14.0f, view.table.roundSettled ? kGreenHover : kGold);
            if (!view.table.roundSettled) {
                components::button(ui, "game.round.settle")
                    .size(190.0f, 44.0f).text("Show result").radius(6.0f)
                    .colors(kGold, kGoldHover, kGoldPressed)
                    .onClick([] {
                        if (controller.settleRound()) state.resultOpen = true;
                    }).build();
            } else {
                components::button(ui, "game.round.review")
                    .size(180.0f, 44.0f).text("Review result").radius(6.0f)
                    .colors(kGold, kGoldHover, kGoldPressed)
                    .onClick([] { state.resultOpen = true; }).build();
                components::button(ui, "game.round.next")
                    .size(180.0f, 44.0f).text("Next round").radius(6.0f)
                    .colors(kGreen, kGreenHover, kGreenPressed)
                    .onClick(nextRound).build();
            }
            components::button(ui, "game.round.setup")
                .size(170.0f, 44.0f).text("Table setup").radius(6.0f)
                .theme(components::theme::dark(), false)
                .onClick(returnToSetup).build();
        }).build();
}

std::string awardsText(const ControllerView& view, const PotResult& pot) {
    std::string result;
    for (const PlayerAward& award : pot.awards) {
        if (!result.empty()) result += "   ";
        if (award.playerIndex >= 0
            && award.playerIndex < static_cast<int>(view.table.players.size())) {
            result += view.table.players[static_cast<size_t>(award.playerIndex)].name;
        } else {
            result += "Player " + std::to_string(award.playerIndex + 1);
        }
        result += "  +" + std::to_string(award.amount);
    }
    return result.empty() ? "No award" : result;
}

void composeResult(eui::Ui& ui, const eui::Screen& screen, const ControllerView& view) {
    if (!state.resultOpen || !view.roundResult.settled) return;

    const float panelWidth = 590.0f;
    const float potRows = static_cast<float>(std::max<size_t>(1, view.roundResult.pots.size()));
    const float panelHeight = std::min(610.0f, 330.0f + potRows * 46.0f);
    const float innerWidth = panelWidth - 48.0f;
    const float panelX = std::max(0.0f, (screen.width - panelWidth) * 0.5f);
    const float panelY = std::max(0.0f, (screen.height - panelHeight) * 0.5f);

    ui.stack("result.overlay")
        .size(screen.width, screen.height)
        .zIndex(900)
        .content([&] {
            ui.rect("result.scrim").size(screen.width, screen.height)
                .color({0.0f, 0.0f, 0.0f, 0.76f}).interactive().build();
            ui.stack("result.panel")
                .position(panelX, panelY).size(panelWidth, panelHeight)
                .content([&] {
                    ui.rect("result.panel.bg").size(panelWidth, panelHeight)
                        .color(kSurface).radius(12.0f).border(1.0f, kGold)
                        .shadow(32.0f, 0.0f, 10.0f, {0.0f, 0.0f, 0.0f, 0.50f}).build();
                    ui.column("result.content")
                        .size(panelWidth, panelHeight).padding(24.0f).gap(10.0f)
                        .content([&] {
                            text(ui, "result.title", "Round result", innerWidth, 38.0f, 26.0f, kGold);
                            text(ui, "result.subtitle", "Pots have been awarded", innerWidth,
                                 24.0f, 14.0f, kMuted);
                            ui.rect("result.divider.top").size(innerWidth, 1.0f).color(kBorder).build();

                            for (size_t i = 0; i < view.roundResult.pots.size(); ++i) {
                                const PotResult& pot = view.roundResult.pots[i];
                                const std::string id = "result.pot." + std::to_string(i);
                                const std::string potName = i == 0
                                    ? "Main pot  " : "Side pot " + std::to_string(i) + "  ";
                                ui.stack(id).size(innerWidth, 40.0f).content([&] {
                                    ui.rect(id + ".bg").size(innerWidth, 40.0f)
                                        .color(kSurfaceRaised).radius(6.0f).build();
                                    ui.text(id + ".label").position(10.0f, 0.0f)
                                        .size(150.0f, 40.0f)
                                        .text(potName + std::to_string(pot.amount))
                                        .fontSize(14.0f).lineHeight(18.0f).color(kMuted)
                                        .verticalAlign(eui::VerticalAlign::Center).build();
                                    ui.text(id + ".awards").position(160.0f, 0.0f)
                                        .size(innerWidth - 170.0f, 40.0f)
                                        .text(awardsText(view, pot)).fontSize(15.0f).lineHeight(19.0f)
                                        .color(kText).horizontalAlign(eui::HorizontalAlign::Right)
                                        .verticalAlign(eui::VerticalAlign::Center).build();
                                }).build();
                            }

                            std::string showdown;
                            for (const PlayerSnapshot& player : view.table.players) {
                                if (player.handDescription.empty()) continue;
                                if (!showdown.empty()) showdown += "    |    ";
                                showdown += player.name + ": " + player.handDescription;
                            }
                            ui.text("result.hands").size(innerWidth, 64.0f)
                                .text(showdown.empty() ? "Round ended without a showdown." : showdown)
                                .fontSize(13.0f).lineHeight(18.0f).wrap(true).color(kMuted).build();
                            if (view.roundResult.humanToppedUp)
                                text(ui, "result.topup", "Your stack was topped up for the next round.",
                                     innerWidth, 24.0f, 13.0f, kGreenHover);
                            ui.rect("result.divider.bottom").size(innerWidth, 1.0f).color(kBorder).build();
                            ui.row("result.actions")
                                .size(innerWidth, 44.0f).gap(10.0f)
                                .content([&] {
                                    components::button(ui, "result.next")
                                        .size(180.0f, 42.0f).text("Next round").radius(6.0f)
                                        .colors(kGreen, kGreenHover, kGreenPressed)
                                        .onClick(nextRound).build();
                                    components::button(ui, "result.setup")
                                        .size(170.0f, 42.0f).text("Table setup").radius(6.0f)
                                        .theme(components::theme::dark(), false)
                                        .onClick(returnToSetup).build();
                                    components::button(ui, "result.close")
                                        .size(170.0f, 42.0f).text("Close").radius(6.0f)
                                        .theme(components::theme::dark(), false)
                                        .onClick([] { state.resultOpen = false; }).build();
                                }).build();
                        }).build();
                }).build();
        }).build();
}

void composeGame(eui::Ui& ui, const eui::Screen& screen, const ControllerView& view) {
    syncRaiseAmount(view);
    const float shellWidth = std::max(
        table_layout::kShellMinWidth,
        screen.width * table_layout::kShellWidthRatio);
    const float shellHeight = std::max(
        table_layout::kShellMinHeight,
        screen.height * table_layout::kShellHeightRatio);
    const float innerWidth = shellWidth - table_layout::kShellPadding * 2.0f;
    const float stageY = table_layout::kShellPadding
        + table_layout::kHeaderHeight + table_layout::kSectionGap;
    const float stageHeight = shellHeight
        - table_layout::kShellPadding * 2.0f
        - table_layout::kHeaderHeight
        - table_layout::kFooterHeight
        - table_layout::kSectionGap * 2.0f;
    const float shellX = std::max(0.0f, (screen.width - shellWidth) * 0.5f);
    const float shellY = std::max(0.0f, (screen.height - shellHeight) * 0.5f);
    const float footerY = stageY + stageHeight + table_layout::kSectionGap;

    ui.stack("game.root")
        .size(screen.width, screen.height)
        .content([&] {
            ui.rect("game.background").size(screen.width, screen.height)
                .color({0.025f, 0.070f, 0.045f, 1.0f}).build();
            ui.stack("game.shell")
                .position(shellX, shellY).size(shellWidth, shellHeight)
                .content([&] {
                    ui.row("game.header")
                        .position(table_layout::kShellPadding, table_layout::kShellPadding)
                        .size(innerWidth, table_layout::kHeaderHeight)
                        .alignItems(eui::Align::CENTER)
                        .content([&] {
                            text(ui, "game.title", "Bot or Fold", innerWidth * 0.32f,
                                 44.0f, 24.0f);
                            const std::string status = view.state == ControllerState::BotThinking
                                ? "Bot is thinking..." : view.state == ControllerState::WaitingForHuman
                                ? "Your turn" : view.table.roundSettled ? "Result ready" : "Round complete";
                            text(ui, "game.street", view.table.state + "  -  " + status,
                                 innerWidth * 0.36f, 44.0f, 14.0f,
                                 view.state == ControllerState::BotThinking ? kGold : kMuted,
                                 eui::HorizontalAlign::Center);
                            text(ui, "game.mode",
                                 view.config.mode == GameMode::Standard ? "STANDARD" : "SHORT DECK",
                                 innerWidth * 0.32f, 44.0f, 13.0f, kMuted,
                                 eui::HorizontalAlign::Right);
                        }).build();

                    composeTableStage(ui, view, table_layout::kShellPadding,
                                      stageY, innerWidth, stageHeight);

                    ui.stack("game.footer")
                        .position(table_layout::kShellPadding, footerY)
                        .size(innerWidth, table_layout::kFooterHeight)
                        .content([&] {
                            if (view.state == ControllerState::WaitingForHuman)
                                humanActions(ui, view, innerWidth);
                            else if (view.state == ControllerState::RoundFinished)
                                roundActions(ui, view, innerWidth);
                            else
                                text(ui, "game.bot.wait", "Calculating the next action...", innerWidth,
                                     48.0f, 15.0f, kMuted, eui::HorizontalAlign::Center);
                        }).build();
                }).build();
        }).build();

    composeResult(ui, screen, view);
    scheduleBot(view);
}

void composeError(eui::Ui& ui, const eui::Screen& screen, const ControllerView& view) {
    if (view.state != ControllerState::Error) return;
    const float panelX = std::max(0.0f, (screen.width - 440.0f) * 0.5f);
    const float panelY = std::max(0.0f, (screen.height - 190.0f) * 0.5f);
    ui.stack("error.overlay")
        .size(screen.width, screen.height)
        .zIndex(1000)
        .content([&] {
            ui.rect("error.scrim").size(screen.width, screen.height)
                .color({0.0f, 0.0f, 0.0f, 0.68f}).interactive().build();
            ui.stack("error.panel")
                .position(panelX, panelY).size(440.0f, 190.0f)
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
        .windowSize(kSetupWidth, kSetupHeight)
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
