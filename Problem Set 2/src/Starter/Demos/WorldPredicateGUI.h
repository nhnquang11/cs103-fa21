#pragma once

#include "../GUI/MiniGUI.h"
#include "WorldViewer.h"
#include <string>
#include <vector>
#include <istream>

class WorldPredicateGUI: public ProblemHandler {
public:
    WorldPredicateGUI(GWindow& window,
                      const std::vector<PredicatedWorld>& worlds,
                      const std::string& title,
                      const std::string& description);

    void windowResized() override;
    void actionPerformed(GObservable* source) override;

    static std::function<bool(const World& world)> parse(std::istream& in, const FOL::BuildContext& context = entityBuildContext());
    static std::function<bool(const World& world)> parse(const std::string& formula, FOL::BuildContext context = entityBuildContext());

    static void doConsole(const std::vector<PredicatedWorld>& worlds,
                          const std::string& title,
                          const std::string& whatsBeingShown);

protected:
    void repaint() override;

private:
    void updateBounds();
    void setIndex(size_t index);

    GRectangle predicateBounds();
    GRectangle titleBounds();
    GRectangle descBounds();
    GRectangle viewerBounds();

    void drawPredicates();
    void drawTitle();
    void drawDesc();
    void drawErrorMessage(const std::string& message);

    /* [<] world [>] */
    Temporary<GContainer> console;
      GLabel*  desc;
      GContainer* buttons;
        GButton* prev;
        GButton* next;

    std::vector<std::shared_ptr<WorldViewer>> viewers;
    size_t index = 0;

    std::string title;
    std::string description;
};
