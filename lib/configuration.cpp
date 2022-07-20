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

#include "configuration.h"
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

Config::Config(const string& configPath){
    ifstream in(configPath.c_str());
    if(!in) {
        std::cerr << "config file not found: " << configPath << endl;
        return;
    }
    boost::property_tree::ptree root;
    boost::property_tree::read_json<boost::property_tree::ptree>(configPath, root);

    lastBackupPath = root.get<string>("lastBackupPath", "");
    resticPasswd = root.get<string>("resticPasswd", "");
    snapCnt = root.get<int>("snapCnt", 0);

    snapInfoPath = root.get<string>("snapInfoPath", "");
    logFile = root.get<string>("logFile", "");

    if (root.count("excludes")) {
        boost::property_tree::ptree ptExcludes = root.get_child("excludes");
        for (auto pos = ptExcludes.begin(); pos != ptExcludes.end(); pos++) {
            string dir = pos->second.get_value<string>();
            excludes.push_back(dir);
        }
        excludes.push_back(snapInfoPath);
        excludes.push_back(configPath);
        excludes.push_back(logFile);
    }
    if (root.count("includes")) {
        boost::property_tree::ptree ptIncludes = root.get_child("includes");
        for (auto pos = ptIncludes.begin(); pos != ptIncludes.end(); pos++) {
            string dir = pos->second.get_value<string>();
            includes.push_back(dir);
        }
    }
    if (root.count("cannotExcludes")) {
        boost::property_tree::ptree ptCannotExcludes = root.get_child("cannotExcludes");
        for (auto pos = ptCannotExcludes.begin(); pos != ptCannotExcludes.end(); pos++) {
            string dir = pos->second.get_value<string>();
            cannotExcludes.push_back(dir);
        }
    }
};