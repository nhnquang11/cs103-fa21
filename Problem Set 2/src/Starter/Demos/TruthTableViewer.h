#pragma once

#include "../GUI/MiniGUI.h"
#include "../Logic/PLExpression.h"
#include <map>
#include <string>

/* Either a PL formula or an error. We really need to get std::variant
 * support. :-)
 */
struct FormulaOrError {
    std::shared_ptr<PL::Expression> formula;
    std::string error;
    std::string section;
};

class TruthTableViewer: public ProblemHandler {
public:
    TruthTableViewer(GWindow& window, const std::string& problemName, const std::string& file);

    void changeOccurredIn(GObservable* source) override;

    static void doConsole(const std::string& problemName,
                          const std::string& filename);

private:
    Temporary<GColorConsole> console;
    Temporary<GComboBox> chooser;

    std::map<std::string, FormulaOrError> formulas;

    std::string problemName;

    void updateDisplay(FormulaOrError f);
    FormulaOrError selectedItem();
};
