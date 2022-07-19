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

Error RestoreSys(const string& snapshotID, string repo, string target) {
    vector<string> excludes;
    return RestoreFull(snapshotID, excludes, repo, target);
}

Error RestoreFull(const string& snapshotID, vector<string>& excludes, string repo, string target) {
    Error err = CheckRestoreInfo(repo, snapshotID, excludes);
    if (err.errNo) {
        return err;
    }
    err = restore(repo, excludes, snapshotID, target);
    // 记录log
    string logFile = c.GetLogFile();
    Log log = setLog(repo, snapshotID, FullRestore, !err.errNo, "");
    addLogs(logFile, log);
    return err;
}

Error RestoreData(const string& snapshotID, vector<string>& excludes, string repo) {
    return RestoreFull(snapshotID, excludes, repo);
}

Error ListSnaps(const string& repo, vector<Snapshot>& snapshots, backupType type) {
    Error err;
    vector<Snapshot> all;
    err = ListAllSnaps(all);
    if (err.errNo) {
        return err;
    }
    for (auto snap : all) {
        if (snap.type == type) {
            if (repo == "" || snap.repo == repo) {
                snapshots.push_back(snap);
            }
        }
    }
    return err;
}

Error ListAllSnaps(vector<Snapshot>& snapshots) {
    Error err;
    string snapPath = c.GetSnapInfoPath();
    if (!CheckDirExists(snapPath)) {
        err.errNo = 1;
        err.error = "snap info file " + snapPath + " not exists";
        return err;
    }
    if (boost::filesystem::is_empty(snapPath)) {
        return err;
    }
    boost::property_tree::ptree root;
    boost::property_tree::read_json<boost::property_tree::ptree>(snapPath, root);
    if (root.count("snapshot")) {
        boost::property_tree::ptree ptSnap = root.get_child("snapshot");
        for (auto pos = ptSnap.begin(); pos != ptSnap.end(); pos++) {
            string typeStr = pos->second.get<string>("backupType");
            Snapshot snap;
            snap.snapshotID = pos->second.get<string>("ID");
            string2backupType(typeStr, snap.type);
            string time = pos->second.get<string>("time");
            string2time(snap.time, time);
            snap.repo = pos->second.get<string>("repo");
            snap.repoDevice = pos->second.get<string>("repoDevice");
            snap.repoMount = pos->second.get<string>("repoMount");
            snapshots.push_back(snap);
        }
    }
    return err;
}

Error RemoveSnapshots(const vector<string>& snapshotID) {
    Error err;
    string snapInfoPath = c.GetSnapInfoPath();
    vector<Snapshot> allSnaps;
    string repo;
    ListAllSnaps(allSnaps);
    for (auto deleteSnap : snapshotID) {
        auto it = allSnaps.begin();
        for (;it != allSnaps.end(); it++) {
            if (deleteSnap == it->snapshotID) {
                repo = it->repo;
                break;
            }
        }
        if (it == allSnaps.end()) {
            err.errNo = 1;
            err.error = "snapshotID " + deleteSnap + " not exists";
            return err;
        }
        err = removeSnapshot(repo, deleteSnap);
        if (!err.errNo) {
            // 移除snapInfo
            removeSnapshotInfo(snapInfoPath, deleteSnap);
            // // 移除repo信息(如果需要)
            // removeRepoInfo(repo);
        }
        // 记录log
        string logFile = c.GetLogFile();
        Log log = {};
        log.operationTime = time(0);
        log.opType = RemoveSnaps;
        log.repo = repo;
        log.status = !err.errNo;
        log.snaps.push_back(deleteSnap);
        addLogs(logFile, log);
    }

    return err;
}

Error CheckSpace(const string& repo, const vector<string>& includes, const vector<string>& excludes) {
    Error err;
    string dfCmd = "df -k";
    dfCmd += repo + " | awk 'END {print}'";
    SystemCmd cmd(dfCmd);
    if (cmd.retcode()) {
        for (auto out : cmd.stderr()) {
            cerr << out << endl;
        }
        return err;
    }
    string result = cmd.stdout().front();
    vector<string> tmp;
    split(result, tmp);
    if (tmp.size() < 6) {
        return err;
    }
    long available = atol(tmp[3].c_str());
    cout << available << endl;
    if (!includes.empty()) {
        long size = 0;
        for (auto include : includes) {
            string duCmd = "du --max-depth 1 -lk " + include;
            SystemCmd cmd(duCmd);
            if (cmd.retcode()) {
                for (auto out : cmd.stderr()) {
                    cerr << out << endl;
                }
                return err;
            }
            string result = cmd.stdout().front();
            vector<string> tmp;
            split(result, tmp);
            if (tmp.size() < 1) {
                return err;
            }
            cout << result << endl;
            size += atol(tmp[0].c_str());
        }
        if (available <= size) {
            err.errNo = 1;
            err.error = "no enough space, available: " + to_string(available) + "k, backup size:" + to_string(size) + "k";
        }
    } else {

    }
    return err;
}

Error ShowLogs(vector<Log>& logs) {
    Error err;
    string logFile = c.GetLogFile();
    if (!CheckDirExists(logFile)) {
        return err;
    }
    if (boost::filesystem::is_empty(logFile)) {
        return err;
    }
    boost::property_tree::ptree root;
    boost::property_tree::read_json<boost::property_tree::ptree>(logFile, root);
    if (root.count("log")) {
        boost::property_tree::ptree ptSnap = root.get_child("log");
        for (auto pos = ptSnap.begin(); pos != ptSnap.end(); pos++) {
            Log log;
            log.snaps.push_back(pos->second.get<string>("snapID"));
            string2operationType(pos->second.get<string>("op_type"), log.opType);
            string time = pos->second.get<string>("time");
            string2time(log.operationTime, time);
            log.repo = pos->second.get<string>("repo");
            log.status = pos->second.get<bool>("status");
            log.comment = pos->second.get<string>("comment");
            logs.push_back(log);
        }
    }
    return err;
}

Error CheckDirsExists(const vector<string>& directory) {
    Error err;
    vector<string> notexists;
    for (auto dir : directory) {
        if (!CheckDirExists(dir)) {
            notexists.push_back(dir);
        }
    }
    if (!notexists.empty()) {
        err.errNo = EXIT_FAILURE;
        err.error = "file or directory ";
        for (auto it : notexists) {
            err.error += it + " ";
        }
        err.error += " not exists";
    }

    return err;
}

bool CheckDirExists(const string& directory) {
    bool exist= false;
    ifstream in(directory.c_str());
    if(in)
        exist = true;
    return exist;
}

Error CheckRestoreInfo(string& repo, const string& snapshotID, const vector<string>& excludes) {
    Error err;
    if (!excludes.empty()) {
        err = CheckDirsExists(excludes);
        if (err.errNo) {
            return err;
        }
    }
    string snapInfoPath = c.GetSnapInfoPath();
    vector<Snapshot> allSnaps;
    ListAllSnaps(allSnaps);
    auto it = allSnaps.begin();
    for (;it != allSnaps.end(); it++) {
        if (snapshotID == it->snapshotID) {
            if (repo == "") {
                repo = it->repo;
                break;
            }
            if (repo != it->repo) {
                err.errNo = EXIT_FAILURE;
                err.error = "there is no snapshot " + snapshotID + " at location " + repo;
                return err;
            } else {
                break;
            }
        }
    }
    if (it == allSnaps.end()) {
        err.errNo = EXIT_FAILURE;
        err.error = "snapshotID " + snapshotID + " not exist";
        return err;
    }
    if (!CheckDirExists(repo)) {
        err.errNo = EXIT_FAILURE;
        err.error = "repo " + repo + " not exist";
    }
    return err;
}

void time2string(const time_t time, string& des) {
    std::tm * ptm = std::localtime(&time);
    char buffer[32];
    std::strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", ptm);
    des=buffer;
}

void string2time(time_t& time, const string& des) {
    tm tm_;                                    // 定义tm结构体。
    int year, month, day, hour, minute, second;// 定义时间的各个int临时变量。
    sscanf(des.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);// 将string存储的日期时间，转换为int临时变量。
    tm_.tm_year = year - 1900;                 // 年，由于tm结构体存储的是从1900年开始的时间，所以tm_year为int临时变量减去1900。
    tm_.tm_mon = month - 1;                    // 月，由于tm结构体的月份存储范围为0-11，所以tm_mon为int临时变量减去1。
    tm_.tm_mday = day;                         // 日。
    tm_.tm_hour = hour;                        // 时。
    tm_.tm_min = minute;                       // 分。
    tm_.tm_sec = second;                       // 秒。
    tm_.tm_isdst = 0;                          // 非夏令时。
    time = mktime(&tm_);                       // 将tm结构体转换成time_t格式。
}

void backupType2string(const backupType& type, string& des) {
    map<backupType, string> m = { {Full, "full"}, {System, "sys"}, {Data, "data"} };
    des = m.find(type)->second;
}

void string2backupType(const string& src, backupType& type) {
    map<string, backupType> m = { {"full", Full}, {"sys", System}, {"data", Data} };
    type = m.find(src)->second;
}

void operationType2string(const operationType& type, string& des) {
    map<operationType, string> m = { {FullBackup, "full backup"}, {SystemBackup, "system backup"}, {DataBackup, "data backup"}, {FullRestore, "full restore"}, {SystemRestore, "system restore"}, {DataRestore, "data restore"}, {RemoveSnaps, "remove"} };
    des = m.find(type)->second;
}

void string2operationType(const string& src, operationType& type) {
    map<string, operationType> m = { {"full backup", FullBackup}, {"system backup", SystemBackup}, {"data backup", DataBackup}, {"full restore", FullRestore}, {"system restore", SystemRestore}, {"data restore", DataRestore}, {"remove", RemoveSnaps} };
    type = m.find(src)->second;
}

}
