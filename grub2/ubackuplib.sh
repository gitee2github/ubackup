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
 
DEBUG=on

function log_debug() {
    if [ "X$DEBUG" = "Xoff" ];then
    	echo "$@"
    else
	    echo "$(date "+%Y-%m-%d %H:%M:%S") :$@" >> /var/log/ubackup.log
    fi
}

function have_lvm() {
    lvs_info=`/usr/sbin/lvs`
    if [ "X$lvs_info" = "X" ] ;then
        ret=0
    else
        ret=1
    fi
    return $ret
}

function backup_restore_mount() {
    mount -o remount,rw /dev/mapper/uos-root
    mount -n -tproc none /sysroot/proc
    mount -n -tsysfs none /sysroot/sys
    mount -n -tdevtmpfs  none /sysroot/dev
    mount -n -tdevpts  none /sysroot/dev/pts
    mount -n -ttmpfs none /sysroot/dev/shm
}

function backup_restore_umount() {
    umount /sysroot/proc
    umount /sysroot/sys
    umount /sysroot/dev/pts
    umount /sysroot/dev/shm
    umount /sysroot/dev
    umount /sysroot/boot/efi
    umount /sysroot/boot
    mount -o remount,ro /dev/mapper/uos-root
}

function active_unactive_logical_volume() {
    if [ "X$@" = "X" ] ;then
        return 1
    fi
    for volume in "$@"
    do
        /usr/sbin/lvchange -ay $volume
    done
    return 0
}

#this funciton must call after have_lvm fun called
function get_unactive_volume_list() {
    local list=''
    for volume in `/usr/sbin/lvs|sed '1d'|awk '{print $2"/"$1}'`
    do
        stat=`/usr/sbin/lvdisplay $volume|grep 'LV Status'|awk '{print $3}'`
        if [ "$stat" = "NOT" ] ;then
            list="$list $volume"
        fi
    done
    echo $list
    return 
}

function active_all_logical_volume() {
    have_lvm
    if [ $? -eq 0 ] ;then 
        return 1
    fi
    vgname=`vgs|sed '1d'|awk '{print $1}'`
    /usr/sbin/vgchange -ay $vgname
    return 0
}

function unactive_all_logical_volume() {
    have_lvm
    if [ $? -eq 0 ] ;then 
        return 1
    fi
    for list in `get_unactive_volume_list`
    do
        /usr/sbin/lvchange -an $list
    done
    return 0
}

function umount_point() {
    for point in `cat /etc/fstab |grep -vE "^($|#)"|awk '{print $2}'|grep -E -v '^/$|none'`
    do
        umount $point >/dev/null 2>&1
    done
    return 0
}