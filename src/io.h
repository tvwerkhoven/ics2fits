/*
 io.h -- input/output routines
 Copyright (C) 2008--2010 Tim van Werkhoven <t.i.m.vanwerkhoven@xs4all.nl>
 Copyright (C) 2009 Michiel van Noort
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __IO_H__
#define __IO_H__

#include <string>
#include <stdio.h>
#include <unistd.h>

// Logging flags
#define IO_NOID         0x00000100      //!< Do not add loglevel string
#define IO_FATAL        0x00000200      //!< Fatal, quit immediately
#define IO_RETURN       0x00000400      //!< Give a non-zero return code
#define IO_NOLF         0x00000800      //!< Do not add linefeed

// Logging levels
#define IO_ERR          0x00000001 | IO_RETURN
#define IO_WARN         0x00000002
#define IO_INFO         0x00000003
#define IO_XNFO         0x00000004
#define IO_DEB1         0x00000005
#define IO_DEB2         0x00000006
#define IO_LEVEL_MASK   0x000000FF
#define IO_MAXLEVEL     0x00000006

using namespace std;

class Io {
	int verb, level_mask;
	FILE *termfd;
	FILE *logfd;
	string logfile;
	
public:
	Io() { Io::init(IO_MAXLEVEL); }
	Io(int l) { Io::init(l); }
	~Io();
	
	void init(int);

	int msg(int, const char*, ...);
	
	int setLogfile(string);
	
	int getVerb() { return verb; }
	int setVerb(int l) { verb = max(1, min(l, IO_MAXLEVEL)); return verb; }
	int setVerb(string l) { return setVerb((int) strtoll(l.c_str(), NULL, 0)); }
	
	int incVerb() { return setVerb(verb+1); }
	int decVerb() { return setVerb(verb-1); }
};

#endif
