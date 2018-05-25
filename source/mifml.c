// mifml.c
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


void mifml_syntax( void );


int main(int argc, char* argv[])
{
	int ret;
	char infile[256];
	char outfile[256];
	char dtdpath[256] = {"\0"};
	char tmparg[256];
	char tmp0[256];
	char curdate[8];
	int outtype = 0;	// 0 == "MIFML"; 1 == "MIF"
	int opts;			// 0 == "full"; 1 == "partial"
	int scripted = 0;	// 0 == not scripted; 1 == scripted (no "pauses" added)
	time_t ltime;
	int c, i;
	int inset  = 0;
	int outset = 0;
	int dtdset = 0;
	int err = 0;
	char chunktype[10];

	time( &ltime );
	strftime( curdate, 7, "%Y%m\0", localtime( &ltime ) );
	if( (EXPIREMONTH > 0) && (atoi(curdate) > EXPIREMONTH) ) {
		printf("\n===============================================================================\n");
		printf(" %s v.%s %s \n", MIF2MIFMLAPPNAME, MIF2MIFMLVER, MIF2MIFMLCREATOR);
		printf("===============================================================================\n\n");
		printf(" The Beta period for this tool has expired!\n\n\n\n\n\n\n\n\n\n\n" );
		printf(" For more information, see %s\n", MIFMLURL );
		printf(" %s\n", MIF2MIFMLCOPYRIGHT );
		printf("===============================================================================\n");
#ifdef WIN32
		system( "pause" );
#endif
		ret = 0;
	}
	else {
		if( argc <= 1 ) {
			mifml_syntax();
			printf( "Enter MIF file name (or ENTER to exit): " );
			gets( infile );
			if( strcmp( infile, "\0" ) == 0 ) {
				return 0;
			}
			inset = 1;
			printf( "Enter MIFML file name: " );
			gets( outfile );
			if( strcmp( infile, "\0" ) == 0 ) {
				outset = 0;
			}
		}
		for( i=1; i<argc; i++ ) {
			strcpy( tmparg, argv[i] );
			c = tmparg[0]; // check first char
			if( c == '/' ) {
				strcpy( tmp0, tmparg );
				strtoupper( tmp0 );
				// /S==scripted; /T=F==type full; /T=P==type partial;
				switch( tmp0[1] ) { // get second char ..
					case 'S':		// set scripted option
						scripted = 1;
						break;
					case 'F':		// set full mifml export
						opts = 0;
						scripted = 1;
						break;
					case 'P':		// set partial mifml export
						opts = 1;
						scripted = 1;
						break;
					case 'T':		// set export-type
						switch( tmp0[3] ) { // get 4th char ..  /T=F==type full; /T=P==type partial
							case 'F':
								opts = 0;
								break;
							case 'P':
								opts = 1;
								break;
							default :
								printf( "> ERROR: Invalid export type %s\n", tmparg );
								err++;
						}
						break;

					case 'X':		// set input filetype to MIFML
						outtype = 1;
						scripted = 1;
						break;
					default :
						printf( "> ERROR: Invalid switch %s\n", tmparg );
						err++;
				}
			}
			else if( inset == 0 ) {
				strcpy( infile, argv[i] );
				inset = 1;
			}
			else if( outset == 0 ) {
				strcpy( outfile, argv[i] );
				outset = 1;
			}
			else if( dtdset == 0 ) {
				strcpy( dtdpath, argv[i] );
				dtdset = 1;
			}
			else {
				printf( "> ERROR: Invalid command line option [%s]\n", argv[i] );
				err++;
			}

		}
		if( inset == 0 ) {
			printf( "> ERROR: No input file specified.\n" );
			err++;
		}
		if( err > 0 ) {
			mifml_syntax();
			if( scripted == 0 ) {
#ifdef WIN32
				system( "pause" );
#endif
			}
			return 0;
		}
		if( outset == 0 ) {
			// if outfile is not set, test infile for type and determine outfilename
			outtype = mifmlFileTest(infile);
			if (outtype == 0) {	// it's a MIF
				sprintf(outfile, "%s.%s\0", infile, MIFMLEXTENSION);
				printf( "> Input file appears to be a MIF .. using MIFML conversion.\n" );
			}
			else if (outtype == 1) { 	// it's a MIFML
				sprintf(outfile, "%s.%s\0", infile, MIFEXTENSION);
				printf( "> Input file appears to be XML .. using MIF conversion.\n" );
			}
			else if (outtype == -2) 	// error reading
				err = 1;
			else {
				printf( "\n> ERROR: Input file appears to be neither MIFML nor MIF.\n" );
				err = 1;
			}
			if (err) {
				mifml_syntax();
				if( scripted == 0 ) {
#ifdef WIN32
					system( "pause" );
#endif
				}
				return 0;
			}
		}
		if( dtdset == 0 ) {
			strcpy( dtdpath, ".\0" );
		}
		if( outtype == 0 ) {
			ret = mif2mifml ( infile, outfile, dtdpath, opts );
			strcpy( chunktype, "lines\0" );
		}
		else {
			ret = mifml2mif ( infile, outfile );
			strcpy( chunktype, "elements\0" );
		}
		if( ret > 0 ) {
			printf("> %d %s processed\n", ret, chunktype);
			printf("> Generated file %s\n", outfile );
			if( scripted == 0 ) {
#ifdef WIN32
				system( "pause" );
#endif
			}
		}
		else {
			if( ret == -1 ) {
				printf("> ERROR: Unable to read file [%s].\n", infile );
			}
			else if( ret == -2 ) {
				printf("> ERROR: Unable to write to file [%s].\n", outfile );
			}
			else if( ret == -99 ) {
				printf("> ERROR: Invalid input file.\n" );
			}
			else if( ret < -4 ) {
				if( ret == -10 ) {
					printf("> ERROR: MIF statement name exceeded assumed maximum.\n" );
				}
				else if( ret == -11 ) {
					printf("> ERROR: MIF nesting levels dropped below zero.\n" );
				}
				else if( ret == -12 ) {
					printf("> ERROR: MIF nesting levels exceeded assumed maximum.\n" );
				}
				else {
					printf("> ERROR: Code %d.\n", ret );
				}
				printf("> Please upload this MIF file to http://www.leximation.com/inbox/ for analysis.\n");
			}
			else {
				printf("> ERROR: Code %d.\n", ret );
				printf("> Please upload this MIF file to http://www.leximation.com/inbox/ for analysis.\n");
			}
			if( scripted == 0 ) {
#ifdef WIN32
				system( "pause" );
#endif
			}
		}
	}
	return ret;
}

void mifml_syntax( void ) {

	printf("\n===============================================================================\n");
	printf(" %s v.%s %s \n", MIF2MIFMLAPPNAME, MIF2MIFMLVER, MIF2MIFMLCREATOR);
	printf("===============================================================================\n\n");
	printf("\tMIFML [/(P|F|X)] <file_in> [<file_out> [<dtd_path>]]\n\n" );
	printf(" Where:\n\n" );
	printf("    /F         - Generates a \"full\" MIFML file (the default).\n" );
	printf("    /P         - Generates a \"partial\" MIFML file.\n" );
	printf("    /X         - Sets the input file type to MIFML (XML) and writes a MIF.\n" );
	printf("    <file_in>  - Name of file to process.\n" );
	printf("    <file_out> - Name of file to generate (default, <file_in>.MIFML).\n" );
	printf("    <dtd_path> - Path to the MIFML.DTD file (default is no path). \n" );
	if( EXPIREMONTH > 0 ) {
		printf("\n >> This Beta application will stop working in %s.\n", EXPIREMONTHSTR );
	}
	printf("\n Supports MIF files up to FM7.2. Later FM versions may work as well.\n" );
	printf("\n For more information, see %s\n", MIFMLURL );
	printf(" %s\n", MIF2MIFMLCOPYRIGHT );
	printf("===============================================================================\n");
}

