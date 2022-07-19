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





}