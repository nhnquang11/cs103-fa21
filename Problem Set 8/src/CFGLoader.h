#pragma once

#include "FormalLanguages/CFG.h"
#include <string>

/* Loads the indicated CFG from the Grammars.cfgs file. */
CFG::CFG loadCFG(const std::string& section, const Languages::Alphabet& alphabet);
