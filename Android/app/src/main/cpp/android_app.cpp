#include "eui_neo.h"

#include <algorithm>

namespace app {

const DslAppConfig& dslAppConfig() {
    static const DslAppConfig config = DslAppConfig{}
        .title("Bot or Fold")
        .pageId("holdem_android_foundation")
        .windowSize(1920, 1080)
        .fps(60.0)
        .iconFont("assets/Font Awesome 7 Free-Solid-900.otf")
        .showDebugStatsInTitle(false);
    return config;
}

void compose(eui::Ui& ui, const eui::Screen& screen) {
    const eui::Color background{0.055f, 0.070f, 0.065f, 1.0f};
    const eui::Color surface{0.095f, 0.110f, 0.105f, 1.0f};
    const eui::Color green{0.12f, 0.55f, 0.34f, 1.0f};
    const eui::Color text{0.94f, 0.96f, 0.95f, 1.0f};
    const eui::Color muted{0.62f, 0.68f, 0.65f, 1.0f};
    const float panelWidth = std::min(920.0f, std::max(0.0f, screen.width - 64.0f));
    const float panelHeight = std::min(500.0f, std::max(0.0f, screen.height - 64.0f));
    const float panelX = (screen.width - panelWidth) * 0.5f;
    const float panelY = (screen.height - panelHeight) * 0.5f;

    ui.stack("root")
        .size(screen.width, screen.height)
        .content([&] {
            ui.rect("background").size(screen.width, screen.height).color(background).build();
            ui.rect("foundation.panel.background")
                .position(panelX, panelY)
                .size(panelWidth, panelHeight)
                .radius(28.0f)
                .color(surface)
                .build();
            ui.column("foundation.panel")
                .position(panelX + 48.0f, panelY + 48.0f)
                .size(panelWidth - 96.0f, panelHeight - 96.0f)
                .gap(24.0f)
                .content([&] {
                    ui.image("foundation.icon")
                        .size(112.0f, 112.0f)
                        .source("assets/icon.png")
                        .radius(24.0f)
                        .build();
                    ui.text("foundation.title")
                        .size(panelWidth - 96.0f, 60.0f)
                        .text("Bot or Fold")
                        .fontSize(44.0f)
                        .lineHeight(54.0f)
                        .color(text)
                        .build();
                    ui.text("foundation.status")
                        .size(panelWidth - 96.0f, 48.0f)
                        .text("Android foundation ready: SDL2 + Vulkan + EUI-NEO")
                        .fontSize(26.0f)
                        .lineHeight(34.0f)
                        .color(green)
                        .build();
                    ui.text("foundation.next")
                        .size(panelWidth - 96.0f, 72.0f)
                        .text("The game GUI will be connected after removing its direct GLFW dependency.")
                        .fontSize(22.0f)
                        .lineHeight(30.0f)
                        .wrap(true)
                        .color(muted)
                        .build();
                })
                .build();
        })
        .build();
}

} // namespace app
