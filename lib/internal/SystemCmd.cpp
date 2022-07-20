/* 
 * We have modified the original author's code.
 * Copyright (c) [2004-2014] Novell, Inc.
 * Copyright (c) 2018 SUSE LLC
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * All Rights Reserved.
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

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ostream>
#include <fstream>
#include <sys/wait.h>
#include <string>
#include <boost/algorithm/string.hpp>
#include "SystemCmd.h"

extern char **environ;

using namespace std;

SystemCmd::SystemCmd(const string& Command_Cv, bool log_output)
	: Combine_b(false), log_output(log_output)
{
    init();
    execute( Command_Cv );
}

void SystemCmd::init()
{
    File_aC[0] = File_aC[1] = NULL;
    pfds[0].events = pfds[1].events = POLLIN;
}

SystemCmd::~SystemCmd()
{
    if( File_aC[IDX_STDOUT] )
	fclose( File_aC[IDX_STDOUT] );
    if( File_aC[IDX_STDERR] )
	fclose( File_aC[IDX_STDERR] );
}

void
SystemCmd::closeOpenFds() const
{
    int max_fd = getdtablesize();
    for( int fd = 3; fd < max_fd; fd++ )
    {
	close(fd);
    }
}

int
SystemCmd::execute(const string& Cmd_Cv)
{
    Background_b = false;
    return doExecute(Cmd_Cv);
}


int
SystemCmd::executeBackground( const string& Cmd_Cv )
{
    Background_b = true;
    return doExecute(Cmd_Cv);
}

int
SystemCmd::executeRestricted( const string& Command_Cv,
			      long unsigned MaxTimeSec, long unsigned MaxLineOut,
			      bool& ExceedTime, bool& ExceedLines )
{
   return 0;
}
