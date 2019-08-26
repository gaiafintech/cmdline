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

namespace cmdline { namespace option {
    class option_base
    {
    public:
        virtual ~option_base() = default;
        virtual bool has_value() const = 0;
        virtual bool set() = 0;
        virtual bool set(const std::string &value) = 0;
        virtual bool has_set() const = 0;
        virtual bool valid() const = 0;
        virtual bool must() const = 0;
        virtual const std::string &name() const = 0;
        virtual char short_name() const = 0;
        virtual const std::string &description() const = 0;
        virtual std::string short_description() const = 0;
    };

    class option_without_value : public option_base
    {
    public:
        option_without_value(const std::string &name,
                             char short_name,
                             const std::string &desc)
        {
            name_ = name;
            short_name_ = short_name;
            desc_ = desc;
            has_ = false;
        }

        ~option_without_value() override = default;
        bool set() override
        {
            has_ = true;
            return true;
        }
        bool has_value() const override { return false; }
        bool set(const std::string &) override { return false; }
        bool has_set() const override { return has_; }
        bool valid() const override {return true; }
        bool must() const override { return false; }
        const std::string &name() const override { return name_; }
        char short_name() const override { return short_name_; }
        const std::string &description() const override { return desc_; }
        std::string short_description() const override { return "--" + name_; }

    protected:
        std::string name_;
        char short_name_;
        std::string desc_;
        bool has_;
    };

    template <class T>
    class option_with_value : public option_base
    {
    public:
        option_with_value(const std::string &name, char short_name,
                          bool need, const T &def, const std::string &desc)
        {

            name_ = name;
            short_name_ = short_name;
            need_ = need;
            has_ = false;
            def_ = def;
            actual_ = def;
            this->desc_ = full_description(desc);
        }
        ~option_with_value() override = default;
        const T &get() const { return actual_; }

        bool has_value() const override { return true; }
        bool set() override { return false;}
        bool set(const std::string &value) override
        {
            try{
                actual_ = read(value);
                has_ = true;
            } catch(const std::exception &e) {
                return false;
            }
            return true;
        }

        bool has_set() const override { return has_; }
        bool valid() const override { return !(need_ && !has_); }
        bool must() const override { return need_; }
        const std::string &name() const override { return name_; }
        char short_name() const override { return short_name_; }
        const std::string &description() const override { return desc_; }
        std::string short_description() const override
        {
            return "--" + name_ + "=" + detail::readable_typename<T>();
        }

    protected:
        std::string full_description(const std::string &desc)
        {
            return desc+ " (" +detail::readable_typename<T>() +
                   (need_ ? "" : " [=" + detail::default_value<T>(def_) + "]" ) +")";
        }

        virtual T read(const std::string &s) = 0;

    private:
        std::string name_;
        char short_name_;
        bool need_;
        std::string desc_;

        bool has_;
        T def_;
        T actual_;
    };

    template <class T, class F>
    class option_with_value_with_reader : public option_with_value<T>
    {
    public:
        option_with_value_with_reader(const std::string &name, char short_name,
                                      bool need, const T def, const std::string &desc, F reader)
                : option_with_value<T>(name, short_name, need, def, desc), reader(reader) {
        }

    protected:
        T read(const std::string &s) override { return reader(s); }

    private:
        F reader;
    };
} }