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

#include "detail.hpp"

namespace cmdline
{
    template <class T>
    struct default_reader
    {
        T operator()(const std::string &str) { return detail::lexical_cast<T>(str); }
    };

    template <class T>
    struct range_reader
    {
        range_reader(const T &low, const T &high): low(low), high(high) { }
        T operator()(const std::string &s) const
        {
            T ret = default_reader<T>()(s);
            if ( !(ret >= low && ret <= high) ) {
                throw cmdline::cmdline_error("range_error");
            }
            return ret;
        }
    private:
        T low;
        T high;
    };

    template <class T>
    range_reader<T> range(const T &low, const T &high) { return range_reader<T>(low, high); }

    template <class T>
    struct one_of_reader
    {
        T operator()(const std::string &s)
        {
            T ret=default_reader<T>()(s);
            if (std::find(alt.begin(), alt.end(), ret) == alt.end())
                throw cmdline_error("");
            return ret;
        }
        void add(const T &v){ alt.push_back(v); }
    private:
        std::vector<T> alt;
    };
}
