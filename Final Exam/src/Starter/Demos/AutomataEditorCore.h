/* Utilities for making the editor/tester/debugger work seamlessly
 * together.
 */
#pragma once

#include "AutomataEditor.h"
#include <vector>
#include <string>
#include <istream>
#include <ostream>
#include <memory>

namespace Core {
    /* For simplicity, the UI caches the name of the most recently viewed automaton.
     * These functions let you set/retrieve that name.
     */
    void setLastFilename(const std::string& filename);
    std::string lastFilename(); // Empty string if none selected
}
