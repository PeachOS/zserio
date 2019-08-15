/**
 * Automatically generated by Zserio C++ extension version 1.2.0.
 */

#ifndef ARRAY_H
#define ARRAY_H

#include <vector>

#include <zserio/BitStreamReader.h>
#include <zserio/BitStreamWriter.h>
#include <zserio/BitFieldUtil.h>
#include <zserio/CppRuntimeException.h>
#include <zserio/StringConvertUtil.h>
#include <zserio/PreWriteAction.h>
#include <zserio/Types.h>

class Array
{
public:
    // Consider defaulting this ctor and giving m_isInitialized default value false
    Array() noexcept;
    Array(zserio::BitStreamReader& in,
            uint32_t _size);

    // new in cpp11
    // MBition: Why is this explicit? No need
    template <typename ZSERIO_T_values>
    explicit Array(uint32_t _size, ZSERIO_T_values&& _values)
    :   m_isInitialized(true), m_size_(_size), m_values_(std::forward<ZSERIO_T_values>(_values))
    {
    }

    // new in cpp11
    // MBition: Why explicit? Why is the definition in header as it's not templated.
    explicit Array(uint32_t _size, std::initializer_list<int32_t> _values)
    :   m_isInitialized(true), m_size_(_size), m_values_(_values)
    {
    }

    // new in cpp11
    Array(Array&& other) = default;
    Array& operator=(Array&& other) = default;

    Array(const Array& other);
    Array& operator=(const Array& other);

    void initialize(
            uint32_t size);

    uint32_t getSize() const noexcept;

    std::vector<int32_t>& getValues() noexcept;
    const std::vector<int32_t>& getValues() const noexcept;
    void setValues(const std::vector<int32_t>& _values);
    void setValues(std::vector<int32_t>&& _values);
    void setValues(std::initializer_list<int32_t> _values); // do we need also this?

    size_t bitSizeOf(size_t bitPosition = 0) const;
    size_t initializeOffsets(size_t bitPosition);

    // MBition: should be noexcept
    bool operator==(const Array& other) const;
    int hashCode() const;

    void read(zserio::BitStreamReader& in);
    void write(zserio::BitStreamWriter& out,
            zserio::PreWriteAction preWriteAction = zserio::ALL_PRE_WRITE_ACTIONS);

private:
    uint32_t m_size_;
    bool m_isInitialized;
    std::vector<int32_t> m_values_;
};

#endif // ARRAY_H
