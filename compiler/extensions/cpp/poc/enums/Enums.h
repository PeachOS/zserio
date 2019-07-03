#ifndef ZSERIO_ENUMS_H_INC
#define ZSERIO_ENUMS_H_INC

#include <type_traits>

#include "zserio/BitStreamReader.h"
#include "zserio/BitStreamWriter.h"

namespace zserio
{

template<typename T>
struct EnumTraits
{
};

template<typename T>
size_t enumToOrdinal(T value);

template<typename T>
T valueToEnum(typename std::underlying_type<T>::type rawValue);

template<typename T>
constexpr typename std::underlying_type<T>::type enumToValue(T value)
{
    return static_cast<typename std::underlying_type<T>::type>(value);
}

template<typename T>
const char* enumToString(T value)
{
    return zserio::EnumTraits<T>::names[enumToOrdinal(value)];
}

template <typename T>
size_t bitSizeOf(T value)
{
}

template <typename T>
size_t initializeOffsets(T value, size_t bitPosition)
{
}

template <typename T>
T read(zserio::BitStreamReader& in)
{
}

template <typename T>
void write(BitStreamWriter& out, T value)
{
}

} // namespace zserio

#endif // ZSERIO_ENUMS_H_INC