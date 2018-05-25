// mifml2mif.c
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


#define MAXVALSIZE 1024
#define MAXBUFSIZE 1024
#define MAXATTRSIZE 20
#define MAXVERSIZE 10
#define MAXEXPTYPESIZE 10
#define MAXDESCSIZE 256
#define MAXTAGSIZE 60


//=============================================================================
// mifml2mif
//
// returns:
//		-1  - unable to open mifml
//		-2  - unable to open mif
//		-99 - nesting level not correct (probably invalid XML)
//
// TODO: Use a *real* XML parser .. xerces??
// TODO: Validate against DTD ??
// TODO: Check for well formedness ??
//
// ignore <? ... ?> and <!-- ... --> and <!DOCTYPE ... >
// convert <MIFFile version="7.00" type="full" description="Generated by FrameMaker 7.1b023">
//    into <MIFFile 7.00> # Generated by Pubs-Tools Mif2MifML v.0.01 - http://www.pubs-tools.com/mifml/
//
// read one char at a time, buffer each element, then write out MIF code
//
// need to process the folowing specially .. ImportObjectData, DocFileInfo, BookFileInfo
// and Strings!

int
mifml2mif(char *mifmlname, char *mifname)
{
	FILE *miffile;
	FILE *mifmlfile;
	char line[1024];
	int c;
	char miftag[MAXTAGSIZE] = {"\0"};
	char miftype[10] = {"\0"};
	char mifval[MAXVALSIZE] = {"\0"};
	char mifattr[MAXATTRSIZE] = {"\0"};
	char mifbuf[MAXBUFSIZE] = {"\0"};
	char valbuf[MAXVALSIZE] = {"\0"};
	char padstr[20] = {"\0"};
	char mifver[MAXVERSIZE] = {"\0"};
	char mifexp[MAXEXPTYPESIZE] = {"\0"};
	char mifdesc[MAXDESCSIZE] = {"\0"};
	char impobjfacetname[10];
	char impobjdatatype[2];
	int padlevel = -1;
	int inattrval = 0; 	// in attribute value: 0==not built; 1==building; 2==built
	int inattr = 0; 	// in attribute name: 0==not built; 1==building; 2==built
	int intag  = 0;		// in tag name: 0==not built; 1==building; 2==built
	int inquot = 0;
	int elemlen = 0;
	int intextelem = 0;
	int inelem = 0;
	int attrtype = 0;	// 1==type; 2==value
	int nelems = 0;
	int dataelem = 0;
	int emptyelem = 0;
	int i, j, ch, ln;
	int impobjdata;
	int iCollapseSpace = 0;
	int lnum = 0;
	int cnum = 0;

	if( ( mifmlfile = fopen( mifmlname, "r" ) ) == NULL ) {
		return -1;
	}
	if( ( miffile = fopen( mifname, "w" )) == NULL ) {
		return -2;
	}

	while( ! feof( mifmlfile ) ) {
		c = fgetc( mifmlfile ); 	// read one char from file
		if( c == '\n' ) {
			lnum++;
			//printf("> %d\n", lnum);
		}
		if( (iCollapseSpace == 1) && (c == ' ') ) {
			//printf(".");
			//printf(".[%d]", lnum);
			continue;
		}
		else if( iCollapseSpace == 1 ) {
			iCollapseSpace = 0;
			//cnum = 0;
			//printf(" END\n");
			strcat( mifbuf, " \0" );
		}
		else {
			iCollapseSpace = 0;
			//printf("%c", c);
		}
		if( (intextelem == 1) && (c == '\n') ) {
			// ignore CRs and all following spaces in String elements ..
			iCollapseSpace = 1;
			//printf("Collapsing! ");
			continue;
		}

		if( (c == '>') && (strcmp( miftag, "String\0" ) == 0) && (intextelem == 0) ) {
			intextelem = 1;
			//printf("String!");
		}
		if( intextelem > 0 ) {
			if( (intextelem == 2) && (c == '>') ) {
				intextelem = 0;
			}
			if( c == '<' ) { // begining of closing tag
				intextelem = 2;
			}
			if( intextelem > 0 ) {
				elemlen++;
				if (strlen(mifbuf)+2 > MAXBUFSIZE) {
					printf("> WARNING: Content value for <%s> exceeds maximum length of %d!\n",
						miftag, MAXBUFSIZE);
					break;
				}
				else {
					sprintf( mifbuf, "%s%c\0", mifbuf, c );
					continue;
				}
			}
		}
		// if "dataelem" .. read to end of line, then read line by line until you hit a closing angle bracket
		// be sure to parse each line to swap entities
		if( dataelem == 1 ) {
			if( impobjdata == 0 ) {
				fprintf( miffile, "%s<%s \n\0", padstr, miftag );
			}
			ln=0;
			while( dataelem == 1 ) {
				fgets( line, sizeof( line ), mifmlfile );
				strtok( line, "\n" );
				ln++;
				if( strchr( line, '>' ) ) {
					if( impobjdata == 1 ) {
						// check for .. flag="endinset" .. in last line, if not there, then don't close inset
						if (instr(line, "\"endinset\"\0", 0, 0) > -1)
							fprintf( miffile, "=EndInset\n\n\0" );
						impobjdata = 0;
					}
					else {
						fprintf( miffile, "%s> # end of %s\n\0", padstr, miftag );
					}
					dataelem = 0;
				}
				else {
					if( ln == 1 ) { // process first line depending on element
						//printf( "> %c%s\n", c, line );
						if( impobjdata == 1 ) {
							strcpy( impobjfacetname, "BAD\0" );
							strcpy( impobjdatatype, "v\0" );
							sprintf( mifbuf, "%c%s\0", c, line );
							// printf( "> %s\n",  mifbuf );
							for( i=0; i<(int)strlen(mifbuf); i++ ) {
								ch = mifbuf[i];
								if( inattrval == 1 ) { //
									if( ch == '"' ) {  // end attrib val, assign to proper attribute variable
										if( strcmp( mifattr, "facetName\0" ) == 0 ) {
											strcpy( impobjfacetname, valbuf );
										}
										else if( strcmp( mifattr, "facetName\0" ) == 0 ) {
											strcpy( impobjdatatype, valbuf );
										}
										else {
											// ??
										}
										strcpy( mifattr, "\0" );
										strcpy( valbuf, "\0" );
										inattrval = 0;
									}
									else {
										if (strlen(valbuf)+2 > MAXVALSIZE) {
											printf("> WARNING: Size of <ImportObject> line exceeds maximum length of %d!\n",
												MAXVALSIZE);
											break;
										}
										else {
											sprintf( valbuf, "%s%c\0", valbuf, ch );
										}
									}
								}
								else {
									if( (ch == '=') || (ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r') ) {
										// ignore spaces
									}
									else if( (ch == '"') && (inattrval == 0) ) {
										inattrval = 1;
									}
									else {
										if (strlen(mifattr)+2 > MAXATTRSIZE) {
											printf("> WARNING: Size of <ImportObject> attribute exceeds maximum length of %d!\n",
												MAXATTRSIZE);
											break;
										}
										else {
											sprintf( mifattr, "%s%c\0", mifattr, ch );
										}
									}
								}
							}
							fprintf( miffile, "=%s\n&%%%s\n\0", impobjfacetname, impobjdatatype );
						}
					}
					else {
						// write line as-is
						char tmpline[1024];
						strcpy(tmpline, line);
						//escapechars( tmpline, line );
						enttochar( line, 0 );
						fprintf( miffile, "%s\n\0", line );
					}
				}
			}
			strcpy( mifattr, "\0" );
			strcpy( valbuf, "\0" );
			strcpy( mifbuf, "\0" );
			strcpy( miftag, "\0" );
			dataelem = 0;
			inelem = 0;
			inquot = 0;
			intag  = 0;
			elemlen = 0;
			intextelem = 0;
			continue;
		}
		if( (inelem == 1) && (c == '>') && (inquot == 0) && (intextelem == 0) ) {

			//if( dataelem == 1 ) {
			//	sprintf( mifbuf, "%s value=\"SKIPPED!\" type=\"str\"/\0", miftag );
			//}
			// process element
			if( (mifbuf[0] == '!') || (mifbuf[0] == '?') ) {
				// skip comments and processing instructions
			}
			else if( emptyelem == 1 ) { // empty elements other than a String
				if( strcmp( miftag, "String\0" ) == 0 ) {
					// TODO: THIS IS A HACK .. a String with space as the value collapses into an empty element ...
					//fprintf( miffile, "%s<%s ` '>\n\0", padstr, miftag );
					//printf( "HACK1: %s\n", miftag );
					// TODO: after processing this empty string, it needs to continue below ...
				}
				else {
					//  <ParaLine
					//  > # end of ParaLine
					//strcpy( padstr, "\0" );
					//for( j=0; j<padlevel; j++ ) { sprintf( padstr, "%s \0", padstr ); }
					fprintf( miffile, "%s<%s \n\0", padstr, miftag );
					fprintf( miffile, "%s> # end of %s\n\0", padstr, miftag );
					//emptyelem = 0;
					//inelem = 0;
					//printf( "END: %s\n", miftag );
				}
			}
			else {

				// locate type and value attribs
				strcpy( mifattr, "\0" );
				strcpy( valbuf, "\0" );
				strcpy( miftype, "\0" );
				strcpy( mifval, "\0" );
				inattrval = 0;
				if( strcmp( miftag, "String\0" ) == 0 ) {
					// process mifbuf to get mifval (string text) and set miftype to "str"
					for( i=(int)strlen(miftag); i<(int)strlen(mifbuf); i++ ) {
						ch = mifbuf[i];
						if( inattrval == 1 ) { //
							if( ch == '<' ) {  // end of text content
								strcpy( miftype, "str\0" );
								strcpy( mifval, valbuf );
								inattrval = 0;
								continue;
							}
							else {
								if (strlen(valbuf)+2 > MAXVALSIZE) {
									printf("> WARNING: Attribute value for <%s> exceeds maximum length of %d!\n",
										miftag, MAXVALSIZE);
									break;
								}
								else
									sprintf( valbuf, "%s%c\0", valbuf, ch );
							}
						}
						else {
							if( (ch == '>') && (inattrval == 0) ) {
								inattrval = 1;
							}
							else {
								// do nothing ??
							}
						}
					}
				}
				else {
					for( i=(int)strlen(miftag); i<(int)strlen(mifbuf); i++ ) {
						ch = mifbuf[i];
						if( inattrval == 1 ) { //
							if( ch == '"' ) {  // end attrib val, assign to proper attribute variable
								if( strcmp( mifattr, "type\0" ) == 0 ) {
									strcpy( miftype, valbuf );
								}
								else if( strcmp( mifattr, "value\0" ) == 0 ) {
									strcpy( mifval, valbuf );
								}
								else if( strcmp( mifattr, "version\0" ) == 0 ) {
									if (strlen(valbuf)+2 > MAXVERSIZE) {
										printf("> WARNING: Size of \"version\" attribute exceeds maximum length of %d!\n",
											MAXVERSIZE);
									}
									else {
										strcpy( mifver, valbuf );
									}
								}
								else if( strcmp( mifattr, "export-type\0" ) == 0 ) {
									if (strlen(valbuf)+2 > MAXEXPTYPESIZE) {
										printf("> WARNING: Size of \"export-type\" attribute exceeds maximum length of %d!\n",
											MAXEXPTYPESIZE);
									}
									else {
										strcpy( mifexp, valbuf );
									}
								}
								else if( strcmp( mifattr, "description\0" ) == 0 ) {
									if (strlen(valbuf)+2 > MAXDESCSIZE) {
										printf("> WARNING: Size of \"description\" attribute exceeds maximum length of %d!\n",
											MAXDESCSIZE);
									}
									else {
										strcpy( mifdesc, valbuf );
									}
								}
								else {
									// ??
								}
								strcpy( mifattr, "\0" );
								strcpy( valbuf, "\0" );
								inattrval = 0;
							}
							else {
								if (strlen(valbuf)+2 > MAXVALSIZE) {
									printf("> WARNING: Attribute value for <%s> exceeds maximum length of %d!\n",
										miftag, MAXVALSIZE);
									break;
								}
								else
									sprintf( valbuf, "%s%c\0", valbuf, ch );
							}
						}
						else {
							if( (ch == '=') || (ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r') ) {
								// ignore spaces
							}
							else if( (ch == '"') && (inattrval == 0) ) {
								inattrval = 1;
							}
							else {
								if (strlen(mifattr)+2 > MAXATTRSIZE) {
									printf("> WARNING: Attribute value for <%s> exceeds maximum length of %d!\n",
										miftag, MAXATTRSIZE);
									break;
								}
								else
									sprintf( mifattr, "%s%c\0", mifattr, ch );
							}
						}
					}
				}
				// replace entity names with actual chars in mifval .. &lt; -> <, etc..
				if( strcmp( mifval, "\0" ) != 0 ) {
					char tmpmifval[1024];
					strcpy(tmpmifval, mifval);
					escapechars( tmpmifval, mifval );
					enttochar( mifval, 1	 );
					//enttochar( mifval );
				}
				if( strcmp( miftype, "str\0" ) == 0 ) {
					fprintf( miffile, "%s<%s `%s'>\n\0", padstr, miftag, mifval );
				}
				else if( strcmp( miftype, "num\0" ) == 0 ) {
					fprintf( miffile, "%s<%s  %s>\n\0", padstr, miftag, mifval );
				}
				else if( strcmp( miftype, "enum\0" )  == 0 ) {
					fprintf( miffile, "%s<%s %s>\n\0", padstr, miftag, mifval );
				}
				else if( strcmp( mifval, "\0" ) != 0 ) {
					// TODO: Count and report errors.
					fprintf( miffile, "%s<%s `%s' ERROR>\n\0", padstr, miftag, mifval );
				}
				else if( mifbuf[strlen(mifbuf)-1] == '/' ) {
					// empty element
					fprintf( miffile, "%s<%s >\n\0", padstr, miftag );
				}
				else if( miftag[0] == '/' ) {
					// strip off leading slash from miftag
					for( j=0; j<(int)strlen(miftag); j++ ) {
						miftag[j] = miftag[j+1];
					}
					padlevel--;
					if( padlevel == -1 ) {
						if( strcmp( miftag, "MIFBook\0" ) == 0 ) { strcpy( miftag, "Book\0" ); }
						fprintf( miffile, "# End of %s\n\0", miftag );
					}
					else {
						strcpy( padstr, "\0" );
						for( j=0; j<padlevel; j++ ) { sprintf( padstr, "%s \0", padstr ); }
						fprintf( miffile, "%s> # end of %s\n\0", padstr, miftag );
					}
				}
				else {
					if( padlevel == -1 ) {
						if( strcmp( miftag, "MIFBook\0" ) == 0 ) { strcpy( miftag, "Book\0" ); }
						fprintf( miffile, "<%s %s> # Generated by %s v.%s %s\n\0", miftag, mifver, MIF2MIFMLAPPNAME,
							MIF2MIFMLVER, MIF2MIFMLCREATOR );
						//fprintf( miffile, "# Original MIF data:\n\0" );
						fprintf( miffile, "#    Generated from %s MIFML file\n\0", mifexp );
						fprintf( miffile, "#    Original MIF %s\n\0", mifdesc );
						fprintf( miffile, "#\n\0" );
						/*
						fprintf( miffile, "# Options:\n\0" );
						fprintf( miffile, "#    Paragraph Text\n\0" );
						fprintf( miffile, "#    Paragraph Tags\n\0" );
						fprintf( miffile, "#    Paragraph Formats\n\0" );
						fprintf( miffile, "#    Font Information\n\0" );
						fprintf( miffile, "#    Markers\n\0" );
						fprintf( miffile, "#    Anchored Frames\n\0" );
						fprintf( miffile, "#    Tables\n\0" );
						fprintf( miffile, "#    Graphics and TextRect Layout\n\0" );
						fprintf( miffile, "#    Master Page Items\n\0" );
						fprintf( miffile, "#    Condition Catalog\n\0" );
						fprintf( miffile, "#    Table Catalogs\n\0" );
						fprintf( miffile, "#    Font Catalog\n\0" );
						fprintf( miffile, "#    Paragraph Catalog\n\0" );
						fprintf( miffile, "#    Document Template\n\0" );
						fprintf( miffile, "#    Document Dictionary\n\0" );
						fprintf( miffile, "#    Variables\n\0" );
						fprintf( miffile, "#    Element Definitions\n\0" );
						fprintf( miffile, "#    Elements\n\0" );
						fprintf( miffile, "#    Format Change Lists\n\0" );
						fprintf( miffile, "#\n\0" );
						*/
					}
					else {
						fprintf( miffile, "%s<%s \n\0", padstr, miftag );
					}
					padlevel++;
					strcpy( padstr, "\0" );
					for( j=0; j<padlevel; j++ ) { sprintf( padstr, "%s \0", padstr ); }
					nelems++;
				}
				//fprintf( miffile, "%s\n\0", mifbuf );
			}
			strcpy( mifattr, "\0" );
			strcpy( valbuf, "\0" );
			strcpy( mifbuf, "\0" );
			strcpy( miftag, "\0" );
			dataelem = 0;
			inelem = 0;
			inquot = 0;
			intag  = 0;
			elemlen = 0;
			intextelem = 0;
			emptyelem = 0;
		}
		if( inelem == 1 ) {
			if( dataelem != 1 ) {
				elemlen++;
				if (strlen(mifbuf)+2 > MAXBUFSIZE) {
					printf("> WARNING: Value for <%s> exceeds maximum length of %d!\n",
						miftag, MAXBUFSIZE);
				}
				else
					sprintf( mifbuf, "%s%c\0", mifbuf, c );

				if( (intag == 1) && (c == '/') && (strcmp(miftag,"\0")!=0) ) {  // ending slash immediately after tag
					intag = 0;
					emptyelem = 1;
					//printf( "TAG: %s\n", miftag );
					if( strcmp( miftag, "String\0" ) == 0 ) {
						emptyelem = 2;  // if a String, this is set to 2
						intextelem = 2;
						// force the "EMPTY" String to have a single space .. this seems to make the most sense .. ??
						strcpy( mifbuf, "String> </String\0" );
					}
				}
				else if( (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') ) {
					intag = 0;
					if( strcmp( miftag, "ImportObjectData\0" ) == 0 ) {
						dataelem = 1;
						impobjdata = 1;
					}
					if( strcmp( miftag, "DocFileInfo\0" ) == 0 ) {
						dataelem = 1;
					}
					if( strcmp( miftag, "BookFileInfo\0" ) == 0 ) {
						dataelem = 1;
					}
				}
				else if( intag == 1 ) {
					if (strlen(miftag)+2 > MAXTAGSIZE) {
						printf("> WARNING: Length of tagname <%s> exceeds maximum size of %d!\n",
							miftag, MAXTAGSIZE);
					}
					else
						sprintf( miftag, "%s%c\0", miftag, c );
				}
			}
		}
		if( (inelem == 0) && (c == '<') ) {			// open element
			inelem = 1;
			inquot = 0;
			intag  = 1;
			emptyelem = 0;
		}
		else if( (inelem == 1) && (c == '"') && (inquot == 0) ) {
			inquot = 1;
		}
		else if( (inelem == 1) && (c == '"') && (inquot == 1) ) {
			inquot = 0;
		}
	}

	fclose( miffile );
	fclose( mifmlfile );

	if( padlevel != -1 ) {
		printf( "PADLEVEL: %d\n", padlevel );
		return -99;
	}

	return nelems;
}
