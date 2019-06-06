/**
 * Automatically generated by Zserio C++ extension version 1.2.0.
 */

#ifndef ZSERIO_ENUMS_H_INC
#define ZSERIO_ENUMS_H_INC

#include <type_traits>

// This should be implemented in runtime library header.
namespace zserio
{

namespace enums
{

template<typename T>
struct Traits
{
};

template<typename T>
size_t toOrdinal(T value);

template<typename T>
T fromValue(typename std::underlying_type<T>::type rawValue);

template<typename T>
constexpr typename std::underlying_type<T>::type toValue(T value)
{
    return static_cast<typename std::underlying_type<T>::type>(value);
}

template<typename T>
const char* toString(T value)
{
    return Traits<T>::names[toOrdinal(value)];
}

} // namespace enums

} // namespace zserio

#endif // ZSERIO_ENUMS_H_INC