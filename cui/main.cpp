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

#include <lib/ubackup.h>
#include <lib/utils/globalOption.h>
#include "lib/internal/SystemCmd.h"
#include <list>
#include <algorithm>

using namespace std;
using namespace ubackup;
const string VERSION = "1.0";
struct Cmd
{
    typedef void (*cmd_func_t)();

    typedef void (*help_func_t)();

    Cmd(const string& name, cmd_func_t cmd_func, help_func_t help_func)
	: name(name), aliases(), cmd_func(cmd_func), help_func(help_func)
    {}

    Cmd(const string& name, const vector<string>& aliases, cmd_func_t cmd_func,
	help_func_t help_func)
	: name(name), aliases(aliases), cmd_func(cmd_func), help_func(help_func)
    {}

    const string name;
    const vector<string> aliases;
    const cmd_func_t cmd_func;
    const help_func_t help_func;
};

int main(int argc, char** argv) {s
    return 0;
}

