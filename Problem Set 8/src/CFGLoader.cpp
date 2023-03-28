#include "CFGLoader.h"
#include "FormalLanguages/CFGParser.h"
#include "FileParser/FileParser.h"
using namespace std;

CFG::CFG loadCFG(const string& section, const Languages::Alphabet& alphabet) {
    auto allAnswers = parseFile("res/Grammars.cfgs");
    string headerName = "[" + section + "]";
    if (!allAnswers.count(headerName)) {
        throw runtime_error("No section labeled " + headerName + " in res/Grammars.cfgs");
    }
    try {
        return CFG::parse(CFG::scan(*allAnswers[headerName]), alphabet);
    } catch (const exception& e) {
        throw runtime_error(string("Error parsing CFG: ") + e.what());
    }
}
