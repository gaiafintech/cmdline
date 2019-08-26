/*
  Copyright (c) 2009-2019, Hideyuki Tanaka, Joel
  All rights reserved.
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  * Neither the name of the <organization> nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.
  THIS SOFTWARE IS PROVIDED BY <copyright holder> ''AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <string>
#include <sstream>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

#include "error.hpp"

namespace cmdline { namespace detail {
    template <typename Target, typename Source, bool Same>
    class lexical_cast_t
    {
    public:
        static Target cast(const Source &arg)
        {
            Target ret;
            std::stringstream ss;
            if (!(ss<<arg && ss>>ret && ss.eof()))
                throw std::bad_cast();
            return ret;
        }
    };

    template <typename Target, typename Source>
    class lexical_cast_t<Target, Source, true>
    {
    public:
        static Target cast(const Source &arg) { return arg; }
    };

    template <typename Source>
    class lexical_cast_t<std::string, Source, false>
    {
    public:
        static std::string cast(const Source &arg)
        {
            std::ostringstream ss;
            ss<<arg;
            return ss.str();
        }
    };

    template <typename Target>
    class lexical_cast_t<Target, std::string, false>
    {
    public:
        static Target cast(const std::string &arg)
        {
            Target ret;
            std::istringstream ss(arg);
            if (!(ss>>ret && ss.eof()))
                throw std::bad_cast();
            return ret;
        }
    };

    template <typename T1, typename T2>
    struct is_same { static const bool value = false; };

    template <typename T>
    struct is_same<T, T>{ static const bool value = true; };

    template<typename Target, typename Source>
    Target lexical_cast(const Source &arg)
    {
        return lexical_cast_t<Target, Source, detail::is_same<Target, Source>::value>::cast(arg);
    }

    static inline std::string demangle(const std::string &name)
    {
#ifdef __GNUC__
        int status=0;
        // see
        // https://gcc.gnu.org/onlinedocs/libstdc++/libstdc++-html-USERS-4.3/a01696.html
        char *p=abi::__cxa_demangle(name.c_str(), nullptr, nullptr, &status);
        if(p) {
            std::string ret(p);
            free(p);
            return ret;
        } else {
            switch (status)
            {
                case -1:
                    throw cmdline::cmdline_error("__cxa_demangle: A memory allocation failiure occurred");
                    break;
                case -2:
                    throw cmdline::cmdline_error("__cxa_demangle: mangled_name is not a valid name under the C++ ABI mangling rules.");
                default:
                    throw cmdline::cmdline_error("__cxa_demangle: unknown demangle error");
            }
        }
#else
        return name;
#endif
    }

    template <class T>
    std::string readable_typename()
    {
        return demangle(typeid(T).name());
    }

    template <class T>
    std::string default_value(T def)
    {
        return detail::lexical_cast<std::string>(def);
    }

    template <>
    inline std::string readable_typename<std::string>()
    {
        return "string";
    }
} }