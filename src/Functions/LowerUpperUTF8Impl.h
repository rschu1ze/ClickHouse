#pragma once
#include <Columns/ColumnString.h>
#include <Functions/LowerUpperImpl.h>
#include <base/defines.h>
#include <Poco/UTF8Encoding.h>
#include <Common/StringUtils.h>
#include <Common/UTF8Helpers.h>

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#if USE_ICU
#include <unicode/unistr.h>
#endif

namespace DB
{

namespace ErrorCodes
{
    extern const int BAD_ARGUMENTS;
}

template <char not_case_lower_bound, char not_case_upper_bound, bool upper>
struct LowerUpperUTF8Impl
{
    static void vector(
        const ColumnString::Chars & data,
        const ColumnString::Offsets & offsets,
        ColumnString::Chars & res_data,
        ColumnString::Offsets & res_offsets)
    {
        if (data.empty())
            return;

        bool all_ascii = isAllASCII(data.data(), data.size());
        if (all_ascii)
        {
            LowerUpperImpl<not_case_lower_bound, not_case_upper_bound>::vector(data, offsets, res_data, res_offsets);
            return;
        }

        res_data.resize(data.size());
        res_offsets.resize_exact(offsets.size());
        size_t curr_offset = 0;
        for (size_t i = 0; i < offsets.size(); ++i)
        {
            const auto * pos =  reinterpret_cast<const char *>(&data[offsets[i-1]]);
            size_t size = offsets[i] - offsets[i-1];

            icu::UnicodeString input(pos, static_cast<int32_t>(size), "UTF-8");
            if constexpr (upper)
                input.toUpper();
            else
                input.toLower();

            String output;
            input.toUTF8String(output);

            res_data.resize(curr_offset + output.size() + 1);
            memcpySmallAllowReadWriteOverflow15(&res_data[curr_offset], output.data(), output.size());
            res_data[curr_offset + output.size()] = 0;

            curr_offset += output.size() + 1;
            res_offsets[i] = curr_offset;
        }
    }

    static void vectorFixed(const ColumnString::Chars &, size_t, ColumnString::Chars &)
    {
        throw Exception(ErrorCodes::BAD_ARGUMENTS, "Functions lowerUTF8 and upperUTF8 cannot work with FixedString argument");
    }
};

}
