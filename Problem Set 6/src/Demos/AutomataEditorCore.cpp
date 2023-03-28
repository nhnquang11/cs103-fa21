#include "AutomataEditorCore.h"
#include "Utilities/Unicode.h"
#include <vector>
using namespace std;

namespace Core {
    namespace {
        string theLastFile;
    }

    string lastFilename() {
        return theLastFile;
    }
    void setLastFilename(const string& filename) {
        theLastFile = filename;
    }
}
