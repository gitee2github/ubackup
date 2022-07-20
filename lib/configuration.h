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

#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H

#include <string>
#include <vector>

using namespace std;

class Config {
public:
    Config(const string& configPath);
    virtual ~Config(){};
    string GetLastBackupPath() {
        return lastBackupPath;
    }

    string GetResticPasswd() {
        return resticPasswd;
    }
    vector<string> GetIncludes() {
        return includes;
    };
    vector<string> GetCannotExcludes() {
        return cannotExcludes;
    };
private:
    string lastBackupPath;
    string resticPasswd;
    vector<string> includes;
    vector<string> cannotExcludes;
};

#endif 