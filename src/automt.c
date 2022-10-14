/**************************************************************************************************************************************/
/***
Things to do. 
Mon Oct 31 14:34:48 PDT 2016

1. Write output to YEAR/MONTH/ directory structure (needed before web page support?)

2. Add support for EVID NUMBER(14) to setupMT.c, makepar.c, mtinv.c/dbsubs.c, mtbestfit.c and automt.c
     Also author support (username/login) or automt

3. web page review support, this is a job for mtinv: mtbestfit.c ?

4. program as background dameon, write pid file so no more than 1 running per directory

DONE . add current and new EventList support, live loop with sleep  = SLEEP_INTERVAL_TIME_SEC

DONE 2020-07-03 . add support for pre, post padding to start and stop times and also lowest_velocity

7. distaz save to station list object

***/
/**************************************************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "../include/mytime.h"
#include "../include/event.h"
#include "../include/AutoMT.h"
#include "../include/station.h"
#include "../include/sac.h"

char progname[256];
char working_directory[256];

int main( int ac, char **av )
{
	int i, j, k;
	EventList e;
	int verbose  = 0;
	int realtime = 1;
	int getwave  = 0;
	int getresp  = 0;
	int isacqc   = 0;
	Search_Parameters sp;

	StationList *sl;
	char station_xml_filename[128];

	char pathname[256];
        char workdir[512];
	char respdir[512];
	char datadir[512];
	char event_dirname[512];
	char curl_options[128];

	char Velocity_Model_Name[64];

/*** sleep time between WEB SERVICE URL queries, to avoid overloading and getting dropped ***/
/*** DNS avoids denial of service like behavior ***/
	int SLEEP_INTERVAL_TIME_SEC = 2; /*** unistd.h unsigned int sleep ( unsigned int seconds ) ***/

/*****************/
/*** functions ***/
/******************/

/*** write to log file ***/

	char logit[1024];
	void writelog( char *message, char *routine, int verbose );

	char *MyTime2StandardString( MyTime *t, char *str );
	char *MyTime2StandardString2( MyTime *t );

/*** set the start- and end-time search parameters ***/

	void Search_Parameters_setduration( Search_Parameters *sp, int realtime );

/*** get the events using web service request, return out in xml file format ***/

	void doEventQuery( Search_Parameters *sp, char *curl_opts, int rt, int verbose );

/*** read the events from XML format file ***/

	int readEventsXML( char *filename, EventList *e, int verbose );

/*** do a full write of the event list use for debugging ***/

	void writeEventList( EventList *e );
	void writeEvent2File( Event *e, char *filename );
	void writeEvents2Log( EventList *e );

/*** do query for network, station query for specified event and search parameters ***/

	void doStationQuery(
		Search_Parameters *sp,
		Event *e,
		char *curl_opts,
		int realtime,
		char *filename,
		int verbose );

/*** do query for network, station, channel query for specified event and search parameters ***/

	void doChannelQuery(
		Search_Parameters *sp,
		Event *e,
		char *curl_opts,
		int realtime,
		char *filename,
		int verbose );

/*** read station or channel level queries from XML format file into memory ***/
/**************************************************************************************************************/
/**** G.Ichinose Feb 28, 2019 ****/
/**** added time Mytime t of the origin to select the correct time epoch of the channel start/end dates ***/
/**************************************************************************************************************/
	/* int readStationXML( char *filename, StationList *sl, int verbose ); */
	int readStationXML( char *filename, StationList *sl, MyTime *t, int verbose );

/*** do a full write of the station list for debugging ***/

	void writeStation( StationList *sl );

/*** get waveforms in sac binary format little endian ***/

	void doWaveformQuery(
		Search_Parameters *sp,
		Event *e,
		StationList *sl, 
		char *curl_opts, 
		int getwave,
		int realtime,
		int SLEEP_INTERVAL_TIME_SEC,
		int verbose );

/*** get sac polezero response files ***/

	void doResponseQuery(
		Search_Parameters *sp,
		Event *e,
		StationList *sl,
		char *curl_opts,
		int getresp,
		int realtime,
		int SLEEP_INTERVAL_TIME_SEC,
		int verbose );

/*** create some files for mtinv setup ***/

	void create_station_file( StationList *sl );
	void create_setupMT( Event *e, char *vmod );

	void doSacQC( StationList *sl, Event *e, int verbose );

/*** posix compatible mkdir(), creates all directories existing or not ***/

	int mkdirp2( const char *, mode_t mode );

/*** misc get par ***/

	int setpar(int,char **),mstpar(),getpar();
	void endpar();
	char *shorten_path( char *, char * );

/************************************************************************************/
/*** begin program                                                                ***/
/************************************************************************************/

	strcpy( pathname, av[0] );
	shorten_path( pathname, progname );
	fprintf( stderr, "pathname=%s progname=%s\n", pathname, progname );

/*** set some search parameter defaults ***/

	strcpy( sp.webserve_agency_name, "USGS" );
	strcpy( sp.webserve_url, "https://earthquake.usgs.gov/fdsnws/event/1" );
	sp.duration_days = 4;
	sp.minmag = 4;
	sp.maxmag = 10;
	sp.limit_return = 2000;
	sp.minlat = -20;
	sp.maxlat = +90;
	sp.minlon = -20;
	sp.maxlon = 170;

	strcpy( sp.metadataWebservice_url, "https://service.iris.edu/fdsnws/station/1" );
	strcpy( sp.waveformWebservice_url, "https://service.iris.edu/irisws/timeseries/1" );
	strcpy( sp.responseWebservice_url, "https://service.iris.edu/irisws/sacpz/1" );
	strcpy( curl_options, " --max-time 60 --connect-timeout 60 -k " );

	sp.maxDistMTdeg = 12;
	sp.minimumWaveformDurationSec = 1800; /* 30 min */
	sp.minimumWaveformDurationSec = 1200; /* 20 min */
	sp.lowestPhaseVelocity = 1.5;        /*  km per sec */
	
/**************************************************************/
/*** getpar ***/
/**************************************************************/

	setpar(ac,av);

	mstpar( "working_directory", "s", working_directory );
 
	getpar( "curl_options", "s", curl_options );

	getpar( "maxDistMTdeg", "f", &(sp.maxDistMTdeg) );
	getpar( "lowestVelocity", "f", &(sp.lowestPhaseVelocity) );
	getpar( "minDurSec", "f", &(sp.minimumWaveformDurationSec) );
	/* strcpy( Velocity_Model_Name, "wus" ); */
	/* ~/Work/mtinv.v4.0.0/data/modeldb/br.mod */
	strcpy( Velocity_Model_Name, "br" );
	mstpar( "vmod", "s", Velocity_Model_Name );

	sp.preCutTime = 600;
	getpar( "precut", "f", &(sp.preCutTime) );  /*** time in sec before origin time for SNR processing = 0 for origin-time ***/

/*** metadata, waveform and response queries ***/
	getpar( "url_metadata", "s", sp.metadataWebservice_url );
	getpar( "url_waveform", "s", sp.waveformWebservice_url );
	getpar( "url_response", "s", sp.responseWebservice_url );

/*** this is for event query ***/
	getpar( "agency",   "s", sp.webserve_agency_name ); /** USGS,ISC,IRIS **/
	getpar( "url",      "s", sp.webserve_url );

	getpar( "realtime", "b", &realtime );
	getpar( "getwave", "b", &getwave );
	getpar( "getresp", "b", &getresp );
	getpar( "sacqc", "b", &isacqc );

	if(realtime)
	{
		getpar( "duration", "f", &(sp.duration_days) );
	}
	else
	{
		mstpar( "start",    "s", sp.start_string );
		mstpar( "end",      "s", sp.end_string );
	}
	getpar( "minmag",   "f", &(sp.minmag) );
	getpar( "maxmag",   "f", &(sp.maxmag) );

	getpar( "minlat",   "f", &(sp.minlat) );
	getpar( "minlon",   "f", &(sp.minlon) );
	getpar( "maxlat",   "f", &(sp.maxlat) );
	getpar( "maxlon",   "f", &(sp.maxlon) );

	getpar( "verbose",  "b", &verbose );

	getpar( "SLEEP_INTERVAL_TIME_SEC", "d", &SLEEP_INTERVAL_TIME_SEC );

	endpar();

	if( chdir( working_directory ) != 0 )
	{
		fprintf( stderr, "%s: no working directory %s\n",
			progname, working_directory );
		exit(-1);
	}

	snprintf( logit, sizeof(logit), "%s: ", progname );
        writelog( logit, "main()", verbose );

/****************************************************************************/
/*** set search time ***/
/****************************************************************************/

	Search_Parameters_setduration( &sp, realtime );

	snprintf( logit, sizeof(logit),
		"realtime=%d start and stop times duration = %g (days): start=%s stop=%s ",
                realtime,
                (sp.et.epoch - sp.st.epoch)/86400,
		MyTime2StandardString2( &(sp.st) ),
		MyTime2StandardString2( &(sp.et) ) );
	writelog( logit, "main()", verbose );

/****************************************************************************/
/*** do the event query ***/
/****************************************************************************/

	doEventQuery( &sp, curl_options, realtime, verbose );

/****************************************************************************/
/*** read the event query results, write to log file                      ***/
/*** return a global event list object                                    ***/
/****************************************************************************/

	if( readEventsXML( sp.xml_filename, &e, verbose ) )
	{
		if(verbose)
		{
		  snprintf( logit, sizeof(logit),
			"success readEventsXML() read %d", e.nevents );
		  writelog( logit, "main()", verbose );
		}
	}

	if(verbose) writeEventList( &e );

	writeEvents2Log( &e );


/****************************************************************************/
/*** events loop request SAC waveform data and SAC_PZs response           ***/
/*** create a station list global object for each event                   ***/
/****************************************************************************/

	sl     = calloc( e.nevents+1, sizeof(StationList) );	

	for( k = 0; k < e.nevents; k++ )
	{
		/****************************************************/
		/*** for( j = 0; j < e.ev[k].norigins; j++ )      ***/
		/*** these subroutines assume single origins      ***/
		/***  e.ev[k].norigins=1 and ev[k].o[0] per event ***/
		/****************************************************/
		snprintf( logit, sizeof(logit),
			"norigins = %d", e.ev[k].norigins );
		writelog( logit, "main()", verbose );

	/***************************************************/
	/*** make event parent and child directories     ***/
	/***************************************************/

		sprintf( event_dirname, "%s/%04d-%02d-%02dT%02d%02d%02d_%s",
			working_directory,
			e.ev[k].o[0].origintime->year,
			e.ev[k].o[0].origintime->month,
			e.ev[k].o[0].origintime->mday,
			e.ev[k].o[0].origintime->hour,
			e.ev[k].o[0].origintime->min,
			e.ev[k].o[0].origintime->isec,
			e.ev[k].o[0].cinfo.agency );

		snprintf( logit, sizeof(logit), "event_dirname=%s", event_dirname );
		writelog( logit, "main()", verbose );

		sprintf( datadir, "%s/Data", event_dirname );
		mkdirp2( datadir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
		
		sprintf( respdir, "%s/Resp", event_dirname );
		mkdirp2( respdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );

		sprintf( workdir, "%s/dev", event_dirname );
		mkdirp2( workdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );

		chdir( event_dirname );

		writeEvent2File( &(e.ev[k]), "event.txt" );
		
	/***************************************************/
	/*** doChannelQuery does station too in xml      ***/
	/***************************************************/
	/***
		doStationQuery( &sp, &(e.ev[k]), curl_options, realtime, station_xml_filename, verbose );
		readStationXML( station_xml_filename, &(sl_tmp[k]), (e.ev[k].o[0].origintime), verbose );
		writeStation( &(sl_tmp[k]) );
	***/
		doChannelQuery( &sp, &(e.ev[k]), curl_options, realtime, station_xml_filename, verbose );

		snprintf( logit, sizeof(logit), 
			"calling readStationXML() %s", station_xml_filename );
		writelog( logit, "main()", verbose );

	/******************************************************************/
	/*** read the sta-channel XML file into the station list object ***/
	/******************************************************************/

		if( readStationXML( station_xml_filename, &(sl[k]), (e.ev[k].o[0].origintime), verbose ) )
		{
			snprintf( logit, sizeof(logit), "success readStationXML()" );
			writelog( logit, "main()", verbose );
		}

		if(verbose) writeStation( &(sl[k]) );

	/***************************************************/
	/*** do sac data waveform requeset               ***/
	/***************************************************/

		snprintf( logit, sizeof(logit), "calling doWaveformQuery datadir=%s", datadir );
                writelog( logit, "main()", verbose );

		chdir( datadir );
		doWaveformQuery( &sp, &(e.ev[k]), &(sl[k]), curl_options, getwave, realtime, SLEEP_INTERVAL_TIME_SEC, verbose );


	/********************************************************/
	/*** create the rdseed.stations in the Data directory ***/
	/********************************************************/

		snprintf( logit, sizeof(logit), "calling create_station_file" );
		writelog( logit, "main()", verbose );

		create_station_file( &(sl[k]) );

	/***************************************************/
	/*** do quality control QC check of the data     ***/
	/***************************************************/

		if( isacqc )
		{
			snprintf( logit, sizeof(logit), "calling doSacQC" );
			writelog( logit, "main()", verbose );

			doSacQC( &(sl[k]), &(e.ev[k]), verbose );
		}

	/***************************************************/
	/*** do sac polezero response request            ***/
	/***************************************************/

		snprintf( logit, sizeof(logit), "calling doResponseQuery" );
                writelog( logit, "main()", verbose );

		chdir( respdir );
		doResponseQuery( &sp, &(e.ev[k]), &(sl[k]), curl_options, getresp, realtime, SLEEP_INTERVAL_TIME_SEC, verbose );

	/***************************************************/
	/*** create the setup script                     ***/
	/***************************************************/

		snprintf( logit, sizeof(logit), "running system command create_setupMT" );
                writelog( logit, "main()", verbose );

		chdir( workdir );
		create_setupMT( &(e.ev[k]), Velocity_Model_Name );

		snprintf( logit, sizeof(logit), "running system command makeglib.csh" );
                writelog( logit, "main()", verbose );

		system( "/bin/csh makeglib.csh" );

		snprintf( logit, sizeof(logit), "running system command run.csh" );
                writelog( logit, "main()", verbose );

		system( "/bin/csh run.csh" );

/*** put this inside run.csh ***/
/*
		snprintf( logit, sizeof(logit), "running system command mtbestfit" );
                writelog( logit, "main()", verbose );
                system( "mtbestfit" );
*/

		snprintf( logit, sizeof(logit), "running system command run2.csh" );
                writelog( logit, "main()", verbose );

	/************************************************************************/
	/*** return to main directory, not ncessary since all paths absolute  ***/
	/************************************************************************/
		chdir( working_directory );

		snprintf( logit, sizeof(logit),
			"Finished with eventid=%s desc=(%s) [%d of %d Events done]. ",
				e.ev[k].eventid,
				e.ev[k].description,
				k,
				e.nevents );

		writelog( logit, "main()", verbose );

	} /*** loop over k-events ***/

	free(sl);

	snprintf( logit, sizeof(logit),
		"Bye Bye, ALL DONE!" );
	writelog( logit, "main()", verbose );

} /*** end of main() ***/



/**************************************************************************************************************************************/
/*** Search_Parameters_setduration ***/
/**************************************************************************************************************************************/

void Search_Parameters_setduration( Search_Parameters *sp, int realtime )
{
	float secperday = 86400, sec_offset;

/*** functional prototypes ***/
        void initialize_mytime( MyTime *t );
        MyTime *myGMTtime( MyTime *t );
        MyTime *epoch2time( MyTime *t, double epoch );
	void parsestring( MyTime *t, char *str );
	char *MyTime2StandardString( MyTime *t, char *str );

	initialize_mytime( &(sp->st) );
	initialize_mytime( &(sp->et) );
	if(realtime)
	{
		myGMTtime( &(sp->et) );
		myGMTtime( &(sp->st) );
		sec_offset = sp->duration_days * secperday;
		epoch2time( &(sp->st), (sp->st.epoch - (double)sec_offset) );
	}
	else
	{
		parsestring( &(sp->st), sp->start_string );
		parsestring( &(sp->et), sp->end_string );
	}
	MyTime2StandardString( &(sp->st), sp->start_string );
        MyTime2StandardString( &(sp->et), sp->end_string );
}

void doEventQuery( Search_Parameters *sp, char *curl_options, int realtime, int verbose )
{
	char url_string[4096];
	char command_line[8196];
	char usgs_special[32];

/*** write to log file ***/
        char logit[1024];
        void writelog( char *message, char *routine, int verbose );

	sprintf( sp->xml_filename, "events.%s.xml", sp->webserve_agency_name );

	if( strcmp( sp->webserve_agency_name, "USGS" ) == 0 )
	{
		sprintf( usgs_special, "method=query&" );
	}
	else
	{
		sprintf( usgs_special, "" );
	}

	sprintf( url_string,
"%s/query?%sformat=xml&starttime=%s&endtime=%s&minlatitude=%g&maxlatitude=%g&minlongitude=%g&maxlongitude=%g&minmagnitude=%g&maxmagnitude=%g&limit=%d&orderby=time", 
		sp->webserve_url, 
		usgs_special,
		sp->start_string,
		sp->end_string,
		sp->minlat,
		sp->maxlat,
		sp->minlon,
		sp->maxlon,
		sp->minmag,
		sp->maxmag,
		sp->limit_return );
	
	sprintf( command_line,
		"/usr/bin/curl %s -o %s \"%s\"",
		curl_options,
		sp->xml_filename,
		url_string );

	snprintf( logit, sizeof(logit), "%s", command_line );
	writelog( logit, "doEventQuery()", verbose );

	system( command_line );	

} /*** end of  Search_Parameters_setduration() ***/




/**************************************************************************************************************************************/
/*** doStationQuery() ***/
/**************************************************************************************************************************************/

void doStationQuery(  Search_Parameters *sp, Event *e, char *curl_options, int realtime, char *filename, int verbose )
{
	char url_string[4096];
        char command_line[8196];

	/*** write to log file ***/
        char logit[1024];
        void writelog( char *message, char *routine, int verbose );

	sprintf( sp->xml_filename, "station.%s.xml", e->eventid );
	strcpy( filename, sp->xml_filename );

	sprintf( url_string, "%s/query?starttime=%s&endtime=%s&latitude=%g&longitude=%g&maxradius=%g&nodata=404&channel=BH?,HH?&level=station&format=xml&matchtimeseries=TRUE&includerestricted=FALSE&includeavailability=FALSE&includecomments=FALSE",
		sp->metadataWebservice_url,
		sp->start_string,
		sp->end_string, 
		e->o[0].latitude,
		e->o[0].longitude,
		sp->maxDistMTdeg );

	sprintf( command_line,
                "/usr/bin/curl %s -o %s \"%s\"",
		curl_options,
                sp->xml_filename,
                url_string );

	snprintf( logit, sizeof(logit), "%s", command_line );
        writelog( logit, "doStationQuery()", verbose );

        system( command_line );
}

void doChannelQuery( Search_Parameters *sp, Event *e, char *curl_options, int realtime, char *filename, int verbose )
{
        char url_string[4096];
        char command_line[8196];

	/*** write to log file ***/
        char logit[1024];
        void writelog( char *message, char *routine, int verbose );

        sprintf( sp->xml_filename, "channel.%s.xml", e->eventid );
	strcpy( filename, sp->xml_filename );

        sprintf( url_string, "%s/query?starttime=%s&endtime=%s&latitude=%g&longitude=%g&maxradius=%g&nodata=404&channel=BH?,HH?&level=channel&format=xml&matchtimeseries=TRUE&includerestricted=FALSE&includeavailability=FALSE&includecomments=FALSE",
                sp->metadataWebservice_url,
                sp->start_string,
                sp->end_string,
                e->o[0].latitude,
                e->o[0].longitude,
                sp->maxDistMTdeg );

        sprintf( command_line,
                "/usr/bin/curl %s -o %s \"%s\"",
		curl_options,
                sp->xml_filename,
                url_string );

	snprintf( logit, sizeof(logit), "%s", command_line );
        writelog( logit, "doChannelQuery()", verbose );

        system( command_line );

} /*** end of doStationQuery() ***/



/**************************************************************************************************************************************/
/*** doWaveformQuery() ***/
/**************************************************************************************************************************************/

void doWaveformQuery(	Search_Parameters *sp,
			Event *e,
			StationList *sl,
			char *curl_options,
			int getwave,
			int realtime,
			int SLEEP_INTERVAL_TIME_SEC,
			int verbose )
{
	int i, j, k;

	char url_string[4096];
	char command_line[8196];
	int status = 1;

/*** write to log file ***/
        char logit[1024];
        void writelog( char *message, char *routine, int verbose );

/*** sets some important SAC header values, also sets dist,az,baz,gcarc ***/
/*** assumes already set net.sta.loc.chan   ***/
/*** perhaps also set origin time?          ***/

	int FixSacHeader( char *sacfilename,
			float stla,
			float stlo,
			float stel,
			float stdp,
			float evla,
			float evlo,
			float evdp,
			float cmpaz,
			float cmpinc, 
			Search_Parameters *searchParameters,
			int verbose );

	char loc[8]; /*** webservice only accepts null location code as "--" ***/

/****** pre_event_time processing ***/

	MyTime *epoch2time( MyTime *t, double epoch );
	MyTime starttime;
	char starttime_string[32];

/*** begin subroutinue ***/

	for( i = 0; i < sl->numNets; i++ )
	{
	  for( j = 0; j < sl->n[i].nstation; j++ )
	  {
		if( sl->n[i].s[j].nchannel < 3 )
		{
			snprintf( logit, sizeof(logit), 
				"not 3-Component station=%s nchannel=%d\n",
					sl->n[i].s[j].code, sl->n[i].s[j].nchannel );
			writelog( logit, "doWaveformQuery()", verbose );

			continue;
		}

		for( k = 0; k < sl->n[i].s[j].nchannel; k++ )
		{
			/***   yr.jday.hr.min.sec.msec.net.sta.loc.chan.sac ***/

			sprintf( sl->n[i].s[j].c[k].sacfilename,
			  "%4d.%03d.%02d.%02d.%02d.%04d.%s.%s.%s.%s.SAC",
				e->o[0].origintime->year,
				e->o[0].origintime->jday,
				e->o[0].origintime->hour,
				e->o[0].origintime->min,
				e->o[0].origintime->isec,
				e->o[0].origintime->msec,
				sl->n[i].code,
				sl->n[i].s[j].code,
				sl->n[i].s[j].c[k].locationCode,
				sl->n[i].s[j].c[k].code );

			if( strcmp( sl->n[i].s[j].c[k].locationCode, "" ) == 0 ) 
			{
				strcpy( loc, "--" );
			}
			else	
			{
				strcpy( loc, sl->n[i].s[j].c[k].locationCode );
			}

	/*** put in a pre-event time before starttime? to do SNR calculations ****/
		/*** given pre_event_time in seconds calc offset from origin time ***/
		/*** convert time in seconds to a string format yyyy-mm-ddThh:mm:ss.ss ****/
			
			epoch2time( &starttime, ( e->o[0].origintime->epoch - sp->preCutTime ) );

			sprintf( starttime_string, "%04d-%02d-%02dT%02d:%02d:%02d",
				starttime.year,
				starttime.month,
				starttime.mday,
				starttime.hour,
				starttime.min,
				starttime.isec );

			
	/*** format options: output(deprecared)=sacbl SAC binary little-endian ***/
	/*** ***/
	/*** sprintf( url_string, "%s/query?network=%s&station=%s&location=%s&channel=%s&starttime=%s&duration=%g&output=sacbl", ***/
	/*** ***/
			sprintf( url_string,
	"%s/query?network=%s&station=%s&location=%s&channel=%s&starttime=%s&duration=%g&format=sac.zip",
				sp->waveformWebservice_url,
 				sl->n[i].code,
				sl->n[i].s[j].code,
				loc,
				sl->n[i].s[j].c[k].code,
				starttime_string, 	/* e->o[0].originTime_string */
				sp->minimumWaveformDurationSec );

		/*** add -s for slient ***/
		/*** change output file from sl->n[i].s[j].c[k].sacfilename to sac.zip file ***/

        		sprintf( command_line,
                		"/usr/bin/curl %s -o %s \"%s\"",
				curl_options, 
				"sac.zip",
                		url_string );

			snprintf( logit, sizeof(logit), "%s", command_line );
			writelog( logit, "doWaveformQuery()", verbose );

        		if(getwave)
			{
				system( command_line );
				sleep(SLEEP_INTERVAL_TIME_SEC);

				system( "/usr/bin/unzip sac.zip" );
	
			/*** format options: output(deprecared)=sacbl SAC binary little-endian / change to format=sac.zip ***/
			/*  AZ.BZN..BHE.M.2020.172.112427.SAC */
			/*  2020.172.11.24.27.0440.AZ.BZN..BHE.SAC */

			/* e->o[0].origintime->year, e->o[0].origintime->jday, e->o[0].origintime->hour, e->o[0].origintime->min, e->o[0].origintime->isec, */

				sprintf( command_line, "/bin/mv %s.%s.%s.%s.?.%4d.%03d.%02d%02d%02d.SAC %s",
						sl->n[i].code,
						sl->n[i].s[j].code,
						sl->n[i].s[j].c[k].locationCode,
						sl->n[i].s[j].c[k].code,
						starttime.year,
						starttime.jday,
						starttime.hour,
						starttime.min,
						starttime.isec,
					sl->n[i].s[j].c[k].sacfilename );

				snprintf( logit, sizeof(logit), "%s", command_line );
				writelog( logit, "doWaveformQuery()", verbose );

				system( command_line );

			/*** read the SAC file back in and populate some header values ***/

				status = FixSacHeader( sl->n[i].s[j].c[k].sacfilename,
                                        sl->n[i].s[j].latitude,
                                        sl->n[i].s[j].longitude,
                                        sl->n[i].s[j].elevation,
                                        sl->n[i].s[j].c[k].depth,
                                        e->o[0].latitude,
                                        e->o[0].longitude,
                                        e->o[0].depth,
                                        sl->n[i].s[j].c[k].azimuth,
                                        sl->n[i].s[j].c[k].dip,
					sp,
					verbose );

			/*** sac file had read ERROR, probably no data return from web server ***/

				if( status == 0 )
					sl->n[i].s[j].usedata = 0;
				else
					sl->n[i].s[j].usedata = 1;

				snprintf( logit, sizeof(logit), "sacfilename=%s status=%d (usedata=0(no)/1(yes))",
					sl->n[i].s[j].c[k].sacfilename,
					status );

				writelog( logit, "doWaveformQuery()", verbose );
			} 

		} /* k-channel */

	  } /* j-station */

	} /* i-network */

} /*** end of doWaveformQuery() ***/




/**************************************************************************************************************************************/
/*** doResponseQuery() ***/
/**************************************************************************************************************************************/

void doResponseQuery( 
			Search_Parameters *sp,
			Event *e,
			StationList *sl,
			char *curl_options, 
			int getresp,
			int realtime,
			int SLEEP_INTERVAL_TIME_SEC,
			int verbose )
{
	int i, j, k;
        char url_string[4096];
        char command_line[8196];
        char sacpzfilename[256];
	char loc[8];

	/*** write to log file ***/
        char logit[1024];
        void writelog( char *message, char *routine, int verbose );

        for( i = 0; i < sl->numNets; i++ )
        {
          for( j = 0; j < sl->n[i].nstation; j++ )
          {
		if( sl->n[i].s[j].usedata != 1 ) 
		{
			snprintf( logit, sizeof(logit), "usedata = %d", sl->n[i].s[j].usedata );
			writelog( logit, "doResponseQuery", verbose );
			continue;
		}
		if( sl->n[i].s[j].nchannel < 3 )
		{
			snprintf( logit, sizeof(logit),
			   "not 3-compent nchannel=%d", sl->n[i].s[j].nchannel );
			writelog( logit, "doResponseQuery", verbose );
			continue;
		}
	
                for( k = 0; k < sl->n[i].s[j].nchannel; k++ )
                {
			/* SAC_PZs_II_RAYN_BH1_00_2015.313.20.00.00.0000_2599.365.23.59.59.99999 */
                        sprintf( sacpzfilename, "SAC_PZs_%s_%s_%s_%s_%04d.%03d.%02d.%02d.%02d.%04d_",
				sl->n[i].code,
                                sl->n[i].s[j].code,
                                sl->n[i].s[j].c[k].code,
                                sl->n[i].s[j].c[k].locationCode,
                                e->o[0].origintime->year,
                                e->o[0].origintime->jday,
                                e->o[0].origintime->hour,
                                e->o[0].origintime->min,
                                e->o[0].origintime->isec,
                                e->o[0].origintime->msec );
	
			if( strcmp( sl->n[i].s[j].c[k].locationCode, "" ) == 0 )
                        {
                                strcpy( loc, "--" );
                        }
                        else
                        {
                                strcpy( loc, sl->n[i].s[j].c[k].locationCode );
                        }

                        sprintf( url_string,
        		"%s/query?network=%s&station=%s&location=%s&channel=%s&starttime=%s&endtime=%s",
                                sp->responseWebservice_url,
                                sl->n[i].code,
                                sl->n[i].s[j].code,
                                loc, 
                                sl->n[i].s[j].c[k].code,
                                e->o[0].originTime_string,
                                sp->end_string );

			sprintf( command_line,
                                "/usr/bin/curl %s -o %s \"%s\"",
				curl_options,
                                sacpzfilename,
                                url_string );

			snprintf( logit, sizeof(logit), "%s", command_line );
			writelog( logit, "doResponseQuery()", verbose );

                        if(getresp)
                        {
                                system( command_line );
				sleep(SLEEP_INTERVAL_TIME_SEC);
                        }

                } /* k-channel */

          } /* j-station */

        } /* i-network */

} /*** end of doResponseQuery() ***/





/**************************************************************************************************************************************/
/*** create_setupMT() ***/
/**************************************************************************************************************************************/

void create_setupMT( Event *e, char *Velocity_Model_Name )
{
	FILE *fp;
	int verbose = 0;

	char origintime_string[48];

/*** write to log file ***/
        char logit[1024];
        void writelog( char *message, char *routine, int verbose );

	fp = fopen( "setup.csh", "w" );
	fprintf( fp, "#!/bin/csh\n" );

	fprintf( fp, "###\n" );
	fprintf( fp, "### %s %s=%g EventID=%s evid=%ld Agency=(%s) lddate=(%s)\n",
		e->description, e->o[0].mag[0].magtype, e->o[0].mag[0].magnitude,
		e->eventid, e->evid, 
		e->cinfo.agency, e->cinfo.creationTime_string );

	fprintf( fp, "### publicID=(%s)\n", e->publicID );
	fprintf( fp, "### preferredOriginID=(%s)\n", e->preferredOriginID );
	fprintf( fp, "###\n" );

/*** assumes mtinv tools are in the users executable path ***/

        int max_number_stations_invert = 8;
        int max_number_stations_compute_grns = 30;
        float max_distance_process_km = 900;
        float max_distance_define_km  = 800;

	fprintf( fp,
 "setupMT -exclude_sta ../../exclude_sta.txt -ev %g %g -depth %g -velocity_model %s -comment \"(%s) (%s)=%g eventid=%s\" -origintime \"%4d-%02d-%02dT%02d:%02d:%02d\" -gmt5 -evid %s -realtime -Nsta %d -Ngrn %d -MaxDistInv %g -MaxDistProc %g ../Data/*.?HZ.SAC\n",
		e->o[0].latitude,
		e->o[0].longitude,
		e->o[0].depth,
		Velocity_Model_Name,
		e->description,
		e->o[0].mag[0].magtype,
		e->o[0].mag[0].magnitude,
		e->eventid,
		e->o[0].origintime->year,
		e->o[0].origintime->month,
		e->o[0].origintime->mday,
		e->o[0].origintime->hour,
		e->o[0].origintime->min,
		(int)rintf(e->o[0].origintime->fsec),
		e->eventid,
		max_number_stations_invert,
		max_number_stations_compute_grns,
		max_distance_define_km,
		max_distance_process_km
	    );
	fprintf( fp, "\n" );
	fclose(fp);

	chmod( "setup.csh", S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH );

	system( "/bin/csh setup.csh" );

	snprintf( logit, sizeof(logit), "finished executing setup.csh script" );
	writelog( logit, "create_setupMT()", verbose );

} /*** end of create_setupMT()  ***/




/**************************************************************************************************************************************/
/*** FixSacHeader ***/
/**************************************************************************************************************************************/

int FixSacHeader( char *sacfilename,
                        float stla,
                        float stlo,
                        float stel,
                        float stdp,
                        float evla,
                        float evlo,
                        float evdp,
                        float cmpaz,
                        float cmpinc,
			Search_Parameters *searchParameters,
			int verbose )
{
	FILE *fp;
	Sac_Header sp;
	float *data;
	char rec[1024], strbuf[24];
	int nitems;
	float TotalTimeDuration;

	float *readsac( Sac_Header *s, char *filename, int verbose );

/*** write to log file ***/
        char logit[1024];
        void writelog( char *message, char *routine, int verbose );

/*** read the sac file ***/
/*** return 0 if readsac returns NULL, sac file probably has "Error: 404: Not Found" WebServer response ***/

	if( (data = (float *)readsac( &sp, sacfilename, verbose )) == NULL )
	{
		return 0;
	}
	else
	{
		snprintf( logit, sizeof(logit), "open file %s", sacfilename );
		writelog( logit, "FixSacHeader()", verbose );
	}

/*** add these header values ***/

	if( sp.stla   == -12345 ) sp.stla   = stla;
	if( sp.stlo   == -12345 ) sp.stlo   = stlo;
	if( sp.stel   == -12345 ) sp.stel   = stel;
	if( sp.stdp   == -12345 ) sp.stdp   = stdp;
	if( sp.evla   == -12345 ) sp.evla   = evla;
	if( sp.evlo   == -12345 ) sp.evlo   = evlo;
	if( sp.evdp   == -12345 ) sp.evdp   = evdp;

/*** dip = -90 vertical -> cmpinc = 0 ***/
/*** dip = 0 horizontal -> cmpinc = 90 ***/
/*** first change inc = dip + 90, dont change azimuth requires no conversion ***/

	if( sp.cmpaz  == -12345 ) sp.cmpaz  = cmpaz;
        if( sp.cmpinc == -12345 ) sp.cmpinc = cmpinc + 90; /*** inc = dip + 90 ***/

/*** do some az and dip error checking ***/

/*** case here is both az=0 and dip=0 NULL return from XML file ***/

	if( ( sp.kcmpnm[2] == 'Z' ) && ( cmpaz == 0 ) && ( cmpinc == 0 ) )
	{
		sp.cmpaz = 0;
		sp.cmpinc= 0;
	}

	if( ( (sp.kcmpnm[2] == 'N') || (sp.kcmpnm[2] == '1') ) && ( ( cmpaz == 0 ) && ( cmpinc == 0 ) ) )
	{
		sp.cmpaz = 0;
		sp.cmpinc = 90;
	}
	
	if( ( (sp.kcmpnm[2] == 'E') || (sp.kcmpnm[2] == '2') ) && ( ( cmpaz == 0 ) && ( cmpinc == 0 ) ) )
	{
		sp.cmpaz = 90;
		sp.cmpinc = 90;
	}

/*** cases where horizontal component but dip is not 0(horizontal) some cases -90? ***/

	if( ( (sp.kcmpnm[2] == 'N') || (sp.kcmpnm[2] == '1') ) && ( cmpinc != 0 ) )
		sp.cmpinc = 90;
	if( ( (sp.kcmpnm[2] == 'E') || (sp.kcmpnm[2] == '2') ) && ( cmpinc != 0 ) )
		sp.cmpinc = 90;

/*** write SAC file back out ***/

	fp = fopen( sacfilename, "wb" );
	fwrite( &sp, sizeof(Sac_Header), 1, fp );
	fwrite( data, sp.npts * sizeof(float), 1, fp );
	fclose(fp);
	free(data);

/*** check for full waveform query return not just small segment, this stops/hangs/crashes sacdata2inv.c ***/

	TotalTimeDuration = searchParameters->minimumWaveformDurationSec - 1;

	/*
	snprintf( logit, sizeof(logit), "%s:  Time Duration (sp.e-sp.b)=%g < TotalTimeDuration = %g  .OR.   npts=%d < ( TotalTimeDuration / sp.delta=%g ) = %g",
                        sacfilename, 
			(sp.e-sp.b), 
			TotalTimeDuration,
			sp.npts, 
			sp.delta, 
			( TotalTimeDuration / sp.delta ) );
	writelog( logit, "FixSacHeader()", verbose );
	*/

	if( fabs( sp.e - sp.b ) < TotalTimeDuration || ( sp.npts < ( TotalTimeDuration / sp.delta ) ) ) 
	{
		snprintf( logit, sizeof(logit), "%s:  Time Duration (sp.e-sp.b)=%g < TotalTimeDuration = %g  .OR.   npts=%d < ( TotalTimeDuration / sp.delta=%g ) = %g",
                        sacfilename,
                        (sp.e-sp.b),
                        TotalTimeDuration,
                        sp.npts,
                        sp.delta,
                        ( TotalTimeDuration / sp.delta ) );
		writelog( logit, "FixSacHeader()", verbose );

		snprintf( logit, sizeof(logit), "%s: BAD SEGMENT return 0", sacfilename );
		writelog( logit, "FixSacHeader()", verbose );

		return 0;
	}
	else
	{
		return 1;
	}

} /*** end of FixSacHeader() ***/




/**************************************************************************************************************************************/
/*** doSacQC() ***/
/**************************************************************************************************************************************/

void doSacQC( StationList *sl, Event *e, int verbose )
{
	int i, j, k;
	FILE *fp;

/*** write to log file ***/

        char logit[1024];
        void writelog( char *message, char *routine, int verbose );

	fp = fopen("sacqc.csh","w" );

	fprintf( fp, "#!/bin/csh\n" );
	fprintf( fp, "#### \n" );
	fprintf( fp, "#### This is an automatic C-shell script written by %s\n", progname );
	fprintf( fp, "#### sacqc - looks at the distribution of raw amplitudes and writes QC results to SAC file headers\n" );
	fprintf( fp, "#### and also writes to database.  Sac file header values kuser=SACQC and kuser=999 when flagged as \n" );
	fprintf( fp, "#### fail QC- see sacqc.db.txt sqlite3 ascii dump to check reasons or look at JPEG plots\n" );
	fprintf( fp, "#### \n" );
	fprintf( fp, "\n" );

	fprintf( fp, "#set QC_DATABASE=/Users/ichinose1/Work/automt_test/sacqc.db\n" );
	fprintf( fp, "set QC_DATABASE=%s/sacqc.db\n", working_directory );
	fprintf( fp, "\n" );

	fprintf( fp, "### Do not write to this file. It is used as temporary file to store histograms for GMT plotting\n" );
	fprintf( fp, "###\n" );
	fprintf( fp, "/bin/rm -f sacqc.out\n" );
	fprintf( fp, "\n" );

	fprintf( fp, "cat >! sacqc.par << EOF\n" );
	fprintf( fp, "xstart=0\n" );
	fprintf( fp, "xstop=10\n" );
	fprintf( fp, "binwidth=0.1\n" );
	fprintf( fp, "nodiff\n" );
	fprintf( fp, "xlog\n" );
	fprintf( fp, "ylog\n" );
	fprintf( fp, "percent\n" ); 
	fprintf( fp, "maxstat=5\n" );
	fprintf( fp, "max_x=9\n" );
	fprintf( fp, "max_xper=2\n" );
	fprintf( fp, "noverbose\n" );
	fprintf( fp, "EOF\n" );
	fprintf( fp, "\n" );

/*** begin subroutinue ***/

        for( i = 0; i < sl->numNets; i++ )
        {
		for( j = 0; j < sl->n[i].nstation; j++ )
		{
			if( sl->n[i].s[j].nchannel >= 3 || sl->n[i].s[j].usedata == 1 )
			{
				for( k = 0; k < sl->n[i].s[j].nchannel; k++ )
				{
					fprintf( fp, "#####\n" );
					fprintf( fp, "sacqc par=sacqc.par db evid=%s gmt5 auth=%s writeback %s\n", 
						e->eventid,
						progname,
						sl->n[i].s[j].c[k].sacfilename );
					fprintf( fp, "\n" );

					fprintf( fp, "sqlite3 ${QC_DATABASE} << EOF\n" );
					fprintf( fp, ".read sacqc_insert.sql\n" );
					fprintf( fp, ".quit\n" );
					fprintf( fp, "EOF\n" );
					fprintf( fp, "\n" );
				}
			}
		}
	}

	fprintf( fp, "##### Dump a text file for this eventid\n" );
	fprintf( fp, "#####\n" );
	fprintf( fp, "sqlite3 ${QC_DATABASE} << EOF\n" );
	fprintf( fp, ".output sacqc.db.txt\n" );
	fprintf( fp, ".headers on\n" );
	fprintf( fp, ".mode col\n" );
	fprintf( fp, ".stats off\n" );
	fprintf( fp, ".timer off\n" );
	fprintf( fp, ".print \"\\nMT_DATA_QUALITY EVID=%s\\n\"\n", e->eventid );
	
	fprintf( fp, "SELECT qcid, net, sta, loc, chan, datetime( time, 'unixepoch' ) as startTime, npts, delta, \n" );
	fprintf( fp, "       qc_mode, qc_mean, qc_median, xmax, xmax_percent, \n" );
	fprintf( fp, "       defmask, mask_reason, auth, lddate \n" );
	fprintf( fp, "FROM\n" );
	fprintf( fp, "       MT_DATA_QUALITY where evid = %s;\n", e->eventid );

	fprintf( fp, ".quit\n" );
	fprintf( fp, "EOF\n" );
	fprintf( fp, "\n" );

	fprintf( fp, "##### Cleanup\n" );
	fprintf( fp, "#####\n" );
	fprintf( fp, "/bin/rm sacqc_insert.sql create.sql sacqc.out sacqc.par sacqc_gmt.csh gmt.history\n" );
	fprintf( fp, "\n" );
	fclose(fp);

	chmod( "sacqc.csh", S_IRWXU|S_IRWXG|S_IRWXO );
        system( "/bin/csh sacqc.csh" );

} /*** end of doSacQC() ***/

