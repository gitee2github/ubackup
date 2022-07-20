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


#ifndef __GLOBAL_OPTION_H
#define __GLOBAL_OPTION_H

#include <string>
#include <vector>
#include "getopts.h"
#include "options.h"

using namespace std;

class GlobalOptions : public Options
{

public:
    static std::string help_text();

    GlobalOptions(GetOpts& parser);

    virtual std::vector<std::string> errors() const override;

    bool version() const
    {
    return _version;
    }
private:

    void parse_options();

    bool _version;

    bool _help;
};

#endif