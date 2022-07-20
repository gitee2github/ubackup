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
