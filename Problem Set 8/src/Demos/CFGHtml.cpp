#include "CFGHtml.h"
#include "../GUI/MiniGUI.h"
#include "Utilities/JSON.h"
#include <sstream>
using namespace std;

namespace {
    const string kNonterminalColor = "red";
    const string kTerminalColor    = "blue";
    const string kHighlightColor   = "#c000ff"; // Nice royal purple color
    const string kFadeColor        = "#a0a0a0";

    const string kNonterminal = R"(<span style="color:%s;font-family:serif;"><b>%s</b></span>)";
    const string kTerminal = R"(<span style="color:%s;font-family:monospace;"><b><tt>%s</tt></b></span>)";

    string symbolsToHTML(const vector<CFG::Symbol>& symbols) {
        ostringstream result;
        if (symbols.empty()) {
            result << "&epsilon;";
        } else {
            for (const auto& s: symbols) {
                result << symbolToHTML(s);
            }
        }

        return result.str();
    }

    string colorFor(const string& base, RenderType type) {
        if (type == RenderType::NORMAL) return base;
        if (type == RenderType::HIGHLIGHT) return kHighlightColor;
        if (type == RenderType::FADE) return kFadeColor;
        abort(); // Logic error!
    }
}

string productionToHTML(const CFG::Production& prod) {
    ostringstream result;
    result << symbolToHTML(CFG::nonterminal(prod.nonterminal))
           << " &rarr; "
           << symbolsToHTML(prod.replacement);
    return result.str();
}

string cfgToHTML(const CFG::CFG& cfg) {
    /* Group productions by their nonterminal. Keep the order of the nonterminals
     * from the original grammar.
     */
    map<char32_t, vector<string>> productions;
    vector<char32_t> order;
    for (const auto& prod: cfg.productions) {
        /* New nonterminal? It comes next. */
        if (!productions.count(prod.nonterminal)) {
            order.push_back(prod.nonterminal);
        }

        productions[prod.nonterminal].push_back(symbolsToHTML(prod.replacement));
    }

    /* Gather everything back together. */
    ostringstream result;
    for (char32_t nonterminal: order) {
        result << symbolToHTML(CFG::nonterminal(nonterminal)) << " &rarr; ";
        for (size_t i = 0; i < productions[nonterminal].size(); i++) {
            result << productions[nonterminal][i];
            if (i + 1 != productions[nonterminal].size()) {
                result << "&nbsp; | &nbsp;";
            }
        }

        result << "<br>";
    }

    return result.str();
}

string symbolToHTML(CFG::Symbol symbol, RenderType type) {
    if (symbol.type == CFG::Symbol::Type::TERMINAL) {
        return format(kTerminal, colorFor(kTerminalColor, type), toUTF8(symbol.ch));
    } else {
        return format(kNonterminal, colorFor(kNonterminalColor, type), toUTF8(symbol.ch));
    }
}
