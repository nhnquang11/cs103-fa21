#pragma once

#include "../FormalLanguages/Languages.h"
#include "../FormalLanguages/Automaton.h"
#include "../GraphEditor/GraphViewer.h"
#include "Utilities/JSON.h"
#include "gobjects.h"
#include "gwindow.h"
#include <string>
#include <set>
#include <unordered_map>
#include <memory>
#include <functional>

namespace Editor {
    class Automaton;

    class State: public GraphEditor::Node {
    public:
        State(GraphEditor::ViewerBase* base,
              const GraphEditor::NodeArgs& args,
              JSON aux);

        bool start();
        void start(bool start);

        bool accepting();
        void accepting(bool accepting);

        virtual JSON toJSON() override;

        virtual void draw(GraphEditor::ViewerBase* base,
                          GCanvas*    canvas,
                          const GraphEditor::NodeStyle& style) override;

    private:
        bool mStart = false;
        bool mAccepting = false;
    };

    class Transition: public GraphEditor::Edge {
    public:
        Transition(GraphEditor::ViewerBase* base,
                   const GraphEditor::EdgeArgs& args,
                   JSON aux);

        const std::set<char32_t>& chars();
        void add(char32_t ch);
        void remove(char32_t ch);

        virtual JSON toJSON() override;

    private:

        class Automaton*   mOwner;
        std::set<char32_t> mChars;

        void updateLabel();
    };

    class Automaton: public GraphEditor::Viewer<State, Transition> {
    public:
        Automaton(JSON j);

        Languages::Alphabet alphabet();
        bool isDFA();
        Automata::NFA toNFA();

        std::vector<std::string> checkValidity();

    protected:
        JSON auxData() override;

    private:
        Languages::Alphabet mAlphabet;
        bool mIsDFA;
    };
}
