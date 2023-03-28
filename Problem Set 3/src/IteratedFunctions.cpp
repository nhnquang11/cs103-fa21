#include "IteratedFunctions.h"
#include <cmath> // For cos
#include <stdexcept>
using namespace std;

/* Compute cos cos cos ... cos x, with 100 cosines. Equivalently, compute
 * cos^100 x, where "cos^100" indicates 100 iterations of cosine.
 */
double cos100(double x) {
    double result = x;
    for (int i = 0; i < 100; ++i) {
        result = cos(result);
    }
    return result;
}

/* Given f(x) = nx(1 - x), compute f^100(x). */
double magic_template(double n, double x) {
    double result = x;
    for (int i = 0; i < 100; ++i) {
        result = n * result * (1 - result);
    }
    return result;
}

/* Given f(x) = 2.00x(1 - x), compute f^100(x). */
double magic2_00(double x) {
    return magic_template(2.00, x);
}

/* Given f(x) = 2.75x(1 - x), compute f^100(x). */
double magic2_75(double x) {
    return magic_template(2.75, x);
}

/* Given f(x) = 3.25x(1 - x), compute f^100(x). */
double magic3_25(double x) {
    return magic_template(3.25, x);
}

/* Given f(x) = 3.50x(1 - x), compute f^100(x). */
double magic3_50(double x) {
    return magic_template(3.50, x);
}

/* Given f(x) = 3.75x(1 - x), compute f^100(x). */
double magic3_75(double x) {
    return magic_template(3.75, x);
}

/* Given f(x) = 3.99x(1 - x), compute f^100(x). */
double magic3_99(double x) {
    return magic_template(3.99, x);
}
