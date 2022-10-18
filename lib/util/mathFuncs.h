#include <cmath>

template <typename T, typename U>
inline T ulim(T val, U upper)
{
    return val < upper ? val : upper;
}

template <typename T, typename U>
inline T llim(T val, U lower)
{
    return val > lower ? val : lower;
}

template <typename T, typename U, typename V>
inline T saturate(T val, U lower, V upper)
{
    T val1 = llim(val, lower);
    T val2 = ulim(val1, upper);

    // T val1 = val > lower ? val : lower;
    // T val2 = val1 < upper ? val1 : upper;
    return val2;
}

template <typename T>
inline int sign(T val)
{
    if (val == 0.0)
        return 0.0;
    else
        return std::signbit(val) ? -1 : 1;
}

template <typename T>
inline double ha2rad(T val)
{
    return val * M_PI / 12.0;
}

template <typename T>
inline double rad2ha(T val)
{
    return val * 12.0 / M_PI;
}

template <typename T>
inline double rad2deg(T val)
{
    return val * 180.0/ M_PI;
}

template <typename T>
inline double deg2rad(T val)
{
    return val * M_PI / 180.0;
}
