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

#include "rsync.h"

Error err;
Error RsyncTool::backup(const string& repo, vector<string>& includes, vector<string>& excludes, string& snapshotID) {
    
    return err;
}

Error RsyncTool::restore(const string& repo, const string& target, const string& snapshotID, vector<string>& excludes, vector<string>& includes) {
    return err;
}

Error RsyncTool::listSnaps(const string& repo, vector<Snapshot>& snapshots) {
    return err;
}

Error RsyncTool::removeSnapshots(const string& repo, const string& snapshots) {
    return err;
}

Error RsyncTool::checkSpace(const string& repo, const vector<string>& includes, const vector<string>& excludes) {
    return err;
}

Error RsyncTool::createSnapID(string& snapshotID) {
    return err;
}

BackupTool
BackupTool::createRsync()
{
    return BackupTool(new RsyncTool());
}
