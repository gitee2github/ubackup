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
    cout << "  backup system:" << '\n'
	 << "\tback up the necessary files to support the operation of the system." << '\n'
	 << '\n'
	 << "   Options for 'backup system' command:" << '\n'
	 << "\t-r, --repo <repopath>\t\trepository to backup to." << '\n'
	 << "\t-m, --comment <comments>\t\tcomment to this backup." << '\n'
	 << endl;
}

void
helpBackupData()
{
   cout << "  backup data <--include dir1 [dir2 ...]> :" << '\n'
	 << "\tback up user-specified directories or files." << '\n'
	 << '\n'
	 << "    Options for 'backup data' command:" << '\n'
	 << "\t-r, --repo <repopath>\t\trepository to backup to." << '\n'
     	 << "\t-i, --include <pattern>\t\tinclude a pattern." << '\n'
	 << "\t-m, --comment <comments>\t\tcomment to this backup." << '\n'
	 << endl;
}

void
helpBackup()
{
    	helpBackupFull();
	helpBackupSys();
	helpBackupData();
}


void
helpRestoreFull()
{
    cout << "  restore full <-n snapshotID> :" << '\n'
	 << "\trestore full system from a snapshot." << '\n'
	 << '\n'
	 << "    Options for 'restore full' command:" << '\n'
	 << "\t-r, --repo <repopath>\t\trepository to backup to or restore from." << '\n'
     	 << "\t-n, <snapshotID>\t\trestore full disk from a snapshot." << '\n'
	 << endl;
}

void
helpRestoreSys()
{
        cout << "  restore system <-n snapshotID> :" << '\n'
	 << "\trestore system from a system type snapshot." << '\n'
	 << '\n'
	 << "    Options for 'restore system' command:" << '\n'
     	 << "\t-n, <snapshotID>\t\trestore system from a snapshot." << '\n'
	 << endl;
}

void
helpRestoreData()
{
       cout << "  restore data <-n snapshotID> :" << '\n'
	 << "\trestore user data from a snapshot." << '\n'
	 << '\n'
	 << "    Options for 'restore data' command:" << '\n'
	 << "\t-r, --repo <repopath>\t\trepository to backup to or restore from." << '\n'
     	 << "\t-n, <snapshotID>\t\trestore the data from a snapshot." << '\n'
	 << "\t-e, --exclude <dir1 [dir2 ...]>\t\trestore exclude some directories or files." << '\n'
	 << endl;
}

void
helpRestore()
{
    	helpRestoreFull();
	helpRestoreSys();
	helpRestoreData();
}

void
helpRemove()
{
	cout << "  remove <-l snapshotID1 [snapshotID2 ...]> :" << '\n'
	 << "\tremoves snapshots according to a policy." << '\n'
	 << '\n'
	 << "    Options for 'remove' command:" << '\n'
     	 << "\t-l, --list <snapshotID1> [snapshotID2 ...]\t\tremoves snapshots according to a policy." << '\n'     
	 << endl;
}

void cmdBackupSys() {
	const struct option options[] = {
	{ "repopath",		required_argument,	0,	'r' },
	{ "comment",		required_argument,	0,	'm' },
	{ 0, 0, 0, 0 }
    };
	GetOpts::parsed_opts opts = getopts.parse("backupsys", options);
	if (getopts.numArgs() != 0) {
		cerr << "backup type should in full, system, data." << endl;
		exit(EXIT_FAILURE);
    }
	GetOpts::parsed_opts::const_iterator opt;
	string repopath;
	vector<string> excludes;
	if ((opt = opts.find("repopath")) != opts.end()) {
		repopath = opt->second.front();
	}
	string comment;
	if ((opt = opts.find("comment")) != opts.end()) {
		comment = opt->second.front();
	}
	vector<string> defaultExcludes;
    vector<string> includes;
    // 备份前调用PreBackup获取将备份的目录
    Error err;
	err = PreBackup(includes, defaultExcludes, System);
    if (err.errNo != 0) {
        cerr << "PreBackup error: " << err.error << endl;
        exit(err.errNo);
    }
	string snapID;

	err = BackupSys(includes, snapID, repopath, comment);
	if (err.errNo != 0) {
		cerr << "Command 'backupsys' failed, error: " << err.error << endl;
		exit(err.errNo);
	} else {
		cout << "backup successful, snapshot: " << snapID << endl;
	}
}



void cmdBackupData() {
	const struct option options[] = {
	{ "repopath",		required_argument,	0,	'r' },
	{ "include",		required_argument,	0,	'i' },
	{ "comment",		required_argument,	0,	'm' },
	{ 0, 0, 0, 0 }
    };
	GetOpts::parsed_opts opts = getopts.parse("backupdata", options);
	if (getopts.numArgs() != 0) {
		cerr << "backup type should in full, system, data." << endl;
		exit(EXIT_FAILURE);
    }
	GetOpts::parsed_opts::const_iterator opt;
	string repopath;
	vector<string> includes;
	if ((opt = opts.find("repopath")) != opts.end()) {
		repopath = opt->second.front();
	}

	if ((opt = opts.find("include")) != opts.end()) {
		includes = opt->second;
	} else {
		cerr << "Missing command argument --include." << endl;
		exit(EXIT_FAILURE);
	}
	string comment;
	if ((opt = opts.find("comment")) != opts.end()) {
		comment = opt->second.front();
	}
	vector<string> defaultExcludes;
    vector<string> null;
    // 备份前调用PreBackup获取默认忽略的目录，用户输入时对比并报错
	Error err = PreBackup(null, defaultExcludes, Full);
    if (err.errNo != 0) {
        cerr << "PreBackup error: " << err.error << endl;
        exit(err.errNo);
    }
    // 检查用户输入的备份目录是否存在
    for (auto it = includes.begin(); it != includes.end(); it++) {
        if (!CheckDirExists(*it)) {
            cerr << "directory " << *it << " not exist" << endl;
            exit(EXIT_FAILURE);
        }
    }
	string snapID;
	vector<string> excludes;
	err = BackupData(includes, excludes, snapID, repopath, comment);
	if (err.errNo != 0) {
		cerr << "Command 'backupdata' failed, error: " << err.error << endl;
		exit(err.errNo);
	} else {
		cout << "backup successful, snapshot: " << snapID << endl;
	}
	exit(EXIT_SUCCESS);
}


void cmdBackupFull() {
	if (!getopts.hasArgs()) {
		cerr << "backup type should in full, system, data." << endl;
		exit(EXIT_FAILURE);
	}
	string typeStr = getopts.popArg();
	if (typeStr == "full") {
	} else if (typeStr == "system") {
		cmdBackupSys();
	} else if (typeStr == "data") {
		cmdBackupData();
	} else {
		cerr << "backup type should in full, system, data." << endl;
		exit(EXIT_FAILURE);
	}
	const struct option options[] = {
	{ "repopath",		required_argument,	0,	'r' },
	{ "exclude",		required_argument,	0,	'e' },
	{ "comment",		required_argument,	0,	'm' },
	{ 0, 0, 0, 0 }
    };
	GetOpts::parsed_opts opts = getopts.parse("backup", options);
	if (getopts.numArgs() != 0) {
		cerr << "backup type should in full, system, data." << endl;
		exit(EXIT_FAILURE);
    }
	GetOpts::parsed_opts::const_iterator opt;
	string repopath;
	vector<string> excludes;
	if ((opt = opts.find("repopath")) != opts.end()) {
		repopath = opt->second.front();
	}

	if ((opt = opts.find("exclude")) != opts.end()) {
		excludes = opt->second;
	}
	string comment;
	if ((opt = opts.find("comment")) != opts.end()) {
		comment = opt->second.front();
	}
	vector<string> defaultExcludes;
    vector<string> cannotExcludes;
    // 检查用户输入的忽略目录是否存在
    for (auto it = excludes.begin(); it != excludes.end(); it++) {
        if (!CheckDirExists(*it)) {
            cerr << "directory " << *it << " not exist" << endl;
            exit(EXIT_FAILURE);
        }
    }
	string snapID;
	Error err = BackupFull(excludes, snapID, repopath, comment);
	if (err.errNo != 0) {
		cerr << "Command 'backup full' failed, error: " << err.error << endl;
		exit(err.errNo);
	} else {
		cout << "backup successful, snapshot: " << snapID << endl;
	}
	exit(EXIT_SUCCESS);
}


void cmdRestoreSys() {
	const struct option options[] = {
	{ "repopath",		required_argument,	0,	'r' },
	{ "snapshot",		required_argument,	0,	'n' },
	{ "noreboot",			no_argument,	0,	0 },
	{ 0, 0, 0, 0 }
    };
	GetOpts::parsed_opts opts = getopts.parse("restore system", options);
	if (getopts.numArgs() != 0) {
		cerr << "restore type should in full, system, data, grub." << endl;
		exit(EXIT_FAILURE);
    }
	string snapshotID;
	string repopath;
	vector<string> excludes;
	restoreParse(opts, snapshotID, repopath, excludes);
	Error err = RestoreSys(snapshotID, repopath);
	if (err.errNo != 0) {
		cerr << "Command 'restore system' failed, error: " << err.error << endl;
		exit(EXIT_FAILURE);
	} else {
		cout << "restore successful." << endl;
	}
	exit(EXIT_SUCCESS);
}

void cmdRestoreGrub() {
	const struct option options[] = {
	{ "repopath",		required_argument,	0,	'r' },
	{ "snapshot",		required_argument,	0,	'n' },
	{ "target",			required_argument,	0,	't' },
	{ 0, 0, 0, 0 }
    };
	GetOpts::parsed_opts opts = getopts.parse("restoregrub", options);
	if (getopts.numArgs() != 0) {
		cerr << "restore type should in full, system, data, grub." << endl;
		exit(EXIT_FAILURE);
    }
	GetOpts::parsed_opts::const_iterator opt;
	string repopath;
	vector<string> excludes;
	if ((opt = opts.find("repopath")) != opts.end()) {
		repopath = opt->second.front();
	} else {
		cerr << "Missing command argument repopath." << endl;
		exit(EXIT_FAILURE);
	}
	string snapshotID;
	if ((opt = opts.find("snapshot")) != opts.end()) {
		snapshotID = opt->second.front();
	} else {
		cerr << "Missing command argument -n snapshotID." << endl;
		exit(EXIT_FAILURE);
	}
	string target;
	if ((opt = opts.find("target")) != opts.end()) {
		target = opt->second.front();
	} else {
		cerr << "Missing command argument target." << endl;
		exit(EXIT_FAILURE);
	}
	Error err = RestoreSys(snapshotID, repopath, target);
	if (err.errNo != 0) {
		cerr << "Command 'restoregrub' failed, error: " << err.error << endl;
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}     

void cmdRestoreData() {
	const struct option options[] = {
	{ "repopath",		required_argument,	0,	'r' },
	{ "snapshot",		required_argument,	0,	'n' },
	{ "exclude",		required_argument,	0,	'e' },
	{ 0, 0, 0, 0 }
    };
	GetOpts::parsed_opts opts = getopts.parse("restoredata", options);
	if (getopts.numArgs() != 0) {
		cerr << "restore type should in full, system, data, grub." << endl;
		exit(EXIT_FAILURE);
    }
	string snapshotID;
	string repopath;
	vector<string> excludes;
	GetOpts::parsed_opts::const_iterator opt;
	if ((opt = opts.find("repopath")) != opts.end()) {
		repopath = opt->second.front();
	}
	if ((opt = opts.find("snapshot")) != opts.end()) {
		snapshotID = opt->second.front();
	} else {
		cerr << "Missing command argument -n snapshotID." << endl;
		exit(EXIT_FAILURE);
	}
	if ((opt = opts.find("exclude")) != opts.end()) {
		excludes = opt->second;
	}
	Error err = RestoreData(snapshotID, excludes, repopath);
	if (err.errNo != 0) {
		cerr << "Command 'restore data' failed, error: " << err.error << endl;
		exit(EXIT_FAILURE);
	} else {
		cout << "restore successful." << endl;
	}
	exit(EXIT_SUCCESS);
}

void cmdRestoreFull() {
	if (!getopts.hasArgs()) {
		cerr << "restore type should in full, system, data, grub." << endl;
		exit(EXIT_FAILURE);
	}
	string typeStr = getopts.popArg();
	if (typeStr == "full") {
	} else if (typeStr == "system") {
		cmdRestoreSys();
	} else if (typeStr == "data") {
		cmdRestoreData();
	} else if (typeStr == "grub") {
		cmdRestoreGrub();
	} else {
		cerr << "restore type should in full, system, data, grub." << endl;
		exit(EXIT_FAILURE);
	}
	const struct option options[] = {
	{ "repopath",		required_argument,	0,	'r' },
	{ "snapshot",		required_argument,	0,	'n' },
	{ "exclude",		required_argument,	0,	'e' },
	{ "noreboot",			no_argument,	0,	0 },
	{ 0, 0, 0, 0 }
    };
	GetOpts::parsed_opts opts = getopts.parse("restore full", options);
	if (getopts.numArgs() != 0) {
		cerr << "restore type should in full, system, data, grub." << endl;
		exit(EXIT_FAILURE);
    }
	string snapshotID;
	string repopath;
	vector<string> excludes;
	restoreParse(opts, snapshotID, repopath, excludes);
	Error err = RestoreFull(snapshotID, excludes, repopath);
	if (err.errNo != 0) {
		cerr << "Command 'restore full' failed, error: " << err.error << endl;
		exit(EXIT_FAILURE);
	} else {
		cout << "restore successful." << endl;
	}
}

void cmdRemove() {
	const struct option options[] = {
		{ "list",		required_argument,	0,	'l' },
		{ 0, 0, 0, 0 }
    	};
	GetOpts::parsed_opts opts = getopts.parse("remove", options);
	if (getopts.numArgs() != 0) {
		cerr << "command 'remove' does not require parameters." << endl;
		exit(EXIT_FAILURE);
    	}
	GetOpts::parsed_opts::const_iterator opt;
	string repopath;
	vector<string> snaps;
	if ((opt = opts.find("list")) != opts.end()) {
		snaps = opt->second;
	} else {
		cerr << "Missing command argument --list." << endl;
		exit(EXIT_FAILURE);
	}
	Error err = RemoveSnapshots(snaps);
	if (err.errNo != 0) {
		cerr << "Command 'remove' failed, error: " << err.error << endl;
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char** argv) {
	
	const list<Cmd> cmds = {
	Cmd("backup", cmdBackupFull, helpBackup),
	Cmd("restore", cmdRestoreFull, helpRestore),
	Cmd("remove", cmdRemove, helpRemove)
	}	
  	  return 0;
}

