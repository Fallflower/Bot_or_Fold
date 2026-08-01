#pragma once

namespace holdem::gui {

enum class Presentation {
    Setup,
    Game
};

void requestPresentation(Presentation presentation, int preferredWidth, int preferredHeight);

} // namespace holdem::gui
