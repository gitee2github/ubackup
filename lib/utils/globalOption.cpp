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
#include "globalOption.h"

using namespace std;

const option OPTIONS[3] = {
{ "version",		no_argument,		0,	'v' },
{ "help",		no_argument,		0,	'h' },
{ 0, 0, 0, 0 }
};

string GlobalOptions::help_text()
{
    return string("    Global options:") + '\n'
    + "\t--help, -h\t\t\thelp for ubackup." + '\n'
    + "\t--version, -v\t\t\tprint version and exit." + '\n' + '\n'
    + "Available Commands:" + '\n'
    + "\tbackup full\t\t\tback up full system." + '\n'
    + "\tbackup system\t\t\tback up the necessary files to support the operation of the system." + '\n'
    + "\tbackup data\t\t\tback up user-specified directories or files." + '\n'
    + "\trestore full\t\t\trestore full system from a snapshot." + '\n'
    + "\trestore system\t\t\trestore system from a system type snapshot." + '\n'
    + "\trestore data\t\t\trestore user data from a snapshot." + '\n'
    + "\tremove\t\t\tremoves snapshots according to a policy." + '\n'
    + "\tlist\t\t\tlist objects in the repository based on type." + '\n'
    + "\tlogs\t\t\tshow option logs." + '\n'
    + "\tshow-exclude\t\t\tshows default exclude files or directories." + '\n'
    + "\tshow-include\t\t\tshows must include files or directories before backupfull." + '\n';
}

GlobalOptions::GlobalOptions(GetOpts& parser) :
    Options(parser)
{
    parse_options();

    _version = has_option("version");
    _help = has_option("help");
}

void GlobalOptions::parse_options()
{
    _options = _parser.parse(OPTIONS);
}
