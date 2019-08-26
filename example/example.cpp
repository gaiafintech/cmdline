/*
  Copyright (c) 2009, Hideyuki Tanaka
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

#include <cmdline/cmdline.hpp>

#include <string>
#include <iostream>

using std::string;
using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
    // create a parser
    cmdline::parser a;

    // add specified type of variable.
    // 1st argument is long name
    // 2nd argument is short name (no short name if '\0' specified)
    // 3rd argument is description
    // 4th argument is mandatory (optional. default is false)
    // 5th argument is default value  (optional. it used when mandatory is false)
    a.add<string>("host", 'h', "host name", true, "");

    // 6th argument is extra constraint.
    // Here, port number must be 1 to 65535 by cmdline::range().
    a.add<int>("port", 'p', "port number", false, 80, cmdline::range(1, 65535));

    // cmdline::oneof() can restrict options.
    a.add<string>("type", 't', "protocol type", false, "http", cmdline::of<string>("http", "https", "ssh", "ftp"));

    // Boolean flags also can be defined.
    // Call add method without a type parameter.
    a.add("gzip", '\0', "gzip when transfer");
    a.footer("see more: https://example.com/");

    // Run parser.
    // It returns only if command line arguments are valid.
    // If arguments are invalid, a parser output error msgs then exit program.
    // If help flag ('--help' or '-?') is specified, a parser output usage message then exit program.
    a.parse_check(argc, argv);

    // use flag values
    cout << a.get<string>("type") << "://"
         << a.get<string>("host") << ":"
         << a.get<int>("port") << endl;

    // boolean flags are referred by calling exist() method.
    if (a.exist("gzip")) cout << "gzip" << endl;
}