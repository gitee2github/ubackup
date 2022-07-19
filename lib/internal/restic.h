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

#ifndef __RESTIC_H
#define __RESTIC_H

#include "interface_adaptor.h"
#include <unistd.h>

class ResticTool : public BackupTool::Impl {

public:
    ResticTool(){};
    virtual ~ResticTool(){};

    virtual Error backup(const string& repo, vector<string>& includes, vector<string>& excludes, string& snapshotID) override;

    virtual Error restore(const string& repo, const string& target, const string& snapshotID, vector<string>& excludes, vector<string>& includes) override;

    virtual Error listSnaps(const string& repo, vector<Snapshot>& snapshots) override;

    virtual Error removeSnapshots(const string& repo, const string& snapshots) override;

    virtual Error checkSpace(const string& repo, const vector<string>& includes, const vector<string>& excludes) override;

    virtual Error createSnapID(string& snapshotID) override;

    // 备份时检查repo是否存在，不存在则创建；查看snap时若不存在repo，则返回成功，snapshots置空；删除时若不存在repo，报错同不存在快照
    Error createRepo(const string& repo);

    // 可能需增加一层对错误的处理，restore时对正在使用的文件/已存在的符号链接 还原出错不应按还原失败处理，跳过这些错误
    Error resticRestore();
    Error createRepo(string& repo);
};

#endif /* __RESTIC_H */
