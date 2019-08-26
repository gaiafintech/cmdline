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
#include <vector>
#include <map>
#include <iostream>

#include "detail.hpp"
#include "error.hpp"
#include "options.hpp"
#include "reader.hpp"

namespace cmdline
{
    template <class T>
    struct of_helper
    {
        void expand(T arg) { r.add(arg); }
        template <class... Args>
        void of(const Args&... args) { int a[] = { (expand(args), 0)... }; }
        one_of_reader<T> r;
    };

    template <class T, class... Args>
    one_of_reader<T> of(const Args&... args)
    {
        of_helper<T> helper;
        helper.of(args...);
        return helper.r;
    }

    class parser
    {
    public:
        parser() = default;
        ~parser()
        {
            for(auto &pair : options_)
                delete pair.second;
        }

        void add(const std::string &name, char short_name = 0, const std::string &desc = "")
        {
            if (options_.count(name))
                throw cmdline::cmdline_error("multiple definition: " + name);

            options_[name] = new option::option_without_value(name, short_name, desc);
            ordered_.push_back(options_[name]);
        }

        template <class T>
        void add(const std::string &name, char short_name = 0,
                 const std::string &desc = "", bool need = true, const T def = T())
        {
            add(name, short_name, desc, need, def, default_reader<T>());
        }

        template <class T, class F>
        void add(const std::string &name, char short_name=0,
                 const std::string &desc = "", bool need = true, const T def = T(), F reader = F())
        {
            if (options_.count(name))
                throw cmdline_error("multiple definition: "+name);
            options_[name]=new option::option_with_value_with_reader<T, F>(name, short_name, need, def, desc, reader);
            ordered_.push_back(options_[name]);
        }

        void footer(const std::string &f) { footer_ = f; }
        void set_program_name(const std::string &name) { program_name_ = name; }

        bool exist(const std::string &name) const
        {
            if (options_.count(name) == 0)
                throw cmdline_error("there is no flag: --" + name);
            return options_.find(name)->second->has_set();
        }

        template <class T>
        const T &get(const std::string &name) const
        {
            if (options_.count(name)==0)
                throw cmdline_error("there is no flag: --" + name);

            auto opt = options_.find(name)->second;
            auto p = dynamic_cast<const option::option_with_value<T>*>(opt);
            if (!p)
                throw cmdline_error("type mismatch flag '" + name + "'");
            return p->get();
        }

        const std::vector<std::string> &rest() const { return others_; }

        bool parse(const std::string &arg)
        {
            std::vector<std::string> args;

            std::string buf;
            bool in_quote=false;
            for (std::string::size_type i=0; i<arg.length(); i++)
            {
                if (arg[i] == '\"') {
                    in_quote=!in_quote;
                    continue;
                }

                if ( arg[i] == ' ' && !in_quote ) {
                    args.push_back(buf);
                    buf = "";
                    continue;
                }

                if (arg[i] == '\\') {
                    i++;
                    if (i >= arg.length()) {
                        errors_.emplace_back("unexpected occurrence of '\\' at end of string");
                        return false;
                    }
                }

                buf+=arg[i];
            }

            if (in_quote){
                errors_.emplace_back("quote is not closed");
                return false;
            }

            if (buf.length()>0)
                args.push_back(buf);

            for(auto &var0 : args)
                std::cout << "\"" << var0 <<"\"" << std::endl;

            return parse(args);
        }

        bool parse(const std::vector<std::string> &args)
        {
            int argc = static_cast<int>(args.size());
            std::vector<const char*> argv(argc);

            for (int i=0; i<argc; i++)
                argv[i] = args[i].c_str();

            return parse(argc, &argv[0]);
        }

        bool parse(int argc, const char * const argv[])
        {
            errors_.clear();
            others_.clear();

            if (argc<1){
                errors_.emplace_back("argument number must be longer than 0");
                return false;
            }

            if (program_name_.empty())
                program_name_ = argv[0];

            std::map<char, std::string> lookup;
            for(auto &p : options_)
            {
                if (p.first.empty())
                    continue;

                char initial = p.second->short_name();
                if (initial)
                {
                    if (lookup.count(initial) > 0) {
                        lookup[initial] = "";
                        errors_.emplace_back(std::string("short option '") + initial+ "' is ambiguous");
                        return false;
                    } else {
                        lookup[initial] = p.first;
                    }
                }
            }

            for (int i=1; i<argc; i++)
            {
                if (strncmp(argv[i], "--", 2) == 0)
                {
                    const char *p = strchr(argv[i]+2, '=');
                    if (p){
                        std::string name(argv[i] + 2, p);
                        std::string val(p + 1);
                        set_option(name, val);
                    } else {
                        std::string name(argv[i] + 2);
                        if (options_.count(name) == 0) {
                            errors_.emplace_back("undefined option: --" + name);
                            continue;
                        }
                        if (options_[name]->has_value())
                        {
                            if (i + 1 >= argc) {
                                errors_.emplace_back("option needs value: --" + name);
                                continue;
                            } else {
                                i++;
                                set_option(name, argv[i]);
                            }
                        } else{
                            set_option(name);
                        }
                    }
                }
                else if (strncmp(argv[i], "-", 1) == 0)
                {
                    if (!argv[i][1])
                        continue;

                    char last = argv[i][1];
                    for (int j = 2; argv[i][j]; j++)
                    {
                        last=argv[i][j];
                        if (lookup.count(argv[i][j-1])==0) {
                            errors_.emplace_back(std::string("undefined short option: -") + (argv[i][j - 1]) );
                            continue;
                        }
                        if (lookup[argv[i][j-1]].empty()) {
                            errors_.emplace_back(std::string("ambiguous short option: -") + (argv[i][j-1]) );
                            continue;
                        }
                        set_option(lookup[argv[i][j-1]]);
                    }

                    if (lookup.count(last) == 0) {
                        errors_.emplace_back(std::string("undefined short option: -") + last);
                        continue;
                    }

                    if (lookup[last].empty()) {
                        errors_.emplace_back(std::string("ambiguous short option: -") + last);
                        continue;
                    }

                    if (i+1<argc && options_[lookup[last]]->has_value()) {
                        set_option(lookup[last], argv[i+1]);
                        i++;
                    } else {
                        set_option(lookup[last]);
                    }
                }
                else{
                    others_.emplace_back(argv[i]);
                }
            }

            for (auto &p : options_) {
                if (!p.second->valid())
                    errors_.push_back("need option: --" + std::string(p.first));
            }

            return errors_.empty();
        }

        void parse_check(const std::string &arg) {
            if (!options_.count("help"))
                add("help", '?', "print this message");
            check(0, parse(arg));
        }

        void parse_check(const std::vector<std::string> &args) {
            if (!options_.count("help"))
                add("help", '?', "print this message");
            check(args.size(), parse(args));
        }

        void parse_check(int argc, char *argv[]) {
            if (!options_.count("help"))
                add("help", '?', "print this message");
            check(argc, parse(argc, argv));
        }

        std::string error() const {
            return errors_.empty() ? "" : errors_[0];
        }

        std::string error_full() const
        {
            std::ostringstream oss;
            for (auto &err : errors_)
                oss << err << std::endl;
            return oss.str();
        }

        std::string usage() const
        {
            std::ostringstream oss;
            oss << "usage: " << program_name_ << " ";
            for (auto &o : ordered_)
            {
                if (o->must())
                    oss << o->short_description() << " ";
            }

            oss << "[options] ... " << std::endl;
            oss << "options:" << std::endl;

            size_t max_width=0;
            for (auto &o : ordered_)
                max_width = std::max(max_width, o->name().length());

            for (auto &o : ordered_)
            {
                if (o->short_name())
                    oss << "  -" << o->short_name() << ", ";
                else
                    oss << "      ";

                oss << "--" << o->name();
                for (size_t j = o->name().length(); j < max_width + 4; j++)
                    oss<<' ';
                oss << o->description() << std::endl;
            }

            oss << std::endl;
            oss << footer_ << std::endl;

            return oss.str();
        }

    private:
        void check(int argc, bool ok)
        {
            if ( (argc == 1 && !ok) || exist("help") ) {
                std::cerr << usage();
                exit(0);
            }

            if (!ok) {
                std::cerr << error() << std::endl << usage();
                exit(1);
            }
        }

        void set_option(const std::string &name)
        {
            if (options_.count(name) == 0) {
                errors_.emplace_back("undefined option: --" + name);
                return;
            }
            if (!options_[name]->set()) {
                errors_.emplace_back("option needs value: --" + name);
                return;
            }
        }

        void set_option(const std::string &name, const std::string &value)
        {
            if (options_.count(name) == 0) {
                errors_.emplace_back("undefined option: --" + name);
                return;
            }
            if (!options_[name]->set(value)) {
                errors_.emplace_back("option value is invalid: --"+name+"="+value);
                return;
            }
        }

    private:
        std::string footer_;
        std::string program_name_;
        std::map<std::string, option::option_base*> options_;
        std::vector<option::option_base*> ordered_;
        std::vector<std::string> others_;
        std::vector<std::string> errors_;
    };
}