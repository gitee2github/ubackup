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

#include "restic.h"
#include "SystemCmd.h"
#include "lib/utils/util.h"
#include <unistd.h>
#include <fstream>

Error errRestic;

Error ResticTool::backup(const string& repo, vector<string>& includes, vector<string>& excludes, string& snapshotID) {
    Error err;	
    string resticCmd = "restic ";
    string subCommand = "backup";
    string options;
    options = options + " -r " + repo;
    if (includes.empty()) {
        subCommand += " /";
        string userExclude;
        for (auto it = excludes.begin(); it != excludes.end(); it++) {
            userExclude += " --exclude ";
            userExclude += *it;
        }
        options += userExclude;
    } else {
        string userInclude;
        for (auto it = includes.begin(); it != includes.end(); it++) {
            userInclude += " ";
            userInclude += *it;
        }
        options += userInclude;
    }
    resticCmd = resticCmd + subCommand + options;
    cout << resticCmd << endl;
    err = createRepo(repo);
    if (err.errNo) {
        return err;
    }
    SystemCmd cmd(resticCmd);

    // restic 0和3都是成功，3是有错误信息的成功(如部分目录无权限等)，1是失败
    if (cmd.retcode() != 0 && cmd.retcode() != 3) {
        for (const string& line : cmd.stderr())
	        cerr << line << endl;
        err.errNo = cmd.retcode();
        err.error = "backup failed";
        return err;
    }
    for (vector<string>::const_iterator cit = cmd.stdout().begin(); cit != cmd.stdout().end(); cit++) {
	    cout << *cit << endl;
        if (cit->find("saved")!=string::npos) {
            cout << cit->find("saved") << endl;
            vector<string> splits;
            split(*cit, splits);
            snapshotID = splits[1];
        }
    }
    return err;
}

Error ResticTool::restore(const string& repo, const string& target, const string& snapshotID, vector<string>& excludes, vector<string>& includes) {
    Error err;
    string resticCmd = "restic ";
    string subCommand = "restore ";
    subCommand += snapshotID;
    string options;
    options = options + " -r " + repo;
    options = options + " --target " + target;
    if (!excludes.empty()) {
        string userExclude;
        for (auto it = excludes.begin(); it != excludes.end(); it++) {
            userExclude += " --exclude ";
            userExclude += *it;
        }
        options += userExclude;
    }
    resticCmd = resticCmd + subCommand + options;
    cout << resticCmd << endl;
    SystemCmd cmd(resticCmd);

    if (cmd.retcode() != 0) {
        for (const string& line : cmd.stderr())
	        cerr << line << endl;
        err.errNo = cmd.retcode();
        err.error = "restore failed";
    }
    return err;
}
Error ResticTool::listSnaps(const string& repo, vector<Snapshot>& snapshots) {
    return errRestic;
}
Error ResticTool::removeSnapshots(const string& repo, const string& snapshots) {
    Error err;
    SystemCmd unlockCmd("restic unlock -r " + repo);
    string resticCmd = "restic ";
    string subCommand = "forget ";
    subCommand += snapshots;
    string options;
    options = options + " -r " + repo;
    resticCmd = resticCmd + subCommand + options;
    cout << resticCmd << endl;
    SystemCmd cmd(resticCmd);

    if (cmd.retcode() != 0) {
        for (const string& line : cmd.stderr())
	        cerr << line << endl;
        err.errNo = cmd.retcode();
        err.error = "forget failed";
    }   
    return err;
}
