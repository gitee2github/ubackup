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

#include <string>

#include "options.h"

using namespace std;

Options::Options(GetOpts& parser) :
    _parser(parser), _options()
{}

Options::~Options()
{}

bool Options::has_option(const string option_name) const
{
    GetOpts::parsed_opts::const_iterator option = get_option(option_name);

    return option != _options.end();
}

GetOpts::parsed_opts::const_iterator
Options::get_option(const string option_name) const
{
    return _options.find(option_name);
}

bool Options::has_errors() const
{
    return !errors().empty();
}
