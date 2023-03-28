#include "ExecutableLogic.h"
#include <stdexcept>

/* ∃x. Cat(x) */
bool isFormulaTrueFor_partI(std::set<Entity> world) {
    for (Entity x: world) {
        if (Cat(x)) {
            return true;
        }
    }
    return false;
}

/* ∀x. Robot(x) */
bool isFormulaTrueFor_partII(std::set<Entity> world) {
    for (Entity x: world) {
        if (!Robot(x)) {
            return false;
        }
    }
    return true;
}

/* ∃x. (Person(x) ∧ Loves(x, x)) */
bool isFormulaTrueFor_partIII(std::set<Entity> world) {
    for (Entity x: world) {
        if (Person(x) && Loves(x, x)) {
            return true;
        }
    }
    return false;
}

/* ∀x. (Cat(x) → Loves(x, x)) */
bool isFormulaTrueFor_partIV(std::set<Entity> world) {
    for (Entity x: world) {
        if (Cat(x) && !Loves(x, x)) {
            return false;
        }
    }
    return true;
}

/*
 * ∀x. (Cat(x) →
 *   ∃y. (Person(y) ∧ ¬Loves(x, y))
 * )
 */
bool isFormulaTrueFor_partV(std::set<Entity> world) {
    for (Entity x: world) {
        if (Cat(x)) {
            bool notLovedBySomeone = false;
            for (Entity y: world) {
                if (Person(y) && !Loves(x, y)) {
                    notLovedBySomeone = true;
                    break;
                }
            }
            if (!notLovedBySomeone) return false;
        }
    }
    return true;
}

/*
 * ∃x. (Robot(x) ↔
 *   ∀y. Loves(x, y)
 * )
 */
bool isFormulaTrueFor_partVI(std::set<Entity> world) {
    /*
     * Robot(x) ↔ ∀y. Loves(x, y) is equivalent to:
     * (¬Robot(x) ∨ ∀y. Loves(x, y)) ∧ (∃y. ¬Loves(x, y) ∨ Robot(x))
     * p1 = ¬Robot(x)
     * p2 = ∀y. Loves(x, y)
     * p3 = ∃y. ¬Loves(x, y)
     * p4 = Robot(x)
     */
    for (Entity x: world) {
        bool p1 = !Robot(x);
        bool p4 = !p1;
        bool p2 = true;
        bool p3 = false;
        for (Entity y: world) {
            if (!Loves(x, y)) {
                p2 = false;
                p3 = true;
                break;
            }
        }
        if ((p1 || p2) && (p3 || p4)) {
            return true;
        }
    }
    return false;
}
