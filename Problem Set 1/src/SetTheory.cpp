#include "SetTheory.h"
#include "error.h"
using namespace std;

/* True or false: S in T? */
bool isElementOf(Object S, Object T) {
    if (!isSet(T)) return false;
    std::set<Object> tSet = asSet(T);
    return tSet.count(S);
}

/* True or false: S is a subset of T? */
bool isSubsetOf(Object S, Object T) {
    if (!isSet(T) || !isSet(S)) return false;
    std::set<Object> sSet = asSet(S);
    std::set<Object> tSet = asSet(T);
    for (Object elem: sSet) {
        if (!tSet.count(elem)) {
            return false;
        }
    }
    return true;
}

/* True or false: S and T are sets, and S n T = emptyset? */
bool areDisjointSets(Object S, Object T) {
    if (!isSet(T) || !isSet(S)) return false;
    std::set<Object> sSet = asSet(S);
    std::set<Object> tSet = asSet(T);
    for (Object elem: sSet) {
        if (tSet.count(elem)) {
            return false;
        }
    }
    return true;
}

/* True or false: S = {T}? */
bool isSingletonOf(Object S, Object T) {
    if (!isSet(S)) return false;
    std::set<Object> sSet = asSet(S);
    return sSet.size() == 1 && sSet.count(T);
}

/* True or false: S and T are sets, and S in P(T)? */
bool isElementOfPowerSet(Object S, Object T) {
    if (!isSet(T) || !isSet(S)) return false;
    std::set<Object> sSet = asSet(S);
    std::set<Object> tSet = asSet(T);
    for (Object elem: sSet) {
        if (!tSet.count(elem)) {
            return false;
        }
    }
    return true;
}

/* True or false: S and T are sets, and S is a subset of P(T)? */
bool isSubsetOfPowerSet(Object S, Object T) {
    if (!isSet(T) || !isSet(S)) return false;
    std::set<Object> sSet = asSet(S);
    std::set<Object> tSet = asSet(T);
    for (Object elem: sSet) {
        if (!isElementOfPowerSet(elem, T)) {
            return false;
        }
    }
    return true;
}

/* True or false: S and T are sets, and S is a subset of P(P(T))? */
bool isSubsetOfDoublePowerSet(Object S, Object T) {
    if (!isSet(T) || !isSet(S)) return false;
    std::set<Object> sSet = asSet(S);
    std::set<Object> tSet = asSet(T);
    for (Object elem: sSet) {
        if (!isSubsetOfPowerSet(elem, T)) {
            return false;
        }
    }
    return true;
}
