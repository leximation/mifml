// mifmlutils.c
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


#include "mifml.h"



// return 0 if a MIF, 1 if a MIFML
int
mifmlFileTest(char *infile)
{
	FILE *fil;
	int outtype = -1;
	char line[30];

	if ((fil = fopen(infile, "r")) == NULL) {
		return -2;
	}
	fgets(line, sizeof(line), fil);
	// if first line starts with "<MIFFile " or "<Book " .. it's a MIF
	// if the first line starts with "<?xml " .. assume it's a MIFML
	if ((strncmp(line, "<MIFFile ", 9) == 0) || (strncmp(line, "<Book ", 6) == 0)) {
		outtype = 0;
	}
	else if (strncmp(line, "<?xml ", 6) == 0) {
		outtype = 1;
	}
	fclose(fil);

	return outtype;
}


int
instr(char *str, char *findstr, int start, int len)
{
	int i, j=0, max;
	int ok=-1, match=-1;
	if (len == 0) max = (int)strlen(str);
	else if (start+len > (int)strlen(str)) max = (int)strlen(str);
	else max = start+len;
	for (i=start; i<max; i++) {
		if (str[i] == findstr[j]) {
			j++;
			if (ok == -1) ok = i;
			if (j == (int)strlen(findstr)) match = ok;
			//break;
		}
		else {
			ok = -1;
			j = 0;
		}
	}
	return match;
}


// returns 1 if string has something other than \n \r \t or ' '
//
int
hascontent(char *str)
{
	int ret = 0;
	int i;
	for (i=0; i<(int)strlen(str); i++) {
		if ((str[i] != ' ') && (str[i] != '\n') && (str[i] != '\r') &&
			(str[i] != '\t') && (str[i] != '\0'))
			ret = 1;
	}
	return ret;
}


void
strtoupper(char *str)
{
	int i;
	for (i=0; i<(int)strlen(str); i++) {
		str[i] = toupper(str[i]);
	}
}



// hextodec

int
hextodec(char *hexstr)
{
	int i, j, k, num, dec=0, c;
	char cstr[2];
	int base = 16;

	k = strlen(hexstr);
	j = k-1;
	for (i=0; i<k; i++) {
		c = hexstr[i];
		if (c > '9') {
			c = tolower(c);
			num = c - 87; // reduce it to 10 thru 15
		}
		else {
			sprintf(cstr, "%c\0", c);
			num = atoi(cstr);
		}
		dec += num * (int)pow(base,j);
		j--;
	}
	return dec;
}

//=============================================================================
// enttochar - replaces certain entity definitions with char values
//
// returns the length of new string (including null terminator)
//
int
enttochar(char *str, int doEscape)
{
	int j=0, i;

	for( i=0; i<(int)strlen(str); i++ ) {
		switch( str[i] ) {
			case '&':
				if( str[i+1]=='#' ) {
					int c, cc, k=0;
					int ok = 1;
					int ishex = 0;
					char tmpstr[9];
					char outhex[9];

					if( str[i+2]=='x' ) {
						ishex = 1;
						//tmpstr[k++] = '\\';
						//tmpstr[k++] = 'x';
					}
					while (ok) {
						c = str[i+k+2+ishex];
						if (((ishex == 1) && ((c >= '0') && (c <= '9'))) ||
							((ishex == 0) && ((c >= '0') && (c <= '9')) ||
							((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'))))
							tmpstr[k++] = c;
						else
							ok = 0;
						if (k >= 8) ok = 0;
					}
					tmpstr[k++] = '\0';
					i = i + k + ishex;
					if (c == ';') i++;
					if (ishex) {
						cc = hextodec(tmpstr);
					}
					else {
						cc = atoi(tmpstr);
					}
					asciiToFmHex( cc, 0, outhex );
					str[j++] = outhex[0];
					str[j++] = outhex[1];
					str[j++] = outhex[2];
					str[j++] = outhex[3];
					str[j++] = ' ';
				}


				// swap &lt; with '<'
				else if( (str[i+1]=='l') && (str[i+2]=='t') && (str[i+3]==';') ) {
					str[j++] = '<';
					i = i+3;
				}
				// swap &gt; with '>'
				else if( (str[i+1]=='g') && (str[i+2]=='t') && (str[i+3]==';') ) {
					if (doEscape)
						str[j++] = '\\';  // TODO (DONE?): is this duplicating the efforts of escapechars?
					str[j++] = '>';
					i = i+3;
				}
				// swap &amp; with '&'
				else if( (str[i+1]=='a') && (str[i+2]=='m') && (str[i+3]=='p') && (str[i+4]==';') ) {
					str[j++] = '&';
					i = i+4;
				}
				// swap &apos; with '\''
				else if( (str[i+1]=='a') && (str[i+2]=='p') && (str[i+3]=='o') && (str[i+4]=='s') && (str[i+5]==';') ) {
					str[j++] = '\'';
					i = i+5;
				}
				// swap &quot; with '"'
				else if( (str[i+1]=='q') && (str[i+2]=='u') && (str[i+3]=='o') && (str[i+4]=='t') && (str[i+5]==';') ) {
					str[j++] = '"';
					i = i+5;
				}
				// swap &quote; with '"'
				else if( (str[i+1]=='q') && (str[i+2]=='u') && (str[i+3]=='o') && (str[i+4]=='t') && (str[i+5]=='e') && (str[i+6]==';') ) {
					str[j++] = '"';
					i = i+6;
				}
				// swap &nbsp; with ' '
				else if( (str[i+1]=='n') && (str[i+2]=='b') && (str[i+3]=='s') && (str[i+4]=='p') && (str[i+5]==';') ) {
					str[j++] = ' ';
					i = i+5;
				}
				break;
			default:
				str[j++] = str[i];
				break;
		}
	}
	str[j] = '\0';

	return j;
}


//=============================================================================
// chartoent - replaces certain characters with entity definitions
//
// returns the length of outstr (including null terminator)
//
//  set 'start' and 'end' to 0 to process all of instr
//
int
chartoent(char *instr, char *outstr, int start, int end, int maxoutlen)
{
	int i, j, c, d, inlen, outlen=0, entchar=0;

	// get size of instr "INLEN"
	inlen = strlen(instr);
	//
	if (end == 0) end = inlen;
	// loop through instr and count chars that need to be "entified"
	for (i=start; i<end; i++) {
		c = instr[i];
		d = instr[i+1];
		if ((c == '<') || (c == '>')) outlen = outlen+4;
		else if (c == '&') outlen = outlen+5;
		else if (c == 39) outlen = outlen+6;  // apostrophe (single quote)
		// disable multi-space processing .. is this OK? .. I think so.
		//else if ((c == ' ') && (d == ' ')) outlen = outlen+6;
		else if (c == '"') outlen = outlen+6;
		else outlen++;
	}
	outlen++;

	// if new length exceeds maxoutlen, set outstr to instr and return negative value
	if (outlen > maxoutlen) {
		strcpy(outstr, instr);
		return inlen * -1;
	}

	j = 0;
	for( i=start; i<end; i++ ) {
		c = instr[i];
		switch( c ) {
			case '<':
				outstr[j++] = '&';
				outstr[j++] = 'l';
				outstr[j++] = 't';
				outstr[j++] = ';';
				break;
			case '>':
				if (instr[i-1] == '\\') j--; // back up and overwrite backslash .. TODO: IS THIS OK IN ALL CASES??
				outstr[j++] = '&';
				outstr[j++] = 'g';
				outstr[j++] = 't';
				outstr[j++] = ';';
				break;
			case '&':
				outstr[j++] = '&';
				outstr[j++] = 'a';
				outstr[j++] = 'm';
				outstr[j++] = 'p';
				outstr[j++] = ';';
				break;
			case 39:  // apostrophe (single quote)
				outstr[j++] = '&';
				outstr[j++] = 'a';
				outstr[j++] = 'p';
				outstr[j++] = 'o';
				outstr[j++] = 's';
				outstr[j++] = ';';
				break;
			case '"':
				outstr[j++] = '&';
				outstr[j++] = 'q';
				outstr[j++] = 'u';
				outstr[j++] = 'o';
				outstr[j++] = 't';
				outstr[j++] = ';';
				break;
/*
			case ' ':
				if (instr[i+1] == ' ') {
					outstr[j++] = '&';
					outstr[j++] = 'n';
					outstr[j++] = 'b';
					outstr[j++] = 's';
					outstr[j++] = 'p';
					outstr[j++] = ';';
				}
				else {
					outstr[j++] = c;
				}
				break;
*/
			case '\\':
				if (instr[i+1] == 'Q') {
					outstr[j++] = '`';
					i++;
				}
//				else if (instr[i+1] == '\\') {  // collapse double backslashes
//					outstr[j++] = '\\';
//					i++;
//				}
				else {
					outstr[j++] = c;
				}
				break;
			default:
				outstr[j++] = c;
				break;
		}
	}
	outstr[j] = '\0';

	return j;
}


//=============================================================================
// escapechars - escapes certain characters as needed
//
// returns the length of outstr or -1 if unable to realloc
//
int
escapechars(char *instr, char *outstr)
{
	int j, i, c, escchar=0, hexchar=0, inlen, outlen;

	// get size of instr "INLEN"
	inlen = strlen(instr);
	// loop through instr and count chars that need to be escaped "ESCNUM"
	for( i=0; i<(int)inlen; i++ ) {
		c = instr[i];
		if (c < 0) hexchar++;
// DON'T ESCAPE BACKSLASHES
//		else if ((c == '\\') || (c == '`') || (c == '>')) escchar++;
		else if ((c == '`') || (c == '>')) escchar++;
	}
	outlen = inlen + (hexchar * 5) + escchar + 1;
	if ((escchar + hexchar) == 0) {
		strcpy(outstr, instr);
		return inlen;
	}
	if (outlen > 1023) return -1;
	// resize outstr to it is INLEN + ESCNUM*5 chars in length
//	if( (outstr = (char*)realloc(outstr, outlen * sizeof( char )) ) ==  NULL )
//      return -1;

	j = 0;
	for( i=0; i<(int)inlen; i++ ) {
		c = instr[i];
// DON'T ESCAPE BACKSLASHES
//		if ((c == '\\') || (c == '`') || (c == '>')) {
		if ((c == '`') || (c == '>')) {
			switch( c ) {
				case '>':
					outstr[j++] = '\\';
					outstr[j++] = '>';
					break;
				case '\\':
					if (instr[i+1] != 'x') {
						outstr[j++] = '\\';
						outstr[j++] = '\\';
					}
					else
						outstr[j++] = c;
					break;
				case '`':
					outstr[j++] = '\\';
					//outstr[j++] = '`';
					outstr[j++] = 'Q';
					break;
				default:
					break;
			}
		}
		else if (c < 0) {
			char hex[5];
			int cc;
			cc = c + 256;
			//sprintf(hex, "\\x%x2\0", cc);
			//printf("X: %d ", cc);

			asciiToFmHex( cc, 0, hex );
			outstr[j++] = hex[0];
			outstr[j++] = hex[1];
			outstr[j++] = hex[2];
			outstr[j++] = hex[3];

			outstr[j++] = ' ';
		}
		else {
			outstr[j++] = c;
		}
	}
	outstr[j] = '\0';

	return j;
}


int
asciiToFmHex(int ascval, int chartype, char *outstr)
{
	//char outstr[5];
	int cc = ascval;
	int j = 0;

	outstr[j++] = '\\';
	outstr[j++] = 'x';
	switch( cc ) {
//130 \xe2
		case 130:
			outstr[j++] = 'e';
			outstr[j++] = '2';
			break;
//131 \xc4
		case 131:
			outstr[j++] = 'c';
			outstr[j++] = '4';
			break;
//132 \xe3
		case 132:
			outstr[j++] = 'e';
			outstr[j++] = '3';
			break;
//133 \xc9
				case 133:
					outstr[j++] = 'c';
					outstr[j++] = '9';
					break;
//134 \xa0
				case 134:
					outstr[j++] = 'a';
					outstr[j++] = '0';
					break;
//135 \xe0
				case 135:
					outstr[j++] = 'e';
					outstr[j++] = '0';
					break;
//136 \xf6
				case 136:
					outstr[j++] = 'f';
					outstr[j++] = '6';
					break;
//137 \xe4
				case 137:
					outstr[j++] = 'e';
					outstr[j++] = '4';
					break;
//138 \xb3
				case 138:
					outstr[j++] = 'b';
					outstr[j++] = '3';
					break;
//139 \xdc
				case 139:
					outstr[j++] = 'd';
					outstr[j++] = 'c';
					break;
//140 \xce
				case 140:
					outstr[j++] = 'c';
					outstr[j++] = 'e';
					break;
//145 \xd4
				case 145:
					outstr[j++] = 'd';
					outstr[j++] = '4';
					break;
//146 \xd5
				case 146:
					outstr[j++] = 'd';
					outstr[j++] = '5';
					break;
//147 \xd2
				case 147:
					outstr[j++] = 'd';
					outstr[j++] = '2';
					break;
//148 \xd3
				case 148:
					outstr[j++] = 'd';
					outstr[j++] = '3';
					break;
//149 \xa5
				case 149:
					outstr[j++] = 'a';
					outstr[j++] = '5';
					break;
//150 \xd0
				case 150:
					outstr[j++] = 'd';
					outstr[j++] = '0';
					break;
//151 \xd1
				case 151:
					outstr[j++] = 'd';
					outstr[j++] = '1';
					break;
//152 \xf7
				case 152:
					outstr[j++] = 'f';
					outstr[j++] = '7';
					break;
//153 \xaa
				case 153:
					outstr[j++] = 'a';
					outstr[j++] = 'a';
					break;
//154 \xf0
				case 154:
					outstr[j++] = 'f';
					outstr[j++] = '0';
					break;
//155 \xdd
				case 155:
					outstr[j++] = 'd';
					outstr[j++] = 'd';
					break;
//156 \xcf
				case 156:
					outstr[j++] = 'c';
					outstr[j++] = 'f';
					break;
//159 \xd9
				case 159:
					outstr[j++] = 'd';
					outstr[j++] = '9';
					break;
//161 \xc1
				case 161:
					outstr[j++] = 'c';
					outstr[j++] = '1';
					break;
//162 \xa2
				case 162:
					outstr[j++] = 'a';
					outstr[j++] = '2';
					break;
//163 \xa3
				case 163:
					outstr[j++] = 'a';
					outstr[j++] = '3';
					break;
//164 \xdb
				case 164:
					outstr[j++] = 'd';
					outstr[j++] = 'b';
					break;
//165 \xb4
				case 165:
					outstr[j++] = 'b';
					outstr[j++] = '4';
					break;
//166 \xad
				case 166:
					outstr[j++] = 'a';
					outstr[j++] = 'd';
					break;
//167 \xa4
				case 167:
					outstr[j++] = 'a';
					outstr[j++] = '4';
					break;
//168 \xac
				case 168:
					outstr[j++] = 'a';
					outstr[j++] = 'c';
					break;
//169 \xa9
				case 169:
					outstr[j++] = 'a';
					outstr[j++] = '9';
					break;
//170 \xbb
				case 170:
					outstr[j++] = 'b';
					outstr[j++] = 'b';
					break;
//171 \xc7
				case 171:
					outstr[j++] = 'c';
					outstr[j++] = '7';
					break;
//172 \xc2
				case 172:
					outstr[j++] = 'c';
					outstr[j++] = '2';
					break;
//173 \x2d
				case 173:
					outstr[j++] = '2';
					outstr[j++] = 'd';
					break;
//174 \xa8
				case 174:
					outstr[j++] = 'a';
					outstr[j++] = '8';
					break;
//175 \xf8
				case 175:
					outstr[j++] = 'f';
					outstr[j++] = '8';
					break;
//176 \xfb
				case 176:
					outstr[j++] = 'f';
					outstr[j++] = 'b';
					break;
//177 \xb1
				case 177:
					outstr[j++] = 'b';
					outstr[j++] = '1';
					break;
//178 \xb7
				case 178:
					outstr[j++] = 'b';
					outstr[j++] = '7';
					break;
//179 \xb8
				case 179:
					outstr[j++] = 'b';
					outstr[j++] = '8';
					break;
//180 \xab
				case 180:
					outstr[j++] = 'a';
					outstr[j++] = 'b';
					break;
//181 \xb5
				case 181:
					outstr[j++] = 'b';
					outstr[j++] = '5';
					break;
//182 \xa6
				case 182:
					outstr[j++] = 'a';
					outstr[j++] = '6';
					break;
//183 \xe1
				case 183:
					outstr[j++] = 'e';
					outstr[j++] = '1';
					break;
//184 \xfc
				case 184:
					outstr[j++] = 'f';
					outstr[j++] = 'c';
					break;
//185 \xb6
				case 185:
					outstr[j++] = 'b';
					outstr[j++] = '6';
					break;
//186 \xbc
				case 186:
					outstr[j++] = 'b';
					outstr[j++] = 'c';
					break;
//187 \xc8
				case 187:
					outstr[j++] = 'c';
					outstr[j++] = '8';
					break;
//188 \xb9
				case 188:
					outstr[j++] = 'b';
					outstr[j++] = '9';
					break;
//189 \xba
				case 189:
					outstr[j++] = 'b';
					outstr[j++] = 'a';
					break;
//190 \xbd
				case 190:
					outstr[j++] = 'b';
					outstr[j++] = 'd';
					break;
//191 \xc0
				case 191:
					outstr[j++] = 'c';
					outstr[j++] = '0';
					break;
//192 \xcb
				case 192:
					outstr[j++] = 'c';
					outstr[j++] = 'b';
					break;
//193 \xe7
				case 193:
					outstr[j++] = 'e';
					outstr[j++] = '7';
					break;
//194 \xe5
				case 194:
					outstr[j++] = 'e';
					outstr[j++] = '5';
					break;
//195 \xcc
				case 195:
					outstr[j++] = 'c';
					outstr[j++] = 'c';
					break;
//196 \x80
				case 196:
					outstr[j++] = '8';
					outstr[j++] = '0';
					break;
//197 \x81
				case 197:
					outstr[j++] = '8';
					outstr[j++] = '1';
					break;
//198 \xae
				case 198:
					outstr[j++] = 'a';
					outstr[j++] = 'e';
					break;
//199 \x82
				case 199:
					outstr[j++] = '8';
					outstr[j++] = '2';
					break;
//200 \xe9
				case 200:
					outstr[j++] = 'e';
					outstr[j++] = '9';
					break;
//201 \x83
				case 201:
					outstr[j++] = '8';
					outstr[j++] = '3';
					break;
//202 \xe6
				case 202:
					outstr[j++] = 'e';
					outstr[j++] = '6';
					break;
//203 \xe8
				case 203:
					outstr[j++] = 'e';
					outstr[j++] = '8';
					break;
//204 \xed
				case 204:
					outstr[j++] = 'e';
					outstr[j++] = 'd';
					break;
//205 \xea
				case 205:
					outstr[j++] = 'e';
					outstr[j++] = 'a';
					break;
//206 \xeb
				case 206:
					outstr[j++] = 'e';
					outstr[j++] = 'b';
					break;
//207 \xec
				case 207:
					outstr[j++] = 'e';
					outstr[j++] = 'c';
					break;
//208 \xc3
				case 208:
					outstr[j++] = 'c';
					outstr[j++] = '3';
					break;
//209 \x84
				case 209:
					outstr[j++] = '8';
					outstr[j++] = '4';
					break;
//210 \xf1
				case 210:
					outstr[j++] = 'f';
					outstr[j++] = '1';
					break;
//211 \xee
				case 211:
					outstr[j++] = 'e';
					outstr[j++] = 'e';
					break;
//212 \xef
				case 212:
					outstr[j++] = 'e';
					outstr[j++] = 'f';
					break;
//213 \xcd
				case 213:
					outstr[j++] = 'c';
					outstr[j++] = 'd';
					break;
//214 \x85
				case 214:
					outstr[j++] = '8';
					outstr[j++] = '5';
					break;
//215 \xb0
				case 215:
					outstr[j++] = 'b';
					outstr[j++] = '0';
					break;
//216 \xaf
				case 216:
					outstr[j++] = 'a';
					outstr[j++] = 'f';
					break;
//217 \xf4
				case 217:
					outstr[j++] = 'f';
					outstr[j++] = '4';
					break;
//218 \xf2
				case 218:
					outstr[j++] = 'f';
					outstr[j++] = '2';
					break;
//219 \xf3
				case 219:
					outstr[j++] = 'f';
					outstr[j++] = '3';
					break;
//220 \x86
				case 220:
					outstr[j++] = '8';
					outstr[j++] = '6';
					break;
//221 \xc5
				case 221:
					outstr[j++] = 'c';
					outstr[j++] = '5';
					break;
//222 \xd7
				case 222:
					outstr[j++] = 'd';
					outstr[j++] = '7';
					break;
//223 \xa7
				case 223:
					outstr[j++] = 'a';
					outstr[j++] = '7';
					break;
//224 \x88
				case 224:
					outstr[j++] = '8';
					outstr[j++] = '8';
					break;
//225 \x87
				case 225:
					outstr[j++] = '8';
					outstr[j++] = '7';
					break;
//226 \x89
		case 226:
			outstr[j++] = '8';
			outstr[j++] = '9';
			break;
//227 \x8b
		case 227:
			outstr[j++] = '8';
			outstr[j++] = 'b';
			break;
//228 \x8a
		case 228:
			outstr[j++] = '8';
			outstr[j++] = 'a';
			break;
//229 \x8c
		case 229:
			outstr[j++] = '8';
			outstr[j++] = 'c';
			break;
//230 \xbe
		case 230:
			outstr[j++] = 'b';
			outstr[j++] = 'e';
			break;
//231 \x8d
		case 231:
			outstr[j++] = '8';
			outstr[j++] = 'd';
			break;
//232 \x8f
		case 232:
			outstr[j++] = '8';
			outstr[j++] = 'f';
			break;
//233 \x8e
		case 233:
			outstr[j++] = '8';
			outstr[j++] = 'e';
			break;
//234 \x90
		case 234:
			outstr[j++] = '9';
			outstr[j++] = '0';
			break;
//235 \x91
		case 235:
			outstr[j++] = '9';
			outstr[j++] = '1';
			break;
//236 \x92
		case 236:
			outstr[j++] = '9';
			outstr[j++] = '2';
			break;
//237 \x93
		case 237:
			outstr[j++] = '9';
			outstr[j++] = '3';
			break;
//238 \x94
		case 238:
			outstr[j++] = '9';
			outstr[j++] = '4';
			break;
//239 \x95
		case 239:
			outstr[j++] = '9';
			outstr[j++] = '5';
			break;
//240 \xb2
		case 240:
			outstr[j++] = 'b';
			outstr[j++] = '2';
			break;
//241 \x96
		case 241:
			outstr[j++] = '9';
			outstr[j++] = '6';
			break;
//242 \x98
		case 242:
			outstr[j++] = '9';
			outstr[j++] = '8';
			break;
//243 \x97
		case 243:
			outstr[j++] = '9';
			outstr[j++] = '7';
			break;
//244 \x99
		case 244:
			outstr[j++] = '9';
			outstr[j++] = '9';
			break;
//245 \x9b
		case 245:
			outstr[j++] = '9';
			outstr[j++] = 'b';
			break;
//246 \x9a
		case 246:
			outstr[j++] = '9';
			outstr[j++] = 'a';
			break;
//247 \xd6
		case 247:
			outstr[j++] = 'd';
			outstr[j++] = '6';
			break;
//248 \xbf
		case 248:
			outstr[j++] = 'b';
			outstr[j++] = 'f';
			break;
//249 \x9d
		case 249:
			outstr[j++] = '9';
			outstr[j++] = 'd';
			break;
//250 \x9c
		case 250:
			outstr[j++] = '9';
			outstr[j++] = 'c';
			break;
//251 \x9e
		case 251:
			outstr[j++] = '9';
			outstr[j++] = 'e';
			break;
//252 \x9f
		case 252:
			outstr[j++] = '9';
			outstr[j++] = 'f';
			break;
//253 \xc6
		case 253:
			outstr[j++] = 'c';
			outstr[j++] = '6';
			break;
//254 \xca
		case 254:
			outstr[j++] = 'c';
			outstr[j++] = 'a';
			break;
//255 \xd8
		case 255:
			outstr[j++] = 'd';
			outstr[j++] = '8';
			break;

		default:  // if not mapped, convert to hex and use it as-is
		{
			char tmp[3];
			sprintf(tmp, "%x2\0", cc);
			outstr[j++] = tmp[0];
			outstr[j++] = tmp[1];
			break;
		}
	}
	outstr[j++] = '\0';
	return 0;
}

/*
ASCII to HEX mapping for Windows char set
0130 \xe2
0131 \xc4
0132 \xe3
0133 \xc9
0134 \xa0
0135 \xe0
0136 \xf6
0137 \xe4
0138 \xb3
0139 \xdc
0140 \xce
0145 \xd4
0146 \xd5
0147 \xd2
0148 \xd3
0149 \xa5
0150 \xd0
0151 \xd1
0152 \xf7
0153 \xaa
0154 \xf0
0155 \xdd
0156 \xcf
0159 \xd9
0161 \xc1
0162 \xa2
0163 \xa3
0164 \xdb
0165 \xb4
0166 \xad
0167 \xa4
0168 \xac
0169 \xa9
0170 \xbb
0171 \xc7
0172 \xc2
0173 \x2d
0174 \xa8
0175 \xf8
0176 \xfb
0177 \xb1
0178 \xb7
0179 \xb8
0180 \xab
0181 \xb5
0182 \xa6
0183 \xe1
0184 \xfc
0185 \xb6
0186 \xbc
0187 \xc8
0188 \xb9
0189 \xba
0190 \xbd
0191 \xc0
0192 \xcb
0193 \xe7
0194 \xe5
0195 \xcc
0196 \x80
0197 \x81
0198 \xae
0199 \x82
0200 \xe9
0201 \x83
0202 \xe6
0203 \xe8
0204 \xed
0205 \xea
0206 \xeb
0207 \xec
0208 \xc3
0209 \x84
0210 \xf1
0211 \xee
0212 \xef
0213 \xcd
0214 \x85
0215 \xb0
0216 \xaf
0217 \xf4
0218 \xf2
0219 \xf3
0220 \x86
0221 \xc5
0222 \xd7
0223 \xa7
0224 \x88
0225 \x87
0226 \x89
0227 \x8b
0228 \x8a
0229 \x8c
0230 \xbe
0231 \x8d
0232 \x8f
0233 \x8e
0234 \x90
0235 \x91
0236 \x92
0237 \x93
0238 \x94
0239 \x95
0240 \xb2
0241 \x96
0242 \x98
0243 \x97
0244 \x99
0245 \x9b
0246 \x9a
0247 \xd6
0248 \xbf
0249 \x9d
0250 \x9c
0251 \x9e
0252 \x9f
0253 \xc6
0254 \xca
0255 \xd8
*/

