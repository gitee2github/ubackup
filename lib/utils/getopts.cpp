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

#include "getopts.h"
#include <iostream>

using namespace std;

const struct option GetOpts::no_options[1] = {
    { 0, 0, 0, 0 }
};

void GetOpts::init(int new_argc, char** new_argv)
{
    argc = new_argc;
    argv = new_argv;
}

string GetOpts::make_optstring(const struct option* longopts) const {
    // '+' - do not permute, stop at the 1st nonoption, which is the command or an argument
    // ':' - return ':' to indicate missing arg, not '?'
    string optstring = "+:";

    for (; longopts && longopts->name; ++longopts)
    {
	if (!longopts->flag && longopts->val)
	{
	    optstring += (char) longopts->val;
	    if (longopts->has_arg == required_argument)
		optstring += ':';
	    else if (longopts->has_arg == optional_argument)
		optstring += "::";
	}
    }

    return optstring;
}

GetOpts::short2long_t
GetOpts::make_short2long(const struct option* longopts) const
{
    short2long_t result;

    for (; longopts && longopts->name; ++longopts)
    {
	if (!longopts->flag && longopts->val)
	{
	    result[longopts->val] = longopts->name;
	}
    }

    return result;
}