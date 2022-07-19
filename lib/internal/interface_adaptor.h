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

#ifndef __INTERFACE_ADAPTOR_H
#define __INTERFACE_ADAPTOR_H

#include <string>
#include <ctime>
#include <vector>
#include <memory>
#include <iostream>
#include "../ubackup.h"

using namespace std;
using namespace ubackup;

class BackupTool {

public:
    static BackupTool createRestic();
    static BackupTool createRsync();

    // backup因空间不足失败时，应报错：空间不足，然后清除本次失败的备份
    Error backup(const string& repo, vector<string>& includes, vector<string>& excludes, string& snapshotID) {
        return impl->backup(repo, includes, excludes, snapshotID);
    }

    Error restore(const string& repo, const string& target, const string& snapshotID, vector<string>& excludes, vector<string>& includes) {
        return impl->restore(repo, target, snapshotID, excludes, includes);
    }

    Error listSnaps(const string& repo, vector<Snapshot>& snapshots) {
        return impl->listSnaps(repo, snapshots);
    }

    Error removeSnapshots(const string& repo, const string& snapshots) {
        return impl->removeSnapshots(repo, snapshots);
    }

    void saveSnapshotInfo(const string& repo, const Snapshot& snapshot);

    Error checkSpace(const string& repo, const vector<string>& includes, const vector<string>& excludes) {
        return impl->checkSpace(repo, includes, excludes);
    }

    Error createSnapID(string& snapshotID) {
        return impl->createSnapID(snapshotID);
    }

public:

    class Impl
    {
    public:
        virtual ~Impl() {};
    /**
     * backup: backup creates a new snapshot and saves the files and directories.
     * repo: repository of snapshots
     * dir[]: the directories or files to backup
     * excludes[]: exclude patterns
     * *snapshotID: backup returns a snapshot ID for this backups.
     * */
    virtual Error backup(const string& repo, vector<string>& includes, vector<string>& excludes, string& snapshotID) = 0;

    /**
     * restore: extracts the data from a snapshot from the repository to a directory.
     * repo: repository of snapshots
     * target: directory to extract data to
     * snapshotID: backup returns a snapshot ID for this backups.
     * dir[]: the directories or files to backup
     * excludes[]: exclude patterns
     * includes[]: include patterns, exclude everything else
     * */
    virtual Error restore(const string& repo, const string& target, const string& snapshotID, vector<string>& excludes, vector<string>& includes) = 0;

    /**
     * listSnaps: lists all snapshots stored in the repository.
     * repo: repository of snapshots
     * snapshots[]: a list of Snapshot, which is the state of a resource at one point in time.
     * */
    virtual Error listSnaps(const string& repo, vector<Snapshot>& snapshots) = 0;

    /**
     * listSnaps: lists all snapshots stored in the repository.
     * repo: repository of snapshots
     * snapshots[]: a list of snapshot to be removed.
     * */
    virtual Error removeSnapshots(const string& repo, const string& snapshots) = 0;

    virtual Error checkSpace(const string& repo, const vector<string>& includes, const vector<string>& excludes) = 0;

    // 
    virtual Error createSnapID(string& snapshotID) = 0;

    };

    BackupTool(Impl* impl) : impl(impl) {}

    Impl& get_impl() { return *impl; }
    const Impl& get_impl() const { return *impl; }

private:

    unique_ptr<Impl> impl;
};

#endif /* __INTERFACE_ADAPTOR_H */
