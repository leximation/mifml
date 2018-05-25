// mifml.h
//

/* MIFML - MIF to XML Conversion Utility
 * Copyright (C) 2004-2008 - Scott Prentice, Leximation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#ifndef abs
#include <math.h>
#endif
#include <string.h>
#include <time.h>
#include <ctype.h>


#define EXPIREMONTHSTR		"July 2008\0"
#define EXPIREMONTH			0 // 200807 //.. set to 0 for no expire
#define MIF2MIFMLVER		"1.0.1\0"
#define MIF2MIFMLAPPNAME	"MIFML Converter\0"
#define MIF2MIFMLCREATOR	"by Leximation, Inc.\0"
#define MIFMLURL			"http://www.leximation.com/tools/mifml/\0"
#define MIF2MIFMLCOPYRIGHT	"Copyright (c) 2004-2008 - Leximation, Inc.\0"
#define MIFMLEXTENSION		"mifml"
#define MIFEXTENSION		"mif"


int mifmlFileTest(char *infile);
int mif2mifml(char *mifname, char *mifmlname, char *dtdpath, int opts);
int mifml2mif(char *mifmlname, char *mifname);
int chartoent(char *instr, char *outstr, int start, int end, int maxoutlen);
int enttochar(char *str, int doEscape);
int escapechars(char *instr, char *outstr);
int asciiToFmHex(int ascval, int chartype, char *outstr);
int hextodec(char *hexstr);
void strtoupper(char *str);
int instr(char *str, char *findstr, int start, int len);
int hascontent(char *str);
