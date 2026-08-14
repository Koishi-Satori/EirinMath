#ifndef EIRIN_MATH_IO_FORMAT_HPP
#define EIRIN_MATH_IO_FORMAT_HPP

#pragma once

#include <version>
#include <string>
#include <algorithm>
#include "../fixed.hpp"
#ifdef __cpp_lib_format
#    define EIRIN_HAS_LIB_FORMAT __cpp_lib_format
#    include <format>
#endif

namespace eirin::io
{
namespace detail
{
    inline constexpr char digit_map_lower[] = "0123456789abcdef";
    inline constexpr char digit_map_upper[] = "0123456789ABCDEF";
} // namespace detail

template <typename CharT, typename T, typename I, unsigned int F, bool R>
class fixed_num_formatter
{
public:
    using char_type = CharT;

    // Format-spec state
    struct state_type
    {
        using char_type = CharT;

        char align_mode = '\0'; // '<' | '>' | '^' | '\0' -> default (right)
        char sign_mode = '-'; // '-' | '+' | ' '
        bool alt_form = false;
        bool zero_pad = false;
        int width_val = 0;
        int precision = -1; // -1 means "not specified"
        char type = 'g'; // g G f F d x X o b B ?
        char_type fill_ch = ' ';
    };

    using fixed_num_type = fixed_num<T, I, F, R>;

    constexpr void set_state(const state_type& st) noexcept
    {
        m_state = st;
    }

    constexpr const state_type& get_state() const noexcept
    {
        return m_state;
    }

    template <typename OutputIt>
    OutputIt format_to(OutputIt out, const fixed_num_type& fp) const
    {
        // determine mode
        bool int_only = false;
        bool uppercase = false;
        int base = 10;

        switch(m_state.type)
        {
        case 'X': uppercase = true; [[fallthrough]];
        case 'x':
            int_only = true;
            base = 16;
            break;
        case 'd':
            int_only = true;
            base = 10;
            break;
        case 'o':
            int_only = true;
            base = 8;
            break;
        case 'B': uppercase = true; [[fallthrough]];
        case 'b':
            int_only = true;
            base = 2;
            break;
        case '?':
            return format_debug_to(out, fp);
        default: // g, G, f, F
            if(m_state.type == 'G' || m_state.type == 'F')
                uppercase = true;
            break;
        }

        // sign & integer part
        bool neg = signbit(fp);
        T int_val = fp.integral_part(); // absolute value

        // digit string
        auto digit_str = int_to_string(int_val, base, uppercase);

        // prefix: sign + alt-prefix
        std::basic_string<CharT> prefix;
        append_sign(prefix, neg);
        if(m_state.alt_form && int_only && base != 10)
            append_alt_prefix(prefix, base, uppercase);

        // The "body" string
        std::basic_string<CharT> body;

        if(int_only)
        {
            if(m_state.zero_pad && m_state.width_val > 0)
            {
                std::size_t total = prefix.size() + digit_str.size();
                if(static_cast<int>(total) < m_state.width_val)
                {
                    std::size_t zeros = static_cast<std::size_t>(m_state.width_val) - total;
                    body = prefix + std::basic_string<CharT>(zeros, static_cast<CharT>('0')) + digit_str;
                }
                else
                {
                    body = prefix + digit_str;
                }
            }
            else
            {
                body = prefix + digit_str;
            }
            return output_aligned(out, body);
        }

        body = prefix + digit_str;
        {
            // fractional part
            T frac_val = fp.fractional_part();
            if constexpr(eirin::detail::is_signed_v<T>)
            {
                if(frac_val < 0)
                    frac_val = -frac_val;
            }

            bool has_frac = (frac_val != 0);
            int used_prec = 0;

            if(has_frac)
            {
                body.push_back(static_cast<CharT>('.'));

                const auto& digits = uppercase ?
                                         detail::digit_map_upper :
                                         detail::digit_map_lower;

                int eff_prec = (m_state.precision < 0) ? 6 : m_state.precision;

                T frac_bits = frac_val;
                for(unsigned int i = 0; i < F && used_prec < eff_prec; ++i)
                {
                    if(frac_bits == 0) break;

                    T shifted = frac_bits * static_cast<T>(10);
                    T digit = shifted >> F;
                    frac_bits = shifted & ((static_cast<T>(1) << F) - static_cast<T>(1));

                    body.push_back(static_cast<CharT>(digits[static_cast<size_t>(digit)]));
                    ++used_prec;
                }
            }

            // trailing zeros for fixed-point types (f/F)
            if(m_state.type == 'f' || m_state.type == 'F')
            {
                int eff_prec = (m_state.precision < 0) ? 6 : m_state.precision;
                while(used_prec < eff_prec)
                {
                    if(used_prec == 0)
                        body.push_back(static_cast<CharT>('.'));
                    body.push_back(static_cast<CharT>('0'));
                    ++used_prec;
                }
            }
        }

        return output_aligned(out, body);
    }

private:
    state_type m_state;

    // format_debug - "?" type: show internal (raw) value in hex
    template <typename OutputIt>
    OutputIt format_debug_to(OutputIt out, const fixed_num_type& fp) const
    {
        bool upper = false;

        T int_val = fp.internal_value(); // raw bits
        bool neg = false;

        using unsigned_type = std::make_unsigned_t<T>;
        unsigned_type uval;

        if constexpr(eirin::detail::is_signed_v<T>)
        {
            if(int_val < 0)
            {
                neg = true;
                uval = static_cast<unsigned_type>(-(int_val + 1)) + unsigned_type(1);
            }
            else
            {
                uval = static_cast<unsigned_type>(int_val);
            }
        }
        else
        {
            uval = static_cast<unsigned_type>(int_val);
        }

        std::basic_string<CharT> body;
        append_sign(body, neg);

        if(m_state.alt_form)
            append_alt_prefix(body, 16, upper);

        const auto& digits = detail::digit_map_lower;

        if(uval == 0)
        {
            body.push_back(static_cast<CharT>('0'));
        }
        else
        {
            std::basic_string<CharT> rev;
            while(uval > 0)
            {
                rev.push_back(static_cast<CharT>(digits[uval % 16U]));
                uval /= 16U;
            }
            std::reverse(rev.begin(), rev.end());
            body += rev;
        }

        return output_aligned(out, body);
    }

    // Convert an integer to a string in the given base
    template <typename ValType>
    static std::basic_string<CharT> int_to_string(ValType ival, int base, bool uppercase)
    {
        const auto& digits = uppercase ? detail::digit_map_upper : detail::digit_map_lower;
        std::basic_string<CharT> result;

        if(ival == 0)
        {
            result.push_back(static_cast<CharT>('0'));
            return result;
        }

        using unsigned_type = std::make_unsigned_t<ValType>;
        unsigned_type uval;
        if constexpr(eirin::detail::is_signed_v<ValType>)
        {
            uval = ival < 0 ? static_cast<unsigned_type>(-(ival + 1)) + unsigned_type(1) : static_cast<unsigned_type>(ival);
        }
        else
        {
            uval = static_cast<unsigned_type>(ival);
        }

        while(uval > 0)
        {
            result.push_back(static_cast<CharT>(digits[uval % static_cast<unsigned_type>(base)]));
            uval /= static_cast<unsigned_type>(base);
        }
        std::reverse(result.begin(), result.end());
        return result;
    }

    // Get the width of alternate-form prefix for a given base
    static constexpr std::size_t alt_prefix_width(int base) noexcept
    {
        switch(base)
        {
        default:
        case 10: return 0;
        case 8: return 1;
        case 2:
        case 16: return 2;
        }
    }

    void append_sign(std::basic_string<CharT>& s, bool neg) const
    {
        switch(m_state.sign_mode)
        {
        default:
        case '-':
            if(neg) s.push_back(static_cast<CharT>('-'));
            break;
        case '+':
            s.push_back(static_cast<CharT>(neg ? '-' : '+'));
            break;
        case ' ':
            s.push_back(static_cast<CharT>(neg ? '-' : ' '));
            break;
        }
    }

    void append_alt_prefix(std::basic_string<CharT>& s, int base, bool upper) const
    {
        switch(base)
        {
        case 2:
            s.push_back(static_cast<CharT>('0'));
            s.push_back(static_cast<CharT>(upper ? 'B' : 'b'));
            break;
        case 8:
            s.push_back(static_cast<CharT>('0'));
            break;
        case 16:
            s.push_back(static_cast<CharT>('0'));
            s.push_back(static_cast<CharT>(upper ? 'X' : 'x'));
            break;
        default:
            break;
        }
    }

    template <typename OutputIt>
    OutputIt output_aligned(OutputIt out, const std::basic_string<CharT>& body) const
    {
        auto sz = body.size();
        if(m_state.width_val <= 0 || static_cast<std::size_t>(m_state.width_val) <= sz)
            return std::copy(body.begin(), body.end(), out);

        std::size_t pad_cnt = static_cast<std::size_t>(m_state.width_val) - sz;
        CharT pad_ch = m_state.zero_pad ? static_cast<CharT>('0') : m_state.fill_ch;

        char eff_align = (m_state.align_mode == '\0') ? '>' : m_state.align_mode;

        switch(eff_align)
        {
        case '<':
            out = std::copy(body.begin(), body.end(), out);
            out = std::fill_n(out, pad_cnt, pad_ch);
            break;

        case '^':
            {
                std::size_t left = pad_cnt / 2;
                std::size_t right = pad_cnt - left;
                out = std::fill_n(out, left, pad_ch);
                out = std::copy(body.begin(), body.end(), out);
                out = std::fill_n(out, right, pad_ch);
                break;
            }

        case '>':
        default:
            out = std::fill_n(out, pad_cnt, pad_ch);
            out = std::copy(body.begin(), body.end(), out);
            break;
        }

        return out;
    }
};
} // namespace eirin::io

#ifdef EIRIN_HAS_LIB_FORMAT

template <typename T, typename I, unsigned int F, bool R, typename CharT>
struct std::formatter<eirin::fixed_num<T, I, F, R>, CharT>
{
    using my_base_fmt = eirin::io::fixed_num_formatter<CharT, T, I, F, R>;

public:
    using fixed_num_type = eirin::fixed_num<T, I, F, R>;

    constexpr auto parse(std::basic_format_parse_context<CharT>& ctx)
        -> typename std::basic_format_parse_context<CharT>::iterator
    {
        using state_type = typename my_base_fmt::state_type;
        state_type st;

        auto it = ctx.begin();
        auto end = ctx.end();

        if(it == end)
        {
            m_fmt.set_state(st);
            return it;
        }

        {
            auto p = it;
            while(p != end && *p >= '0' && *p <= '9')
                ++p;
            if(p != end && *p == ':')
                it = p + 1;
        }

        if(it == end)
        {
            m_fmt.set_state(st);
            return it;
        }

        // [[fill]align]
        {
            auto nxt = it;
            ++nxt;
            if(nxt != end && (*nxt == '<' || *nxt == '>' || *nxt == '^'))
            {
                st.fill_ch = *it;
                st.align_mode = *nxt;
                it += 2;
            }
            else if(*it == '<' || *it == '>' || *it == '^')
            {
                st.align_mode = *it;
                ++it;
            }
        }

        // [sign]
        if(it != end && (*it == '+' || *it == '-' || *it == ' '))
        {
            st.sign_mode = *it;
            ++it;
        }

        // [#]
        if(it != end && *it == '#')
        {
            st.alt_form = true;
            ++it;
        }

        // [0]
        if(it != end && *it == '0')
        {
            st.zero_pad = true;
            ++it;
        }

        // [width]
        if(it != end && *it >= '0' && *it <= '9')
        {
            st.width_val = 0;
            while(it != end && *it >= '0' && *it <= '9')
            {
                st.width_val = st.width_val * 10 + static_cast<int>(*it - '0');
                ++it;
            }
        }

        // [.precision]
        if(it != end && *it == '.')
        {
            ++it;
            st.precision = 0;
            while(it != end && *it >= '0' && *it <= '9')
            {
                st.precision = st.precision * 10 + static_cast<int>(*it - '0');
                ++it;
            }
        }

        // [type]
        if(it != end)
        {
            switch(*it)
            {
            case 'g':
            case 'G':
            case 'f':
            case 'F':
            case 'd':
            case 'x':
            case 'X':
            case 'o':
            case 'b':
            case 'B':
            case '?':
                st.type = *it;
                ++it;
                break;
            case '}':
                break;
            default:
                EIRIN_THROW_EXCEPTION(std::format_error, "invalid format type");
            }
        }

        m_fmt.set_state(st);
        return it;
    }

    template <typename FormatContext>
    auto format(const fixed_num_type& fp, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        return m_fmt.format_to(ctx.out(), fp);
    }

private:
    my_base_fmt m_fmt;
};

#endif

#endif
