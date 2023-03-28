#pragma once

#include <memory>
#include <istream>
#include <queue>
#include <string>
#include <set>
#include <ostream>

namespace SetTheory {
    struct SetObject;
    class Object parse(std::istream& source);

    namespace Detail {
        template <typename T> struct Identity { typedef T type; };
    }

    /* Type: Object
     *
     * An opaque type representing an object, which can either be a set or
     * some other object.
     *
     * Please do not access the fields of this struct. Use our provided helper functions
     * to operate on Objects.
     */
    class Object {
    public:
        Object() = default;

        bool operator< (const Object& rhs) const;    // Needed for std::set

        /* You are not allowed to directly compare Objects against one another for equality.
         * If you get an error on this line, it's probably because you tried writing something
         * like
         *
         *     if (obj1 == obj2) { ... }
         *
         * If that's the case, see if you can find another way to express the idea you're working
         * with.
         */
        template <typename T> bool operator==(const T&) {
            static_assert(sizeof(typename Detail::Identity<T>::type) == 0,
                          "Oops! For the purposes of this assignment, you aren't allowed to compare objects against one another using the == operator. See if you can find another approach.");
            return false;
        }

    private:
        std::shared_ptr<const SetTheory::SetObject> impl;

        static Object parse(std::istream&);
        static Object parseSet(std::queue<std::string>& tokens);
        static Object parseThing(std::queue<std::string>& tokens);
        static Object parseObject(std::queue<std::string>& tokens);
        static Object parseSingleObject(std::queue<std::string>& tokens);

        friend SetTheory::Object SetTheory::parse(std::istream&);
        friend std::ostream& operator<< (std::ostream&, const Object&);
        friend bool isSet(Object);
        friend std::set<Object> asSet(Object);
        friend bool isValid(Object);

        Object(std::shared_ptr<const SetTheory::SetObject> impl) : impl(impl) {}
    };

    template <typename T> bool operator==(const T& lhs, Object rhs) {
        return rhs == lhs;
    }

    /* Operator <<
     *
     * Allows you to print out an object to the console for debugging purposes. You can use it like
     * this:
     *
     *    Object obj = // ... //
     *    std::cout << "Object: " << obj << std::endl;
     */
    std::ostream& operator<< (std::ostream& out, const Object& obj);

    /* Given an object, returns whether that object is a set.
     *
     * For example, given an object representing the number 1,
     * this would return false. Given an object representing
     * { 1, 2, 3 }, this function would return true.
     */
    bool isSet(Object o);

    /* Given an object that represents a set, returns a view of
     * that object as a set.
     *
     * For example, suppose you have something like this:
     *
     *    Object o = // ... something you know is a set ... //
     *
     * You could then actually see the contents of that set
     * by writing something like
     *
     *    std::set<Object> S = asSet(o);
     *
     * If you try to convert an object to a set and that object
     * isn't actually a set, this function will trigger an error.
     *
     * Here's a nice way to iterate over all the contents of an
     * Object that you know is a set:
     *
     *    Object o = // ... something you know is a set .. //
     *    for (Object x: asSet(o)) {
     *       // ... do something to x ... //
     *    }
     */
    std::set<Object> asSet(Object o);

    /* Returns whether an object is valid. Invalid objects result from
     * default-constructing an object.
     */
    bool isValid(Object o);
}

using SetTheory::Object;
using SetTheory::asSet;
using SetTheory::isSet;
