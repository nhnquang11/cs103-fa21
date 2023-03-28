#include "SetInternal.h"
#include "StrUtils/StrUtils.h"
#include <iterator>
#include <sstream>
using namespace std;

namespace SetTheory {
    /* These functions just introspect on the internal details of the SetObject type.
     * We provided these functions as wrappers around the internal state to simplify
     * the overall logic you needed to write.
     */
    bool isSet(Object o) {
        if (!isValid(o)) throw runtime_error("Uninitialized object.");
        return o.impl->isSet();
    }

    /* Ask the underlying object for a set. */
    std::set<Object> asSet(Object o) {
        if (!isValid(o)) throw runtime_error("Uninitialized object.");
        return o.impl->asSet();
    }

    /* Check if the internal object exists. */
    bool isValid(Object obj) {
        return obj.impl != nullptr;
    }


    /* * * * * Implementation of the set types. * * * * */

    /* An ActualSet is an object that wraps a std::set<Object> */
    bool ActualSet::isSet() const {
        return true;
    }
    set<Object> ActualSet::asSet() const {
        return theSet;
    }

    /* Converting a set to a string embraces everything and then puts in commas
     * as appropriate.
     *
     * I'm not sure that "embrace" means "to put braces around," but it should.
     */
    string ActualSet::toString() const {
        ostringstream result;
        result << "{";

        /* List all the members, comma-separated. */
        for (auto itr = theSet.begin(); itr != theSet.end(); itr++) {
            result << *itr;
            if (next(itr) != theSet.end()) {
                result << ", ";
            }
        }

        result << "}";
        return result.str();
    }

    /* Actual objects are just wrappers around strings. */
    bool ActualObject::isSet() const {
        return false;
    }
    set<Object> ActualObject::asSet() const {
        throw runtime_error("Object " + name + " is not a set!");
    }
    string ActualObject::toString() const {
        return name;
    }

    /* * * * * General implementation of the Object type. */

    /* Sets are compared lexicographically by their string ordering.
     * This produces reasonable results for most objects, though sets
     * of numbers might display in a weird order.
     *
     * Then again, sets aren't supposed to be ordered in the first place,
     * so... :-)
     */
    bool Object::operator< (const Object& rhs) const {
        return impl->toString() < rhs.impl->toString();
    }

    /* Output operator just dumps the string representation. */
    ostream& operator<< (ostream& out, const Object& obj) {
        return out << obj.impl->toString();
    }
}
