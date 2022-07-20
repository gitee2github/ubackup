#!/bin/bash
# Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of version 2 of the GNU General Public License as published by 
# the Free Software Foundation.
#  
# This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY; 
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
# See the GNU General Public License for more details.
#  
# You should have received a copy of the GNU General Public License along with this program; 
# If not, see <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>.
# 
# To contact us about this file by physical or electronic mail, 
# you may find current contact information at https://www.uniontech.com/.
 
backupid=$1
if [ "X$backupid" = "X" ] ;then
    exit 1
fi
. /etc/ubackup/profile.d/ubackuplib.sh
log_debug '--BEGIN to restore--'
# this to thread operator this log file ,as so we can use as follwer
function env_Print() {
    /usr/sbin/lvdisplay               >> /var/log/ubackup.log
    /usr/bin/mount                    >> /var/log/ubackup.log
    /usr/bin/ls  -l /dev/dm-*         >> /var/log/ubackup.log
    /usr/bin/ls  -l /dev/disk/by-uuid >> /var/log/ubackup.log
    /usr/bin/ls  -l /dev/disk/by-id/  >> /var/log/ubackup.log
    /usr/bin/ls  -l /dev/mapper/      >> /var/log/ubackup.log
}
#
have_lvm
lvmflg=$?
if [ $lvmflg -eq 1 ]; then
    log_debug '--have detect lvm infomation--'
    list=$(get_unactive_volume_list)
    active_unactive_logical_volume $list
    log_debug "--after active_unactive_logical_volume,return value is :$?--"
    log_debug "--the unactive logical volume is $list--"
fi

log_debug "--According /etc/fstab to processing mount point begin--"
/usr/bin/mount -a
log_debug "--According /etc/fstab to processing mount point end,return value is :$?--"

type=$(/usr/bin/ubackup list |awk -F"|" '{if ($1 ~ "'$backupid'") {print $3}}')
if [ "X$type" = "X" ];then
    log_debug "--the backup id is NULL--"
    umount_point
    if [ $lvmflg -eq 1 ];then
        unactive_all_logical_volume $list
    fi
    grub2_menu_restore
    log_debug '--End to restore--'
    exit 1
fi
log_debug "--begin to exec restore command--"
/usr/bin/ubackup restore $type -n $backupid
log_debug "--the exe command is: \"/usr/bin/ubackup restore $type -n $backupid\",return value is :$?--"
log_debug "--end to exec restore command--"

log_debug "--begin to exec umount_point function--"
umount_point
log_debug "--end to exec umount_point function,return value is :$?--"
if [ $lvmflg -eq 1 ];then
    unactive_all_logical_volume $list
    log_debug "--after unactive_all_logical_volume,return value is :$?--"
fi
grub2_menu_restore
log_debug '--End to restore--'
exit 0
