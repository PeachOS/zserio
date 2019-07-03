#ifndef ZSERIO_ARRAYS_H_INC
#define ZSERIO_ARRAYS_H_INC

#include <type_traits>
#include <vector>

#include "BitStreamWriter.h"
#include "BitStreamReader.h"
#include "BitPositionUtil.h"
#include "VarUInt64Util.h"
#include "BitStreamException.h"
#include "PreWriteAction.h"
#include "BitSizeOfCalculator.h"

namespace zserio
{

template <typename T>
T sum(const std::vector<T>& array)
{
    T summation = T();
    for (const T& element : array)
        summation += element;

    return summation;
}

template <typename ARRAY_TRAITS>
size_t bitSizeOf(const std::vector<typename ARRAY_TRAITS::type>& array, size_t bitPosition)
{
    if (ARRAY_TRAITS::IS_BITSIZEOF_CONSTANT)
        return ARRAY_TRAITS::BIT_SIZE * array.size();

    size_t endBitPosition = bitPosition;
    for (const typename ARRAY_TRAITS::type& element : array)
        endBitPosition += ARRAY_TRAITS::bitSizeOf(endBitPosition, element);

    return endBitPosition - bitPosition;
}

template <typename ARRAY_TRAITS>
size_t bitSizeOfAligned(const std::vector<typename ARRAY_TRAITS::type>& array, size_t bitPosition)
{
    size_t endBitPosition = bitPosition;
    const size_t arraySize = array.size();
    if (ARRAY_TRAITS::IS_BITSIZEOF_CONSTANT && arraySize > 0)
    {
        endBitPosition = alignTo(NUM_BITS_PER_BYTE, endBitPosition);
        endBitPosition += (arraySize - 1) * alignTo(NUM_BITS_PER_BYTE, ARRAY_TRAITS::BIT_SIZE) +
                ARRAY_TRAITS::BIT_SIZE;
    }
    else
    {
        for (const typename ARRAY_TRAITS::type& element : array)
        {
            endBitPosition = alignTo(NUM_BITS_PER_BYTE, endBitPosition);
            endBitPosition += ARRAY_TRAITS::bitSizeOf(endBitPosition, element);
        }
    }

    return endBitPosition - bitPosition;
}

template <typename ARRAY_TRAITS>
size_t bitSizeOfAuto(const std::vector<typename ARRAY_TRAITS::type>& array, size_t bitPosition)
{
    const size_t lengthBitSizeOf = getBitSizeOfVarUInt64(array.size());

    return lengthBitSizeOf + bitSizeOf<ARRAY_TRAITS>(array, bitPosition + lengthBitSizeOf);
}

template <typename ARRAY_TRAITS>
size_t bitSizeOfAlignedAuto(const std::vector<typename ARRAY_TRAITS::type>& array, size_t bitPosition)
{
    const size_t lengthBitSizeOf = getBitSizeOfVarUInt64(array.size());

    return lengthBitSizeOf + bitSizeOfAligned<ARRAY_TRAITS>(array, bitPosition + lengthBitSizeOf);
}

template <typename ARRAY_TRAITS>
size_t initializeOffsets(std::vector<typename ARRAY_TRAITS::type>& array, size_t bitPosition)
{
    if (ARRAY_TRAITS::IS_BITSIZEOF_CONSTANT)
        return bitPosition + ARRAY_TRAITS::BIT_SIZE * array.size();

    size_t endBitPosition = bitPosition;
    // can't use 'typename ARRAY_TRAITS::type&' because std::vector<bool> returns rvalue
    for (auto&& element : array)
        endBitPosition = ARRAY_TRAITS::initializeOffsets(endBitPosition, element);

    return endBitPosition;
}

template <typename ARRAY_TRAITS, typename OFFSET_INITIALIZER>
size_t initializeOffsetsAligned(std::vector<typename ARRAY_TRAITS::type>& array, size_t bitPosition,
        OFFSET_INITIALIZER offsetInitializer)
{
    size_t endBitPosition = bitPosition;
    size_t index = 0;
    // can't use 'typename ARRAY_TRAITS::type&' because std::vector<bool> returns rvalue
    for (auto&& element : array)
    {
        endBitPosition = alignTo(NUM_BITS_PER_BYTE, endBitPosition);
        offsetInitializer.setOffset(index, bitsToBytes(endBitPosition));
        endBitPosition = ARRAY_TRAITS::initializeOffsets(endBitPosition, element);
        index++;
    }

    return endBitPosition;
}

template <typename ARRAY_TRAITS>
size_t initializeOffsetsAuto(std::vector<typename ARRAY_TRAITS::type>& array, size_t bitPosition)
{
    return initializeOffsets<ARRAY_TRAITS>(array, bitPosition + getBitSizeOfVarUInt64(array.size()));
}

template <typename ARRAY_TRAITS, typename OFFSET_INITIALIZER>
size_t initializeOffsetsAlignedAuto(std::vector<typename ARRAY_TRAITS::type>& array, size_t bitPosition,
        OFFSET_INITIALIZER offsetInitializer)
{
    return initializeOffsetsAligned<ARRAY_TRAITS, OFFSET_INITIALIZER>(array,
            bitPosition + getBitSizeOfVarUInt64(array.size()), offsetInitializer);
}

template <typename ARRAY_TRAITS, typename ELEMENT_FACTORY = nullptr_t>
void read(std::vector<typename ARRAY_TRAITS::type>& array, BitStreamReader& in, size_t size,
        ELEMENT_FACTORY elementFactory = nullptr)
{
    array.clear();
    array.reserve(size);
    for (size_t index = 0; index < size; ++index)
        ARRAY_TRAITS::read(array, in, index, elementFactory);
}

template <typename ARRAY_TRAITS, typename OFFSET_CHECKER, typename ELEMENT_FACTORY = nullptr_t>
void readAligned(std::vector<typename ARRAY_TRAITS::type>& array, BitStreamReader& in, size_t size,
        OFFSET_CHECKER offsetChecker, ELEMENT_FACTORY elementFactory = nullptr)
{
    array.clear();
    array.reserve(size);
    for (size_t index = 0; index < size; ++index)
    {
        in.alignTo(NUM_BITS_PER_BYTE);
        offsetChecker.checkOffset(index, bitsToBytes(in.getBitPosition()));
        ARRAY_TRAITS::read(array, in, index, elementFactory);
    }
}

template <typename ARRAY_TRAITS, typename ELEMENT_FACTORY = nullptr_t>
void readAuto(std::vector<typename ARRAY_TRAITS::type>& array, BitStreamReader& in,
        ELEMENT_FACTORY elementFactory = nullptr)
{
    const uint64_t arraySize = in.readVarUInt64();
    read<ARRAY_TRAITS, ELEMENT_FACTORY>(array, in, convertVarUInt64ToArraySize(arraySize), elementFactory);
}

template <typename ARRAY_TRAITS, typename OFFSET_CHECKER, typename ELEMENT_FACTORY = nullptr_t>
void readAlignedAuto(std::vector<typename ARRAY_TRAITS::type>& array, BitStreamReader& in,
        OFFSET_CHECKER offsetChecker, ELEMENT_FACTORY elementFactory = nullptr)
{
    const uint64_t arraySize = in.readVarUInt64();
    readAligned<ARRAY_TRAITS, OFFSET_CHECKER, ELEMENT_FACTORY>(array, in,
            convertVarUInt64ToArraySize(arraySize), offsetChecker, elementFactory);
}

template <typename ARRAY_TRAITS, typename ELEMENT_FACTORY = nullptr_t>
void readImplicit(std::vector<typename ARRAY_TRAITS::type>& array, BitStreamReader& in,
        ELEMENT_FACTORY elementFactory = nullptr)
{
    array.clear();
    BitStreamReader::BitPosType bitPosition;
    // we must read until end of the stream because we don't know element sizes
    while (true)
    {
        bitPosition = in.getBitPosition();
        const size_t index = array.size();
        try
        {
            ARRAY_TRAITS::read(array, in, index, elementFactory);
        }
        catch (BitStreamException&)
        {
            // set correct end bit position in the stream avoiding padding at the end
            in.setBitPosition(bitPosition);
            break;
        }
    }
}

template <typename ARRAY_TRAITS>
void write(std::vector<typename ARRAY_TRAITS::type>& array, BitStreamWriter& out)
{
    // can't use 'typename ARRAY_TRAITS::type&' because std::vector<bool> returns rvalue
    for (auto&& element : array)
        ARRAY_TRAITS::write(out, element);
}

template <typename ARRAY_TRAITS, typename OFFSET_CHECKER>
void writeAligned(std::vector<typename ARRAY_TRAITS::type>& array, BitStreamWriter& out,
        OFFSET_CHECKER offsetChecker)
{
    size_t index = 0;
    // can't use 'typename ARRAY_TRAITS::type&' because std::vector<bool> returns rvalue
    for (auto&& element : array)
    {
        out.alignTo(NUM_BITS_PER_BYTE);
        offsetChecker.checkOffset(index, bitsToBytes(out.getBitPosition()));
        ARRAY_TRAITS::write(out, element);
        index++;
    }
}

template <typename ARRAY_TRAITS>
void writeAuto(std::vector<typename ARRAY_TRAITS::type>& array, BitStreamWriter& out)
{
    out.writeVarUInt64(static_cast<uint64_t>(array.size()));
    write<ARRAY_TRAITS>(array, out);
}

template <typename ARRAY_TRAITS, typename OFFSET_CHECKER>
void writeAlignedAuto(std::vector<typename ARRAY_TRAITS::type>& array, BitStreamWriter& out,
        OFFSET_CHECKER offsetChecker)
{
    out.writeVarUInt64(static_cast<uint64_t>(array.size()));
    writeAligned<ARRAY_TRAITS, OFFSET_CHECKER>(array, out, offsetChecker);
}

template <size_t NUM_BITS, typename T, typename ENABLED = void>
struct BitFieldArrayTraits;

template <size_t NUM_BITS, typename T>
struct BitFieldArrayTraits<NUM_BITS, T, typename std::enable_if<std::is_same<T, int8_t>::value ||
                                                                std::is_same<T, int16_t>::value ||
                                                                std::is_same<T, int32_t>::value>::type>
{
    typedef T type;

    static size_t bitSizeOf(size_t, type)
    {
        return BIT_SIZE;
    }

    static size_t initializeOffsets(size_t bitPosition, type)
    {
        return bitPosition + BIT_SIZE;
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(static_cast<type>(in.readSignedBits(BIT_SIZE)));
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeSignedBits(value, BIT_SIZE);
    }

    static const bool IS_BITSIZEOF_CONSTANT = true;
    static const size_t BIT_SIZE = NUM_BITS;
};

template <size_t NUM_BITS>
struct BitFieldArrayTraits<NUM_BITS, int64_t>
{
    typedef int64_t type;

    static size_t bitSizeOf(size_t, type)
    {
        return BIT_SIZE;
    }

    static size_t initializeOffsets(size_t bitPosition, type)
    {
        return bitPosition + BIT_SIZE;
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(in.readSignedBits64(BIT_SIZE));
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeSignedBits64(value, BIT_SIZE);
    }

    static const bool IS_BITSIZEOF_CONSTANT = true;
    static const size_t BIT_SIZE = NUM_BITS;
};

template <size_t NUM_BITS, typename T>
struct BitFieldArrayTraits<NUM_BITS, T, typename std::enable_if<std::is_same<T, uint8_t>::value ||
                                                                std::is_same<T, uint16_t>::value ||
                                                                std::is_same<T, uint32_t>::value>::type>
{
    typedef T type;

    static size_t bitSizeOf(size_t, type)
    {
        return BIT_SIZE;
    }

    static size_t initializeOffsets(size_t bitPosition, type)
    {
        return bitPosition + BIT_SIZE;
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(static_cast<type>(in.readBits(BIT_SIZE)));
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeBits(value, BIT_SIZE);
    }

    static const bool IS_BITSIZEOF_CONSTANT = true;
    static const size_t BIT_SIZE = NUM_BITS;
};

template <size_t NUM_BITS>
struct BitFieldArrayTraits<NUM_BITS, uint64_t>
{
    typedef uint64_t type;

    static size_t bitSizeOf(size_t, type)
    {
        return BIT_SIZE;
    }

    static size_t initializeOffsets(size_t bitPosition, type)
    {
        return bitPosition + BIT_SIZE;
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(in.readBits64(BIT_SIZE));
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeBits64(value, BIT_SIZE);
    }

    static const bool IS_BITSIZEOF_CONSTANT = true;
    static const size_t BIT_SIZE = NUM_BITS;
};

template <typename T>
struct StdIntArrayTraits : public BitFieldArrayTraits<sizeof(T) * 8, T>
{
};

template <typename T>
struct VarIntNNArrayTraits;

template <>
struct VarIntNNArrayTraits<int16_t>
{
    typedef int16_t type;

    static size_t bitSizeOf(size_t, type value)
    {
        return getBitSizeOfVarInt16(value);
    }

    static size_t initializeOffsets(size_t bitPosition, type value)
    {
        return bitPosition + bitSizeOf(bitPosition, value);
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(in.readVarInt16());
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeVarInt16(value);
    }

    static const bool IS_BITSIZEOF_CONSTANT = false;
    static const size_t BIT_SIZE = 0;
};

template <>
struct VarIntNNArrayTraits<int32_t>
{
    typedef int32_t type;

    static size_t bitSizeOf(size_t, type value)
    {
        return getBitSizeOfVarInt32(value);
    }

    static size_t initializeOffsets(size_t bitPosition, type value)
    {
        return bitPosition + bitSizeOf(bitPosition, value);
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(in.readVarInt32());
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeVarInt32(value);
    }

    static const bool IS_BITSIZEOF_CONSTANT = false;
    static const size_t BIT_SIZE = 0;
};

template <>
struct VarIntNNArrayTraits<int64_t>
{
    typedef int64_t type;

    static size_t bitSizeOf(size_t, type value)
    {
        return getBitSizeOfVarInt64(value);
    }

    static size_t initializeOffsets(size_t bitPosition, type value)
    {
        return bitPosition + bitSizeOf(bitPosition, value);
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(in.readVarInt64());
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeVarInt64(value);
    }

    static const bool IS_BITSIZEOF_CONSTANT = false;
    static const size_t BIT_SIZE = 0;
};

template <>
struct VarIntNNArrayTraits<uint16_t>
{
    typedef uint16_t type;

    static size_t bitSizeOf(size_t, type value)
    {
        return getBitSizeOfVarUInt16(value);
    }

    static size_t initializeOffsets(size_t bitPosition, type value)
    {
        return bitPosition + bitSizeOf(bitPosition, value);
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(in.readVarUInt16());
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeVarUInt16(value);
    }

    static const bool IS_BITSIZEOF_CONSTANT = false;
    static const size_t BIT_SIZE = 0;
};

template <>
struct VarIntNNArrayTraits<uint32_t>
{
    typedef uint32_t type;

    static size_t bitSizeOf(size_t, type value)
    {
        return getBitSizeOfVarUInt32(value);
    }

    static size_t initializeOffsets(size_t bitPosition, type value)
    {
        return bitPosition + bitSizeOf(bitPosition, value);
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(in.readVarUInt32());
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeVarUInt32(value);
    }

    static const bool IS_BITSIZEOF_CONSTANT = false;
    static const size_t BIT_SIZE = 0;
};

template <>
struct VarIntNNArrayTraits<uint64_t>
{
    typedef uint64_t type;

    static size_t bitSizeOf(size_t, type value)
    {
        return getBitSizeOfVarUInt64(value);
    }

    static size_t initializeOffsets(size_t bitPosition, type value)
    {
        return bitPosition + bitSizeOf(bitPosition, value);
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(in.readVarUInt64());
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeVarUInt64(value);
    }

    static const bool IS_BITSIZEOF_CONSTANT = false;
    static const size_t BIT_SIZE = 0;
};

template <typename T>
struct VarIntArrayTraits;

template <>
struct VarIntArrayTraits<int64_t>
{
    typedef int64_t type;

    static size_t bitSizeOf(size_t , type value)
    {
        return getBitSizeOfVarInt(value);
    }

    static size_t initializeOffsets(size_t bitPosition, type value)
    {
        return bitPosition + bitSizeOf(bitPosition, value);
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(in.readVarInt());
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeVarInt(value);
    }

    static const bool IS_BITSIZEOF_CONSTANT = false;
    static const size_t BIT_SIZE = 0;
};

template <>
struct VarIntArrayTraits<uint64_t>
{
    typedef uint64_t type;

    static size_t bitSizeOf(size_t , type value)
    {
        return getBitSizeOfVarUInt(value);
    }

    static size_t initializeOffsets(size_t bitPosition, type value)
    {
        return bitPosition + bitSizeOf(bitPosition, value);
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(in.readVarUInt());
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeVarUInt(value);
    }

    static const bool IS_BITSIZEOF_CONSTANT = false;
    static const size_t BIT_SIZE = 0;
};

struct Float16ArrayTraits
{
    typedef float type;

    static size_t bitSizeOf(size_t, type)
    {
        return BIT_SIZE;
    }

    static size_t initializeOffsets(size_t bitPosition, type)
    {
        return bitPosition + BIT_SIZE;
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(in.readFloat16());
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeFloat16(value);
    }

    static const bool IS_BITSIZEOF_CONSTANT = true;
    static const size_t BIT_SIZE = 16;
};

struct Float32ArrayTraits
{
    typedef float type;

    static size_t bitSizeOf(size_t, type)
    {
        return BIT_SIZE;
    }

    static size_t initializeOffsets(size_t bitPosition, type)
    {
        return bitPosition + BIT_SIZE;
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(in.readFloat32());
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeFloat32(value);
    }

    static const bool IS_BITSIZEOF_CONSTANT = true;
    static const size_t BIT_SIZE = 32;
};

struct Float64ArrayTraits
{
    typedef double type;

    static size_t bitSizeOf(size_t, type)
    {
        return BIT_SIZE;
    }

    static size_t initializeOffsets(size_t bitPosition, type)
    {
        return bitPosition + BIT_SIZE;
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(in.readFloat64());
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeFloat64(value);
    }

    static const bool IS_BITSIZEOF_CONSTANT = true;
    static const size_t BIT_SIZE = 64;
};

struct BoolArrayTraits
{
    typedef bool type;

    static size_t bitSizeOf(size_t, type)
    {
        return BIT_SIZE;
    }

    static size_t initializeOffsets(size_t bitPosition, type)
    {
        return bitPosition + BIT_SIZE;
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(in.readBool());
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeBool(value);
    }

    static const bool IS_BITSIZEOF_CONSTANT = true;
    static const size_t BIT_SIZE = 1;
};

struct StringArrayTraits
{
    typedef std::string type;

    static size_t bitSizeOf(size_t, const type& value)
    {
        return getBitSizeOfString(value);
    }

    static size_t initializeOffsets(size_t bitPosition, const type& value)
    {
        return bitPosition + bitSizeOf(bitPosition, value);
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t, ELEMENT_FACTORY)
    {
        array.emplace_back(in.readString());
    }

    static void write(BitStreamWriter& out, type value)
    {
        out.writeString(value);
    }

    static const bool IS_BITSIZEOF_CONSTANT = false;
    static const size_t BIT_SIZE = 0;
};

template <typename T>
struct ObjectArrayTraits
{
    typedef T type;

    static size_t bitSizeOf(size_t bitPosition, const type& value)
    {
        return value.bitSizeOf(bitPosition);
    }

    static size_t initializeOffsets(size_t bitPosition, type& value)
    {
        return value.initializeOffsets(bitPosition);
    }

    template <typename ELEMENT_FACTORY>
    static void read(std::vector<type>& array, BitStreamReader& in, size_t index,
            ELEMENT_FACTORY elementFactory)
    {
        elementFactory.create(array, in, index);
    }

    static void write(BitStreamWriter& out, type& value)
    {
        value.write(out, NO_PRE_WRITE_ACTION);
    }

    static const bool IS_BITSIZEOF_CONSTANT = false;
    static const size_t BIT_SIZE = 0;
};

} // namespace zserio

#endif // ZSERIO_ARRAYS_H_INC