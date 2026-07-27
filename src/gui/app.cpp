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
constexpr float kFooterHeight = 56.0f;

// 绿色椭圆桌面相对于可用牌桌区域的尺寸和起点。
constexpr float kTableXRatio = 0.115f;
constexpr float kTableYRatio = 0.115f;
constexpr float kTableWidthRatio = 0.77f;
constexpr float kTableHeightRatio = 0.69f;

// 玩家信息框尺寸。
// 8、9 人桌使用 crowded 宽度，人数较少时使用 normal 宽度。
constexpr int kCrowdedPlayerCount = 8;
constexpr float kCrowdedSeatWidthRatio = 0.18f;
constexpr float kNormalSeatWidthRatio = 0.24f;
constexpr float kSeatMinWidth = 100.0f;
constexpr float kSeatMaxWidth = 500.0f;
// 增大该值会增加整个玩家信息框的高度。
constexpr float kSeatHeightToWidth = 0.56f;

// 玩家围绕桌面的椭圆半径。数值越大，玩家框越靠近窗口边缘。
constexpr float kSeatRadiusXRatio = 0.39f;
constexpr float kSeatRadiusYRatio = 0.34f;
constexpr float kSeatHorizontalEdgeReserve = 0.58f;
constexpr float kSeatVerticalEdgeReserve = 0.60f;
constexpr float kSeatCenterYRatio = 0.47f;

// 9 人桌的局部座位微调。
// 相对位置编号：人类玩家为 0，从人类位置开始顺时针依次为 1...8。
// 1/8 位于人类两侧：向下并略微向左右外侧移动。
constexpr float kNineSeatLowerSideOutwardXRatio = 0.012f;
constexpr float kNineSeatLowerSideDownYRatio = 0.045f;
// 3/6 位于左上和右上：分别向左上、右上移动。
constexpr float kNineSeatUpperCornerOutwardXRatio = 0.022f;
constexpr float kNineSeatUpperCornerUpYRatio = 0.035f;

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

namespace action_layout {

// Raise 滑动条调节参数。
constexpr float kRaiseSliderHeight = 54.0f;
constexpr float kRaiseSliderTrackHeightRatio = 0.14f;
constexpr float kRaiseSliderKnobHeightRatio = 0.40f;
// 滑动条位置到筹码数量采用幂函数，左侧提供更细的 1 筹码级调节，
// 右侧随位置上升而明显加快。数值越大，前段越精细、后段越陡峭。
constexpr float kRaiseCurveExponent = 2.25f;
// 非线性提示阶梯与滑轨等宽；每一级高度也使用上面的幂函数计算。
constexpr int kRaiseGuideSteps = 12;
constexpr float kRaiseGuideHeightRatio = 0.22f;
constexpr float kRaiseGuideGap = 1.5f;
// 每次滚轮移动约 1% 的滑动条位置；映射到筹码后始终至少变化 1。
constexpr int kRaiseWheelStepsPerRange = 100;
constexpr int kRaiseWheelMaxNotches = 5;

} // namespace action_layout

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
    bool autoSettlementPending = false;
    // 新决策第一次操作 Raise 时强制刷新一次完整渲染缓存。
    bool raiseInteractionNeedsFullPaint = true;
    // 上一帧的游戏视图签名，用于识别会改变 UI 结构的状态切换。
    bool paintSignatureInitialized = false;
    bool paintedHasGame = false;
    ControllerState paintedControllerState = ControllerState::Setup;
    int paintedActivePlayer = -2;
    int paintedStreet = -2;
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

bool isRoundWinner(const RoundResult& result, int playerIndex) {
    for (const PotResult& pot : result.pots) {
        if (std::find(pot.winners.begin(), pot.winners.end(), playerIndex)
                != pot.winners.end()) {
            return true;
        }
        for (const PlayerAward& award : pot.awards) {
            if (award.playerIndex == playerIndex && award.amount > 0) return true;
        }
    }
    return false;
}

std::string roundWinnersText(const RoundResult& result,
                             const std::vector<PlayerSnapshot>& players) {
    std::string summary;
    for (const PotResult& pot : result.pots) {
        for (const PlayerAward& award : pot.awards) {
            if (award.amount <= 0) continue;
            if (!summary.empty()) summary += "   ";
            if (award.playerIndex >= 0
                && award.playerIndex < static_cast<int>(players.size())) {
                summary += players[static_cast<size_t>(award.playerIndex)].name;
            } else {
                summary += "Player " + std::to_string(award.playerIndex + 1);
            }
            summary += "  +" + std::to_string(award.amount);
        }
    }
    return summary.empty() ? "No award information" : summary;
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
    state.raiseInteractionNeedsFullPaint = true;
}

void requestFullPaintForViewTransition(const ControllerView& view) {
    const int activePlayer = view.hasGame ? view.table.activePlayerIndex : -1;
    const int street = view.hasGame ? view.table.stateCode : -1;
    const bool changed = !state.paintSignatureInitialized
        || state.paintedHasGame != view.hasGame
        || state.paintedControllerState != view.state
        || state.paintedActivePlayer != activePlayer
        || state.paintedStreet != street;
    if (!changed) return;

    state.paintSignatureInitialized = true;
    state.paintedHasGame = view.hasGame;
    state.paintedControllerState = view.state;
    state.paintedActivePlayer = activePlayer;
    state.paintedStreet = street;
    app::detail::requestFullPaint();
}

void requestFirstRaiseInteractionFullPaint() {
    if (!state.raiseInteractionNeedsFullPaint) return;
    state.raiseInteractionNeedsFullPaint = false;
    app::detail::requestFullPaint();
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

void settleEndedRoundIfNeeded(const ControllerView& view) {
    if (!view.hasGame || view.state == ControllerState::Error
        || !view.table.roundEnded || view.table.roundSettled
        || state.autoSettlementPending) {
        return;
    }

    state.autoSettlementPending = true;
    const bool settled = controller.settleRound();
    state.autoSettlementPending = false;
    if (settled) {
        // settleRound() publishes a new view; schedule the next frame so the
        // table immediately shows all cards and the winner highlight.
        app::detail::requestFullPaint();
        app::requestUpdate();
    }
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
                                        resizeMainWindow(kTableWidth, kTableHeight);
                                    }
                                }).build();
                        }).build();
                }).build();
        }).build();
}

void composeSeat(eui::Ui& ui, const PlayerSnapshot& player, bool dealer,
                 bool winner, bool showCards,
                 float x, float y, float width, float height) {
    const std::string id = "game.seat." + std::to_string(player.index);
    eui::Color background = player.human
        ? eui::Color{0.11f, 0.18f, 0.15f, 0.98f}
        : eui::Color{0.075f, 0.105f, 0.092f, 0.96f};
    if (player.folded) background = {0.10f, 0.10f, 0.10f, 0.82f};

    ui.stack(id)
        .position(x, y).size(width, height)
        .content([&] {
            ui.rect(id + ".bg")
                .size(width, height).color(background).radius(9.0f)
                .border(winner ? 3.0f : (player.active ? 2.0f : 1.0f),
                        winner ? kRedHover : (player.active ? kGold : kBorder))
                .shadow(winner ? 20.0f : (player.active ? 18.0f : 8.0f), 0.0f, 4.0f,
                        winner ? eui::Color{0.76f, 0.25f, 0.27f, 0.42f}
                               : (player.active ? eui::Color{0.85f, 0.62f, 0.14f, 0.34f}
                                                  : eui::Color{0.0f, 0.0f, 0.0f, 0.24f}))
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
            // Folded players hide both face-down backs and hole cards during play.
            // Once the round is settled, showCards is true and every player's
            // revealed cards are rendered at full opacity.
            if (showCards || !player.folded) {
                composeCard(ui, id + ".card.0", first, cardX, cardY,
                            cardWidth, cardHeight, true);
                composeCard(ui, id + ".card.1", second, cardX + cardStep, cardY,
                            cardWidth, cardHeight, true);
            }

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
                .text(player.name)
                // .text(player.name + (player.human ? " (You)" : ""))
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
            // 牌局进行中，这个文本框显示玩家最近一次操作；
            // round 结算后复用同一个区域显示 handType 的牌型描述。
            const std::string bottomText = showCards
                ? (player.handDescription.empty()
                    ? "No hand information" : player.handDescription)
                : actionLabel(player);
            ui.text(id + ".action").position(
                    infoX, height * table_layout::kPlayerActionYRatio)
                .size(infoWidth, height * table_layout::kPlayerActionHeightRatio)
                .text(bottomText).fontSize(detailFont)
                .lineHeight(detailFont + table_layout::kPlayerFontLineHeightExtra)
                .color(!showCards && player.folded ? kRedHover : kText)
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
                      const RoundResult& result,
                      const std::vector<PlayerSnapshot>& players,
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

    std::string potText = table.sidePots.size() > 1
        ? "Total pot  " + std::to_string(table.pot) + "  |  "
            + std::to_string(table.sidePots.size()) + " pots"
        : "Pot  " + std::to_string(table.pot);
    // 结算态复用原 Pot 徽章展示获奖玩家及其分配到的筹码，
    // 不再在公共牌下方额外创建结果框。
    if (result.settled)
        potText += "  ->  " + roundWinnersText(result, players);
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
                 result.settled
                     ? std::clamp(cardWidth * 0.13f, 11.0f, 16.0f)
                     : std::clamp(cardWidth * 0.19f, 13.0f, 19.0f),
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

            composeCommunity(ui, view.table, view.roundResult, view.table.players,
                             width, height);

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

                float offsetX = 0.0f;
                float offsetY = 0.0f;
                if (count == 9) {
                    switch (relative) {
                    case 1:
                        offsetX = -width * table_layout::kNineSeatLowerSideOutwardXRatio;
                        offsetY = height * table_layout::kNineSeatLowerSideDownYRatio;
                        break;
                    case 3:
                        offsetX = -width * table_layout::kNineSeatUpperCornerOutwardXRatio;
                        offsetY = -height * table_layout::kNineSeatUpperCornerUpYRatio;
                        break;
                    case 6:
                        offsetX = width * table_layout::kNineSeatUpperCornerOutwardXRatio;
                        offsetY = -height * table_layout::kNineSeatUpperCornerUpYRatio;
                        break;
                    case 8:
                        offsetX = width * table_layout::kNineSeatLowerSideOutwardXRatio;
                        offsetY = height * table_layout::kNineSeatLowerSideDownYRatio;
                        break;
                    default:
                        break;
                    }
                }

                const float seatX = std::clamp(
                    centerX + radiusX * std::cos(angle) - seatWidth * 0.5f + offsetX,
                    2.0f, width - seatWidth - 2.0f);
                const float seatY = std::clamp(
                    centerY + radiusY * std::sin(angle) - seatHeight * 0.5f + offsetY,
                    2.0f, height - seatHeight - 2.0f);
                composeSeat(ui, player, player.index == view.table.dealerIndex,
                            isRoundWinner(view.roundResult, player.index),
                            view.table.roundSettled,
                            seatX, seatY, seatWidth, seatHeight);
            }
        }).build();
}

int raiseAmountFromSlider(float value, int minimum, int maximum) {
    if (maximum <= minimum) return minimum;
    const float normalized = std::clamp(value, 0.0f, 1.0f);
    return minimum + static_cast<int>(std::lround(
        std::pow(normalized, action_layout::kRaiseCurveExponent)
            * static_cast<float>(maximum - minimum)));
}

float raiseSliderValue(int amount, int minimum, int maximum) {
    if (maximum <= minimum) return 1.0f;
    const float normalized = std::clamp(
        static_cast<float>(amount - minimum) / static_cast<float>(maximum - minimum),
        0.0f, 1.0f);
    return std::clamp(
        std::pow(normalized, 1.0f / action_layout::kRaiseCurveExponent),
        0.0f, 1.0f);
}

int raiseAmountFromWheel(int amount, int minimum, int maximum,
                         int direction, int notches) {
    int adjusted = std::clamp(amount, minimum, maximum);
    const float positionStep = 1.0f
        / static_cast<float>(action_layout::kRaiseWheelStepsPerRange);
    for (int notch = 0; notch < notches; ++notch) {
        const float currentPosition = raiseSliderValue(adjusted, minimum, maximum);
        const float nextPosition = std::clamp(
            currentPosition + static_cast<float>(direction) * positionStep,
            0.0f, 1.0f);
        const int mapped = raiseAmountFromSlider(nextPosition, minimum, maximum);
        if (direction > 0)
            adjusted = std::min(maximum, std::max(adjusted + 1, mapped));
        else
            adjusted = std::max(minimum, std::min(adjusted - 1, mapped));
    }
    return adjusted;
}

void composeRaiseSlider(eui::Ui& ui, const DecisionRequest& decision,
                        bool enabled, float width) {
    const std::string id = "game.action.raise.slider";
    const float height = action_layout::kRaiseSliderHeight;

    if (!enabled) {
        ui.stack(id).size(width, height).content([&] {
            ui.rect(id + ".disabled.track")
                .position(0.0f, height * 0.30f).size(width, height * 0.16f)
                .color(kSurfaceRaised).radius(height * 0.08f).build();
            text(ui, id + ".disabled.text", "Raise unavailable", width, height,
                 13.0f, kMuted, eui::HorizontalAlign::Center);
        }).build();
        return;
    }

    const int minimum = decision.minRaise;
    const int maximum = decision.maxRaise;
    state.raiseAmount = std::clamp(state.raiseAmount, minimum, maximum);

    const float value = raiseSliderValue(state.raiseAmount, minimum, maximum);
    const float trackHeight = std::max(
        3.0f, height * action_layout::kRaiseSliderTrackHeightRatio);
    const float trackY = height * 0.38f;
    const float knobSize = std::max(
        14.0f, height * action_layout::kRaiseSliderKnobHeightRatio);
    const bool adjustable = maximum > minimum;

    ui.stack(id)
        .size(width, height)
        .sliderState(id, value, width, knobSize,
            [minimum, maximum](float nextValue) {
                requestFirstRaiseInteractionFullPaint();
                state.raiseAmount = raiseAmountFromSlider(nextValue, minimum, maximum);
            })
        .content([&] {
            // 非线性映射提示：阶梯和滑轨完全等宽。每一级高度使用与数值
            // 映射相同的幂函数，因此左侧缓慢、右侧陡峭；颜色同步由浅至深。
            // 该图形仅作视觉说明，不参与交互命中测试。
            const float guideHeight = height * action_layout::kRaiseGuideHeightRatio;
            const float guideY = 1.0f;
            const float totalGap = action_layout::kRaiseGuideGap
                * static_cast<float>(action_layout::kRaiseGuideSteps - 1);
            const float stepWidth = std::max(
                1.0f,
                (width - totalGap)
                    / static_cast<float>(action_layout::kRaiseGuideSteps));
            const eui::Color lightGold{0.98f, 0.88f, 0.58f, 0.34f};
            const eui::Color deepAmber{0.58f, 0.29f, 0.035f, 0.92f};
            for (int step = 0; step < action_layout::kRaiseGuideSteps; ++step) {
                const float progress = static_cast<float>(step + 1)
                    / static_cast<float>(action_layout::kRaiseGuideSteps);
                const float curved = std::pow(
                    progress, action_layout::kRaiseCurveExponent);
                const float stepHeight = 1.5f
                    + (guideHeight - 1.5f) * curved;
                const float shade = std::pow(progress, 1.20f);
                const eui::Color stepColor{
                    lightGold.r + (deepAmber.r - lightGold.r) * shade,
                    lightGold.g + (deepAmber.g - lightGold.g) * shade,
                    lightGold.b + (deepAmber.b - lightGold.b) * shade,
                    lightGold.a + (deepAmber.a - lightGold.a) * shade,
                };
                const float stepX = static_cast<float>(step)
                    * (stepWidth + action_layout::kRaiseGuideGap);
                ui.rect(id + ".curve.step." + std::to_string(step))
                    .position(stepX, guideY + guideHeight - stepHeight)
                    .size(stepWidth, stepHeight)
                    .color(stepColor).radius(std::min(2.0f, stepWidth * 0.20f))
                    .build();
            }
            ui.rect(id + ".track")
                .position(0.0f, trackY).size(width, trackHeight)
                .color(kSurfaceRaised).radius(trackHeight * 0.5f).build();
            ui.rect(id + ".fill")
                .position(0.0f, trackY).size(width * value, trackHeight)
                .color(kGold).radius(trackHeight * 0.5f)
                .sliderFillFrom(id).build();
            ui.rect(id + ".knob")
                .position(0.0f, trackY + (trackHeight - knobSize) * 0.5f)
                .size(knobSize, knobSize).color(kText).radius(knobSize * 0.5f)
                .shadow(10.0f, 0.0f, 3.0f, {0.82f, 0.60f, 0.17f, 0.24f})
                .sliderKnobFrom(id).build();

            const float labelY = height * 0.52f;
            const float labelHeight = height * 0.45f;
            ui.text(id + ".minimum").position(0.0f, labelY)
                .size(width * 0.25f, labelHeight)
                .text(std::to_string(minimum)).fontSize(12.0f).lineHeight(16.0f)
                .color(kMuted).verticalAlign(eui::VerticalAlign::Center).build();
            ui.text(id + ".current").position(width * 0.25f, labelY)
                .size(width * 0.50f, labelHeight)
                .text(std::to_string(state.raiseAmount)).fontSize(15.0f).lineHeight(19.0f)
                .color(kGold).horizontalAlign(eui::HorizontalAlign::Center)
                .verticalAlign(eui::VerticalAlign::Center).build();
            ui.text(id + ".maximum").position(width * 0.75f, labelY)
                .size(width * 0.25f, labelHeight)
                .text(std::to_string(maximum)).fontSize(12.0f).lineHeight(16.0f)
                .color(kMuted).horizontalAlign(eui::HorizontalAlign::Right)
                .verticalAlign(eui::VerticalAlign::Center).build();

            if (adjustable) {
                ui.rect(id + ".hit")
                    .size(width, height).color({0.0f, 0.0f, 0.0f, 0.0f})
                    .zIndex(10).interactive().sliderInputFrom(id)
                    .onScroll([minimum, maximum](const core::ScrollEvent& event) {
                        if (std::fabs(event.y) <= 0.001) return;
                        requestFirstRaiseInteractionFullPaint();
                        const int notches = std::clamp(
                            static_cast<int>(std::lround(std::fabs(event.y))),
                            1, action_layout::kRaiseWheelMaxNotches);
                        const int direction = event.y > 0.0 ? 1 : -1;
                        state.raiseAmount = raiseAmountFromWheel(
                            state.raiseAmount, minimum, maximum, direction, notches);
                    }).build();
            }
        }).build();
}

void humanActions(eui::Ui& ui, const ControllerView& view, float width) {
    const DecisionRequest& decision = view.table.decision;
    const bool canFold = hasAction(decision, FOLD);
    const bool canCall = hasAction(decision, CALL);
    const bool canRaise = hasAction(decision, RAISE);
    // 缩短 Raise 控件，避免在宽屏下吞掉整行操作区；剩余空间由行布局留白。
    const float raiseControlWidth = std::clamp(width * 0.22f, 230.0f, 380.0f);

    ui.row("game.actions")
        .size(width, 56.0f).gap(10.0f)
        .alignItems(eui::Align::CENTER).justifyContent(eui::Align::CENTER)
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
            composeRaiseSlider(ui, decision, canRaise, raiseControlWidth);
            components::button(ui, "game.action.raise")
                .size(145.0f, 44.0f)
                .text(state.raiseAmount == decision.maxRaise
                    ? "All-in" : "Raise " + std::to_string(state.raiseAmount))
                .radius(6.0f).colors(kGold, kGoldHover, kGoldPressed)
                .disabled(!canRaise)
                .onClick([] { controller.submitHumanAction({RAISE, state.raiseAmount}); }).build();
        }).build();
}

void returnToSetup() {
    controller.returnToSetup();
    state.botTaskPending = false;
    resizeMainWindow(kSetupWidth, kSetupHeight);
}

void nextRound() {
    if (controller.nextRound()) {
        state.decisionPlayer = -1;
    }
}

void roundActions(eui::Ui& ui, const ControllerView& view, float width) {
    ui.row("game.round.actions")
        .size(width, 48.0f).gap(10.0f).alignItems(eui::Align::CENTER)
        .content([&] {
            if (!view.table.roundSettled) {
                text(ui, "game.round.done", "Settling result...", 220.0f, 44.0f,
                     14.0f, kGold);
            } else {
                text(ui, "game.round.done", "Result shown on table", 220.0f, 44.0f,
                     14.0f, kGreenHover);
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
    ControllerView view = controller.view();
    settleEndedRoundIfNeeded(view);
    view = controller.view();
    requestFullPaintForViewTransition(view);
    if (view.hasGame) composeGame(ui, screen, view);
    else composeSetup(ui, screen);
    composeError(ui, screen, view);
}

} // namespace app
