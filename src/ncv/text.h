#ifndef NANOCV_TEXT_H
#define NANOCV_TEXT_H

#include "types.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace ncv
{
        namespace text
        {
                using namespace boost::algorithm;

                // align a string to fill the given size
                string_t resize(const string_t& str, size_t size, align alignment = align::left);

                // string cast for built-in types
                template
                <
                        typename tvalue
                >
                string_t to_string(tvalue value)
                {
                        return std::to_string(value);
                }
                template <>
                inline string_t to_string(string_t value)
                {
                        return value;
                }

                template
                <
                        typename tvalue
                >
                tvalue from_string(const string_t& str)
                {
                        return boost::lexical_cast<tvalue>(str);
                }

                // strinct cast for enumerations
                template <>
                inline string_t to_string(protocol type)
                {
                        switch (type)
                        {
                        case protocol::train:           return "train";
                        case protocol::test:            return "test";
                        default:                        return "train";
                        }
                }

                template <>
                inline protocol from_string<protocol>(const string_t& string)
                {
                        if (string == "train")          return protocol::train;
                        if (string == "test")           return protocol::test;
                        throw std::invalid_argument("invalid protocol type <" + string + ">!");
                        return protocol::train;
                }

                template <>
                inline string_t to_string(color_mode mode)
                {
                        switch (mode)
                        {
                        case color_mode::luma:          return "luma";
                        case color_mode::rgba:          return "rgba";
                        default:                        return "luma";
                        }
                }

                template <>
                inline color_mode from_string<color_mode>(const string_t& string)
                {
                        if (string == "luma")           return color_mode::luma;
                        if (string == "rgba")           return color_mode::rgba;
                        throw std::invalid_argument("invalid color mode <" + string + ">!");
                        return color_mode::luma;
                }

                template <>
                inline string_t to_string(channel dtype)
                {
                        switch (dtype)
                        {
                        case channel::red:              return "red";
                        case channel::green:            return "green";
                        case channel::blue:             return "blue";
                        case channel::luma:             return "luma";
                        case channel::cielab_l:         return "cielab_l";
                        case channel::cielab_a:         return "cielab_a";
                        case channel::cielab_b:         return "cielab_b";
                        default:                        return "luma";
                        }
                }

                template <>
                inline channel from_string<channel>(const string_t& string)
                {
                        if (string == "red")            return channel::red;
                        if (string == "green")          return channel::green;
                        if (string == "blue")           return channel::blue;
                        if (string == "luma")           return channel::luma;
                        if (string == "cielab_l")       return channel::cielab_l;
                        if (string == "cielab_a")       return channel::cielab_a;
                        if (string == "cielab_b")       return channel::cielab_b;
                        throw std::invalid_argument("Invalid channel type <" + string + ">!");
                        return channel::luma;
                }

                // compact a list of values into a string using the given glue string
                template
                <
                        typename tvalue
                >
                string_t concatenate(const std::vector<tvalue>& values, const string_t& glue = ",")
                {
                        string_t ret;
                        std::for_each(std::begin(values), std::end(values), [&] (const tvalue& val)
                        {
                                ret += to_string(val) + glue;
                        });

                        return ret.empty() ? ret : ret.substr(0, ret.size() - glue.size());
                }

                // decode parameter by name: [name1=value1[,name2=value2[...]]
                // the default value is returned if the parameter cannot be found or is invalid.
                template
                <
                        class tvalue
                >
                tvalue from_params(const string_t& params, const string_t& param_name, tvalue default_value)
                {
                        strings_t tokens, dual;

                        text::split(tokens, params, text::is_any_of(","));
                        for (size_t i = 0; i < tokens.size(); i ++)
                        {
                                text::split(dual, tokens[i], text::is_any_of("="));
                                if (dual.size() == 2 && dual[0] == param_name)
                                {
                                        string_t value = dual[1];
                                        for (   size_t j = i + 1;
                                                j < tokens.size() && tokens[j].find("=") == string_t::npos;
                                                j ++)
                                        {
                                                value += "," + tokens[j];
                                        }

                                        try
                                        {
                                                return from_string<tvalue>(value);
                                        }
                                        catch (std::exception&)
                                        {
                                                return default_value;
                                        }
                                }
                        }

                        return default_value;
                }
        }
}

#endif // NANOCV_TEXT_H
