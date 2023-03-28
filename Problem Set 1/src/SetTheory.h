#pragma once
#include "SetTheory/Object.h"

/* You need to implement these functions. */
bool isElementOf(Object S, Object T);
bool isSubsetOf(Object S, Object T);
bool areDisjointSets(Object S, Object T);
bool isSingletonOf(Object S, Object T);
bool isElementOfPowerSet(Object S, Object T);
bool isSubsetOfPowerSet(Object S, Object T);
bool isSubsetOfDoublePowerSet(Object S, Object T);
