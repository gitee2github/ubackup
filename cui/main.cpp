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

GetOpts getopts;

/**
 * ubackup backup full -r repopath --exclude dir1 dir2 ...
 * ubackup backup system -r repopath
 * ubackup backup data -r repopath --include dir1 dir2 ...
 * ubackup restore sys -n snapshotID
 * ubackup restore data -n snapshotID [--exclude dir1 dir2 ...]
 * ubackup restore full -n snapshotID [--exclude dir1 dir2 ...]
 * ubackup remove -l snapshotID1 snapshotID2 ...
 * ubackup list [-r repopath] [-t backupType]
 * ubackup logs
 * // ubackup ls -n snapshotID
 * ubackup show-exclude
 * ubackup show-include
 * // ubackup restoregrub -r repopath -n snapshotID -t target
 * */
void
helpBackupFull()
{
    cout << "  backup full:" << '\n'
	 << "\tback up full system." << '\n'
	 << '\n'
	 << "    Options for 'backup full' command:" << '\n'
	 << "\t-r, --repo <repopath>\t\trepository to backup to." << '\n'
     	 << "\t-e, --exclude <pattern>\t\texclude a pattern." << '\n'
	 << "\t-m, --comment <comments>\t\tcomment to this backup." << '\n'
	 << endl;
}

void
helpBackupSys()
{
    return;
}

void
helpBackupData()
{
   return;
}

void
helpBackup()
{
    	helpBackupFull();
	helpBackupSys();
	helpBackupData();
}

void cmdBackupFull() {
	return;
}

int main(int argc, char** argv) {
	
	const list<Cmd> cmds = {
	Cmd("backup", cmdBackupFull, helpBackup)
	}	
  	  return 0;
}

