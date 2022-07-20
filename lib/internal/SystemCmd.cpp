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
    ExceedTime = ExceedLines = false;
    int ret = executeBackground( Command_Cv );
    unsigned long ts = 0;
    unsigned long ls = 0;
    unsigned long start_time = time(NULL);
    while( !ExceedTime && !ExceedLines && !doWait( false, ret ) )
    {
	if( MaxTimeSec>0 )
	{
	    ts = time(NULL)-start_time;
       	}
	if( MaxLineOut>0 )
	{
	    ls = numLines()+numLines(false,IDX_STDERR);
	}
	ExceedTime = MaxTimeSec>0 && ts>MaxTimeSec;
	ExceedLines = MaxLineOut>0 && ls>MaxLineOut;
	sleep( 1 );
    }
    if( ExceedTime || ExceedLines )
    {
	kill( Pid_i, SIGKILL );
	unsigned count = 0;
	int Status_ii;
	int Wait_ii = -1;
	while( count < 5 && Wait_ii <= 0 )
	{
	    Wait_ii = waitpid( Pid_i, &Status_ii, WNOHANG );
	    count++;
	    sleep( 1 );
	}
	/*
	r = kill( Pid_i, SIGKILL );
	count=0;
	waitDone = false;
	while( count<8 && !waitDone )
	    {
	    waitDone = doWait( false, ret );
	    count++;
	    sleep( 1 );
	    }
	*/
	Ret_i = -257;
    }
    else
    	    Ret_i = ret;
    return ret;
}

#define PRIMARY_SHELL "/bin/sh"
#define ALTERNATE_SHELL "/bin/bash"

int 
SystemCmd::doExecute( const string& Cmd )
{
    string Shell_Ci = PRIMARY_SHELL;
    if( access( Shell_Ci.c_str(), X_OK ) != 0 )
    {
	Shell_Ci = ALTERNATE_SHELL;
    }

    lastCmd = Cmd;

    File_aC[IDX_STDERR] = File_aC[IDX_STDOUT] = NULL;
    invalidate();
    int sout[2];
    int serr[2];
    bool ok_bi = true;
    if( !testmode && pipe(sout)<0 )
    {
	ok_bi = false;
    }
    if( !testmode && !Combine_b && pipe(serr)<0 )
    {
	ok_bi = false;
    }
    if( !testmode && ok_bi )
    {
	pfds[0].fd = sout[0];
	if( fcntl( pfds[0].fd, F_SETFL, O_NONBLOCK )<0 )
	{
	}
	if( !Combine_b )
	{
	    pfds[1].fd = serr[0];
	    if( fcntl( pfds[1].fd, F_SETFL, O_NONBLOCK )<0 )
	    {
	    }
	}

	const vector<const char*> env = make_env();

	switch( (Pid_i = fork()) )
	{
	    case 0:
		if( dup2( sout[1], STDOUT_FILENO )<0 )
		{
		}
		if( !Combine_b && dup2( serr[1], STDERR_FILENO )<0 )
		{
		}
		if( Combine_b && dup2( STDOUT_FILENO, STDERR_FILENO )<0 )
		{
		}
		if( close( sout[0] )<0 )
		{
		}
		if( !Combine_b && close( serr[0] )<0 )
		{
		}
		closeOpenFds();
		Ret_i = execle(Shell_Ci.c_str(), Shell_Ci.c_str(), "-c", Cmd.c_str(), nullptr, &env[0]);
		break;
	    case -1:
		Ret_i = -1;
		break;
	    default:
		if( close( sout[1] )<0 )
		{
		}
		if( !Combine_b && close( serr[1] )<0 )
		{
		}
		Ret_i = 0;
		File_aC[IDX_STDOUT] = fdopen( sout[0], "r" );
		if( File_aC[IDX_STDOUT] == NULL )
		{
		}
		if( !Combine_b )
		{
		    File_aC[IDX_STDERR] = fdopen( serr[0], "r" );
		    if( File_aC[IDX_STDERR] == NULL )
		    {
		    }
		}
		if( !Background_b )
		{
    			doWait( true, Ret_i );
		}
		break;
	}
    }
    else if( !testmode )
    {
	Ret_i = -1;
    }
    else
    {
	Ret_i = 0;
    }
    if( Ret_i == -127 || Ret_i == -1 )
    {
    }
    if( !testmode )
    	    checkOutput();
    if (Ret_i != 0 && log_output)
    	    logOutput();
    return Ret_i;
}

bool
SystemCmd::doWait( bool Hang_bv, int& Ret_ir )
{
    int Wait_ii;
    int Status_ii;

    do
    {
	int sel = poll( pfds, Combine_b?1:2, 1000 );
	if (sel < 0)
	{
	}
	if( sel>0 )
	{
    		checkOutput();
	}
	Wait_ii = waitpid( Pid_i, &Status_ii, WNOHANG );
    }while( Hang_bv && Wait_ii == 0 );

    if( Wait_ii != 0 )
    {
	checkOutput();
	fclose( File_aC[IDX_STDOUT] );
	File_aC[IDX_STDOUT] = NULL;
	if( !Combine_b )
	{
	    fclose( File_aC[IDX_STDERR] );
	    File_aC[IDX_STDERR] = NULL;
	}
	if (WIFEXITED(Status_ii))
	{
	    Ret_ir = WEXITSTATUS(Status_ii);
	    if (Ret_ir == 126) {}
	    else if (Ret_ir == 127) {}
	}
	else
	{
	    Ret_ir = -127;
	}
    }
    return Wait_ii != 0;
}

void
SystemCmd::setCombine(bool val)
{
    Combine_b = val;
}

void
SystemCmd::setTestmode(bool val)
{
    testmode = val;
}

unsigned
SystemCmd::numLines( bool Sel_bv, OutputStream Idx_iv ) const
{
    unsigned Ret_ii;

    if( Idx_iv > 1 )
    {
    }
    if( Sel_bv )
    {
	Ret_ii = SelLines_aC[Idx_iv].size();
    }
    else
    {
	Ret_ii = Lines_aC[Idx_iv].size();
    }
    return Ret_ii;
}

string
SystemCmd::getLine( unsigned Nr_iv, bool Sel_bv, OutputStream Idx_iv ) const
{
    string ret;

    if( Idx_iv > 1 )
    {
    }
    if( Sel_bv )
    {
	if( Nr_iv < SelLines_aC[Idx_iv].capacity() )
	{
	    ret = *SelLines_aC[Idx_iv][Nr_iv];
	}
    }
    else
    {
	if( Nr_iv < Lines_aC[Idx_iv].size() )
	{
	    ret = Lines_aC[Idx_iv][Nr_iv];
	}
    }
    return ret;
}

void
SystemCmd::invalidate()
{
    for (int Idx_ii = 0; Idx_ii < 2; Idx_ii++)
    {
	SelLines_aC[Idx_ii].resize(0);
	Lines_aC[Idx_ii].clear();
	NewLineSeen_ab[Idx_ii] = true;
    }
}

void
SystemCmd::checkOutput()
{
	return ;
}
