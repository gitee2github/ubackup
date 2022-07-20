/* 
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as published by 
 * the Free Software Foundation.
 *  
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY; 
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License along with this program; 
 * If not, see <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>.
 * 
 * To contact us about this file by physical or electronic mail, 
 * you may find current contact information at https://www.uniontech.com/.
 */

#ifndef GET_OPTS_H
#define GET_OPTS_H

#include <getopt.h>
#include <string>
#include <map>
#include <list>
#include <vector>
#include <iostream>


class GetOpts
{
public:

    static const struct option no_options[1];

    typedef std::map<std::string, std::vector<std::string> > parsed_opts;

    void init(int argc, char** argv);

    // longopts.flag must be NULL
    parsed_opts parse(const struct option* longopts);
    parsed_opts parse(const char* command, const struct option* longopts);

    bool hasArgs() const { return argc - optind > 0; }

    int numArgs() const {
        return argc - optind; 
    }

    const char* popArg() { return argv[optind++]; }

private:

    int argc;
    char** argv;

    std::string make_optstring(const struct option* longopts) const;

    typedef std::map<int, const char*> short2long_t;

    short2long_t make_short2long(const struct option* longopts) const;
};

#endif
