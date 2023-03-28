#pragma once

/* Compute cos cos cos ... cos x, with 100 cosines. Equivalently, compute
 * cos^100 x, where "cos^100" indicates 100 iterations of cosine.
 */
double cos100(double x);

/* Given f(x) = 2.00x(1 - x), compute f^100(x). */
double magic2_00(double x);

/* Given f(x) = 2.75x(1 - x), compute f^100(x). */
double magic2_75(double x);

/* Given f(x) = 3.25x(1 - x), compute f^100(x). */
double magic3_25(double x);

/* Given f(x) = 3.50x(1 - x), compute f^100(x). */
double magic3_50(double x);

/* Given f(x) = 3.75x(1 - x), compute f^100(x). */
double magic3_75(double x);

/* Given f(x) = 3.99x(1 - x), compute f^100(x). */
double magic3_99(double x);
