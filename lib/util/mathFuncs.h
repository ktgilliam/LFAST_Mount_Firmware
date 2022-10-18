#include <cmath>

template <typename T, typename U>
inline T ulim(T val, U lim)
{
    return val > lim ? val : lim;
}

template <typename T, typename U>
inline T llim(T val, U lim)
{
    return val < lim ? val : lim;
}

template <typename T, typename U, typename V>
inline T saturate(T val, U lower, V upper)
{
    T val1 = llim(val, lower);
    T val2 = ulim(val1, upper);
    return val2;
}
