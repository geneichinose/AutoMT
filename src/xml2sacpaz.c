#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "../include/mytime.h"
#include "../include/station.h"

char progname[256];
char working_directory[256];

int main( int ac, char **av )
{
	int verbose  = 0;

/*** misc get par ***/

        int setpar(int,char **),mstpar(),getpar();
        void endpar();

/*** write to log file ***/

	char logit[1024];
	void writelog( char *message, char *routine, int verbose );

/*** read station or channel level queries from XML format file into memory ***/
/*** added time Mytime t of the origin to select the correct time epoch of the channel start/end dates ***/

	StationList *sl;
	char station_xml_filename[128];
	int readStationXML( char *filename, StationList *sl, MyTime *t, int verbose );

/*** do a full write of the station list for debugging ***/
	void writeStation( StationList *sl );

	void write_sacpaz( StationList *sl );

/*** functional prototypes mytime.h : timesubs.c ***/

	MyTime origin_time;
	char time_string[24];
	void parsestring( MyTime *t, char *str );
	char *MyTime2StandardString( MyTime *t, char *str );
	char *MyTime2StandardString2( MyTime *t );
	MyTime *myGMTtime( MyTime *t );

/*****************************/
/*** begin main ***/
/*****************************/
	strcpy( progname, av[0] );
	strcpy( working_directory, "." );
	/* myGMTtime( &origin_time ); */
	strcpy( time_string, MyTime2StandardString2( myGMTtime( &origin_time ) ) );

	setpar(ac,av);
	getpar( "verbose", "b", &verbose );
	mstpar( "xml", "s", station_xml_filename );
	getpar( "time", "s", time_string );
	endpar();

	parsestring( &origin_time, time_string );

	if(verbose)
	{
		snprintf( logit, sizeof(logit), "station_xml_filename = %s time = %s (%s)",
			station_xml_filename,
			time_string,
			MyTime2StandardString( &origin_time, time_string ) );
		writelog( logit, "main()", verbose );	
	}

	sl = calloc( 1, sizeof(StationList) );

	if( readStationXML( station_xml_filename, sl, &origin_time, verbose ) )
	{
		snprintf( logit, sizeof(logit), "success readStationXML()" );
		writelog( logit, "main()", verbose );

		writeStation( sl );
		write_sacpaz( sl );
	}

	free( sl );
}
