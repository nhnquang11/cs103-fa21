#pragma once

/* Utilities to get snazzy renderings of CFGs and CFG components. */

#include "../FormalLanguages/CFG.h"
#include <string>

/* Renders a CFG as HTML using the styling from lecture. */
std::string cfgToHTML(const CFG::CFG& cfg);

/* Renders a production as HTML using the styling from lecture. */
std::string productionToHTML(const CFG::Production& prod);

/* Renders a symbol as HTML. */
enum class RenderType {
    NORMAL,
    HIGHLIGHT,
    FADE
};

std::string symbolToHTML (CFG::Symbol symbol, RenderType type = RenderType::NORMAL);
