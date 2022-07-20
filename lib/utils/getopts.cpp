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

GetOpts::parsed_opts
GetOpts::parse(const struct option* longopts)
{
    return parse(NULL, longopts);
}

GetOpts::parsed_opts
GetOpts::parse(const char* command, const struct option* longopts)
{
    parsed_opts result;
    opterr = 0;			// we report errors on our own

    string optstring = make_optstring(longopts);
    short2long_t short2long = make_short2long(longopts);

    while (true)
    {
	int option_index = 0;
	int c = getopt_long(argc, argv, optstring.c_str(), longopts, &option_index);

	switch (c)
	{
	    case -1:
		return result;

	    case '?':
		cerr << "Unknown option " << argv[optind - 1] << endl;
		cerr << "Try 'ubackup --help' for more information." << endl;
		exit(EXIT_FAILURE);

	    case ':':
        cerr << "Missing argument for command option " << argv[optind - 1] << endl;
		cerr << "Try 'ubackup --help' for more information." << endl;
		exit(EXIT_FAILURE);

	    default:
		const char* opt = c ? short2long[c] : longopts[option_index].name;		
		result[opt].push_back(optarg ? optarg : "");
		while (optind < argc && *argv[optind] != '-') {
			result[opt].push_back(argv[optind] ? argv[optind] : "");
			optind++;
		}
		
		break;
	}
    }
}
