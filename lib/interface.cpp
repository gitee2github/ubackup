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

#include "ubackup.h"
#include "utils/util.h"
#include "configuration.h"
#include "config.h"
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>
#include <ctime>
#include <map>

namespace ubackup{

const string configPath = "/etc/ubackup/ubackup.json";
Config c(configPath);
	
Error cannotExclude(Config c, vector<string> excludes) {
    Error err;
    for (auto dir : c.GetCannotExcludes())
    {
        for (auto exclude : excludes)
        {
            if (!exclude.compare(0, dir.length(), dir)) {
                err.errNo = 1;
                err.error = "fire or directory " + dir + " cannot exclude.\n";
            }
        }
    }
    return err;
}

Error cannotInclude(Config c, vector<string> includes, vector<string> exclude) {
    Error err;
    string excludes;
    for (auto dir : c.GetExcludes()) {
        excludes += " " + dir;
    }
    for (auto dir : c.GetExcludes()) {
        for (auto include : includes) {
            if (!include.compare(0, dir.length(), dir)) {
                err.errNo = 1;
                err.error = "fire or directory " + dir + " cannot backup.\n";
                err.error += "the following directory cannot backup: " + excludes;
            }
        }
    }

    return err;
}

void addLogs(const string& logFile, const Log& log) {
    Error err;
    boost::property_tree::ptree root;
    boost::property_tree::ptree pt1;
    string timeStr;
    time2string(log.operationTime, timeStr);
    pt1.put("time", timeStr);
    pt1.put("snapID", log.snaps.front());
    pt1.put("repo", log.repo);
    string opTypeStr;
    operationType2string(log.opType, opTypeStr);
    pt1.put("op_type", opTypeStr);
    pt1.put<bool>("status", log.status);
    pt1.put("comment", log.comment);
    if (!CheckDirExists(logFile) || boost::filesystem::is_empty(logFile)) {
        if (!createFile(logFile)) {
            cerr << "Error creating log file: " << logFile << endl;
            return;
        }
        SystemCmd cmd("sudo touch " + logFile);
        boost::property_tree::ptree ptSnap;
        ptSnap.push_back(make_pair("", pt1));
        root.add_child("log", ptSnap);
    } else {
        boost::property_tree::read_json<boost::property_tree::ptree>(logFile, root);
        if (root.count("log")) {
            root.get_child("log").push_back(make_pair("", pt1));
        }
    }
    boost::property_tree::write_json(logFile, root);
}

void saveSnapshotInfo(const string& snapFile, const Snapshot& snapshot) {
    map<backupType, string> m = { {Full, "full"}, {System, "sys"}, {Data, "data"} };
    Error err;
    boost::property_tree::ptree root;
    boost::property_tree::ptree pt1;
    pt1.put("ID", snapshot.snapshotID);
    string timeStr;
    time2string(snapshot.time, timeStr);
    pt1.put("time", timeStr);
    auto it = m.find(snapshot.type);
    if (it == m.end()) {
        cerr << "unknown snapshot type" << endl;
        return;
    }
    pt1.put("backupType", it->second);
    pt1.put("repo", snapshot.repo);
    pt1.put("repoDevice", snapshot.repoDevice);
    pt1.put("repoMount", snapshot.repoMount);
    if (!CheckDirExists(snapFile) || boost::filesystem::is_empty(snapFile)) {
        if (!createFile(snapFile)) {
            cerr << "Error creating snapshot " << snapFile << endl;
            return;
        }
        SystemCmd cmd("sudo touch " + snapFile + "&& sudo chmod 777 " + snapFile);
        boost::property_tree::ptree ptSnap;
        ptSnap.add_child(snapshot.snapshotID, pt1);
        root.add_child("snapshot", ptSnap);
    } else {
        boost::property_tree::read_json<boost::property_tree::ptree>(snapFile, root);
        if (root.count("snapshot")) {
            root.get_child("snapshot").add_child(snapshot.snapshotID, pt1);
        }
    }
    boost::property_tree::write_json(snapFile, root);
}

void removeSnapshotInfo(const string& snapFile, const string& snapshotID) {
    if (!CheckDirExists(snapFile)) {
        return;
    }
    boost::property_tree::ptree root;
    boost::property_tree::read_json<boost::property_tree::ptree>(snapFile, root);
    if (root.count("snapshot")) {
        boost::property_tree::ptree ptSnap = root.get_child("snapshot");
        if (ptSnap.count(snapshotID)) {
            root.get_child("snapshot").erase(snapshotID);
        }
        boost::property_tree::write_json(snapFile, root);
    }
}

Error backup(const string& repo, vector<string>& includes, vector<string>& excludes, string& snapshotID, backupType type) {
    // 备份前将repo路径添加进excludes
    excludes.push_back(repo);
    Error err;
    if (access(RESTICBIN, X_OK) != 0) {
        err.errNo = EXIT_FAILURE;
        err.error = "restic not exists";
        return err;
    }
    BackupTool bt(BackupTool::createRestic());
    setenv("RESTIC_PASSWORD", c.GetResticPasswd().c_str(),0);
    string now;
    time2string(time(0), now);
    cout << "backup begin " + now << endl;
    err = bt.backup(repo, includes, excludes, snapshotID);
    time2string(time(0), now);
    cout << "backup end " + now << endl;
    return err;
}

Error restore(const string& repo, vector<string>& excludes, const string& snapshotID, string& target) {
    Error err;
    if (access(RESTICBIN, X_OK) != 0) {
        err.errNo = EXIT_FAILURE;
        err.error = "restic not exists";
        return err;
    }
    BackupTool bt(BackupTool::createRestic());
    vector<string> includes;
    setenv("RESTIC_PASSWORD", c.GetResticPasswd().c_str(),0);
    err = bt.restore(repo, target, snapshotID, excludes, includes);
    return err;
}

Error removeSnapshot(const string& repo, const string& snapshotID) {
    Error err;
    if (access(RESTICBIN, X_OK) != 0) {
        err.errNo = EXIT_FAILURE;
        err.error = "restic not exists";
        return err;
    }
    BackupTool bt(BackupTool::createRestic());
    setenv("RESTIC_PASSWORD", c.GetResticPasswd().c_str(),0);
    err = bt.removeSnapshots(repo, snapshotID);
    return err;
}

Error BackupFull(vector<string>& excludes, string& snapshotID, string repo, string comment) {
    Error err;
    vector<string> includes;
    vector<string> allExcludes = c.GetExcludes();
    if (repo == "") {
        repo = c.GetLastBackupPath();
    }
    //检查输入参数repo和excludes，excludes不能排除项检查
    if (!CheckDirExists(repo)) {
        err.errNo = EXIT_FAILURE;
        err.error = "repo " + repo + " not exists";
        return err;
    }
    if (!excludes.empty()) {
        err = CheckDirsExists(excludes);
        if (err.errNo) {
            return err;
        }
        err = cannotExclude(c, excludes);
        if (err.errNo) {
            return err;
        }
        vector<string> defaultExclude = c.GetExcludes();
        for (auto it = defaultExclude.begin(); it != defaultExclude.end(); it++) {
            cout << *it << endl;
        }
        allExcludes.insert(allExcludes.end(), excludes.begin(), excludes.end());
    }
    // 检查repo空间是否足够，不足则报错
    err = CheckSpace(repo, includes, excludes);
    if (err.errNo != 0) {
        return err;
    }
    err = backup(repo, includes, allExcludes, snapshotID, Full);
    // 记录log
    string logFile = c.GetLogFile();
    Log log = setLog(repo, snapshotID, FullBackup, !err.errNo, comment);
    addLogs(logFile, log);
    if (!err.errNo) {
        // repo和snap写入对应info文件
        string snapInfoFile = c.GetSnapInfoPath();
        Snapshot snap = setSnap(repo, snapshotID, Full);
        saveSnapshotInfo(snapInfoFile, snap);
    }
    return err;
}

Error BackupSys(vector<string>& includes, string& snapshotID, string repo, string comment) {
    Error err;
    vector<string> excludes;
    if (repo == "") {
        repo = c.GetLastBackupPath();
    }
    //检查输入参数repo
    if (!CheckDirExists(repo)) {
        err.errNo = EXIT_FAILURE;
        err.error = "repo " + repo + " not exists";
        return err;
    }

    Config c(configPath);
    includes = c.GetIncludes();
    // 检查repo空间是否足够，不足则报错
    err = CheckSpace(repo, includes, excludes);
    if (err.errNo != 0) {
        return err;
    }
    err = backup(repo, includes, excludes, snapshotID, System);
    // 记录log
    string logFile = c.GetLogFile();
    Log log = setLog(repo, snapshotID, SystemBackup, !err.errNo, comment);
    addLogs(logFile, log);
    if (!err.errNo) {
        // repo和snap写入对应info文件
        string snapInfoFile = c.GetSnapInfoPath();
        Snapshot snap = setSnap(repo, snapshotID, System);
        saveSnapshotInfo(snapInfoFile, snap);
        return err;
    }
}

Error BackupData(vector<string>& includes, vector<string>& excludes, string& snapshotID, string repo, string comment) {
    Error err;
    if (repo == "") {
        repo = c.GetLastBackupPath();
    }
    //检查输入参数repo和excludes，excludes不能排除项检查
    if (!CheckDirExists(repo)) {
        err.errNo = EXIT_FAILURE;
        err.error = "repo " + repo + " not exists";
        return err;
    }
    err = CheckDirsExists(includes);
    if (err.errNo) {
        return err;
    }
    Config c(configPath);
    err = cannotInclude(c, includes, excludes);
    if (err.errNo) {
        return err;
    }
    // 检查repo空间是否足够，不足则报错
    err = CheckSpace(repo, includes, excludes);
    if (err.errNo) {
        return err;
    }
    err = backup(repo, includes, excludes, snapshotID, Data);
    // 记录log
    string logFile = c.GetLogFile();
    Log log = setLog(repo, snapshotID, DataBackup, !err.errNo, comment);
    addLogs(logFile, log);
    if (!err.errNo) {
        // repo和snap写入对应info文件
        string snapInfoFile = c.GetSnapInfoPath();
        Snapshot snap = setSnap(repo, snapshotID, Data);
        saveSnapshotInfo(snapInfoFile, snap);
    }
    return err;
}

Error PreBackup(vector<string>& includes, vector<string>& excludes, backupType type) {
    Error err;
    Config c(configPath);
    excludes = c.GetExcludes();
    if (type == System) {
        includes = c.GetIncludes();
    } else if (type == Full) {
        includes = c.GetCannotExcludes();
    }
    return err;
}

}