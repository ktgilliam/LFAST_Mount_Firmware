#include <cmath>

template <typename T>
inline T ulim(T val, T lim)
{
    return val > lim ? val : lim;
}

template <typename T>
inline T llim(T val, T lim)
{
    return val < lim ? val : lim;
}

template <typename T>
inline T saturate(T val, T lower, T upper)
{
    T val1 = llim(val, lower);
    T val2 = ulim(val1, upper);
    return val2;
}