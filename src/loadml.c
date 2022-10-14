#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mytime.h"

int main(int ac, char **av)
{
	int nrows = 0;
	void loadcatalog( char *filename, char *db, int *nrows );

	loadcatalog( "/Users/ichinose1/Work/MonteCristo/maps/eqs/dbmag.data.txt", "/Users/ichinose1/Work/mtinv.v4.0.0/data/mt.db", &nrows );
}

void loadcatalog( char *filename, char *db, int *nrows )
{
	FILE *fp;
	char rec[512];

	int irow = 0;
	char timestr[48], magtype[8], agency[8], id[128];
	char cdum, dum[8];
	float lat, lon, dep, mag;
	long evid;

	MyTime ot;
	char *MyTime2StandardString( MyTime *t, char *str );
	void parsestring( MyTime *t, char *str );
	float dt = 5.0;

/*
	2020-05-08T14:55:08.468Z 38.0472   -118.7414  6.02 3.7  mwr us us70009cel
	2020-05-11T20:26:07.410Z 38.0055   -118.7035    -1 3.49  mw nc nc73383655
	2020-05-12T18:15:13.700Z 37.450333 -118.025  13.02 3.45  ml nc nc73384075
	2020-05-15T11:03:27.176Z 38.1689   -117.8497  2.7  6.5   ml nn nn00725272
	2020-05-15T11:09:09.140Z 38.1406   -117.9952 10.8  4.1   ml nn nn00725277
	2020-05-15T11:11:32.736Z 38.188    -117.7692   18  3.5   ml nn nn00742884
*/
	fp = fopen( filename, "r" );

	while( fgets( rec, 512, fp ) != NULL )
	{
		sscanf( rec, "%s %f %f %f %f %s %s %[^\n]",
			timestr, &lat, &lon, &dep, &mag, 
			magtype, agency, id );
	
		parsestring( &ot, timestr );
		sscanf( id, "%c%c%ld", &cdum, &cdum, &evid );

		if( strcmp( agency, "nn" ) == 0 || strcmp( agency, "nc" ) == 0 )
		{
			fprintf( stdout, "###\n" );
			fprintf( stdout, "### irow=%d evid=%ld input=(%s) trans=(%s) lat=%g lon=%g dep=%g magtype=%s mag=%g agency=%s id=%s\n", 
				irow, evid, timestr, MyTime2StandardString( &ot, timestr ), lat, lon, dep, magtype, mag, agency, id );
			fprintf( stdout, "###\n" );

			fprintf( stdout, "sqlite3 %s << EOF\n", db );
			fprintf( stdout, ".headers on\n" );

			fprintf( stdout, "select * from MT_ORIGIN_STAGE where evid = %ld;\n", evid );

			fprintf( stdout, "update MT_ORIGIN_STAGE set ml = %g, mlid = %ld ", mag, evid );

			fprintf( stdout, " where orid = ( select orid from MT_ORIGIN_STAGE where time between %lf - %f and %lf + %f );\n",
				ot.epoch, dt, ot.epoch, dt );

			
			fprintf( stdout, "select * from MT_ORIGIN_STAGE where time between %lf - %f and %lf - %f;\n",
                        	ot.epoch, dt, ot.epoch, dt );

			fprintf( stdout, ".quit\n" );
			fprintf( stdout, "EOF\n" );

			fprintf( stdout, "\n" );
			irow++;
		}
	}
	fclose(fp);

	*nrows = irow;
	return;
}

/*****
	### load usgs catalog to read time, lat, lon, ml, evid
	### create a sqlite3 csh script to update the table using update/select query
	###
	sqlite3 /Users/ichinose1/Work/mtinv.v4.0.0/data/mt.db << EOF
	.headers on
	update MT_ORIGIN_STAGE set ml = 5.2, mlid = 1755218 where orid = (select orid from MT_ORIGIN_STAGE where time between 1593509063.54-5 and 1593509063.54+5 ) ;
	select * from MT_ORIGIN_STAGE where orid = 11027;
	.quit
	EOF
*****/
