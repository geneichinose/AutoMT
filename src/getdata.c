/****
Sun Feb 21 00:09:51 PST 2021
Version 1.0
	New Features:
	1. automatic GMT v5.x.x map of stations (use sac2gmtmap)
	2. Bugfix gaps results in 100s of little files (remove unused *.SAC) or use sacmerge
	3. creates devMT/setupMT.csh
	4. If the unzipped SAC filename (net.sta.loc.chan.M.year.jday.hhmmss.SAC has a start 
		time later in the filename than predicted from XML-channel then it cannot be 
		identified by /bin/mv
		Solution: Use "unzip -Z1" to get exact file names so /bin/mv can detect files
	5. Channels now required for all search types
Bugs:
	none

In Progress:
	1. Build a seperate waveform viewing app... 
		automatic GMT v5.x.x waveform plots  (use WaveformProcessor-wp)
		2A. 3-C low frequency narrow band filters with SNR 0.01 0.02, 0.02-0.05, 0.025-0.055, 0.033-0.055, 0.02-0.1, 0.033-0.01, 0.05-0.1
		2B. 1-10Hz band filtered
		2C automatic instrument response correction useing libresponse
		2D gmt plots

Future Updates:
	1. 24-hour Helicorder Plots
	2. automatic STA/LTA and kurtosis phase picker 
	3. 3-C analysis poloarization phase picker
	4. Spectrogram
	5. Beamform if array
	6. Magnitude Calculation: given location (calculate ML and mb and Ms)
	7. Record Section Plot: given location 
	8. backprojection analysis
	9. correlation, templates, subspace det
***/

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "../include/sac.h"
#include "../include/mytime.h"
#include "../include/station.h"
#include "../include/getdata.h"

char progname[256];
char working_directory[256];

int main( int ac, char **av )
{
	int i, j, k;

	int verbose  = 0;
	int getwave  = 0;
	int getresp  = 0;

	int map_gmt_stations = 0;

	Search_Window_Parameters sp;
	StationList *sl;
	
	char station_xml_filename[128];
	char pathname[256];
	char respdir[512];
	char datadir[512];

/*** sleep time between WEB SERVICE URL queries, to avoid overloading and getting dropped ***/
/*** DNS avoids denial of service like behavior ***/
/*** unistd.h unsigned int sleep ( unsigned int seconds ) ***/
	int SLEEP_INTERVAL_TIME_SEC = 2;

/*****************/
/*** functions ***/
/*****************/

/*** write to log file ***/

	char logit[1024];
	void writelog( char *message, char *routine, int verbose );

/**************************************************************************************************************/
/*** do query for network, station, channel query for specified event and search parameters ***/
/**************************************************************************************************************/
	void doChannelQuery_noEvent(
                Search_Window_Parameters *sp,
                char *filename,
                int verbose );

/**************************************************************************************************************/
/*** read station or channel level queries from XML format file into memory ***/
/**** added time Mytime t of the origin to select the correct time epoch of the channel start/end dates ***/
/**************************************************************************************************************/

	int readStationXML( char *filename, StationList *sl, MyTime *t, int verbose );

/*** do a full write of the station list for debugging ***/

        void writeStation( StationList *sl );

/*** get waveforms in sac binary format little endian ***/

        void doWaveformQuery_noEvent(
                Search_Window_Parameters *sp,
                StationList *sl,
                int getwave,
		int map_gmt_stations, 
		int SLEEP_INTERVAL_TIME_SEC,
                int verbose );

/*** get sac polezero response files ***/

        void doResponseQuery_noEvent(
                Search_Window_Parameters *sp,
                StationList *sl,
                int getresp,
		int SLEEP_INTERVAL_TIME_SEC,
                int verbose );

/*** create some files for mtinv setup ***/

	void create_station_file( StationList *sl );

/*** posix compatible mkdir(), creates all directories existing or not ***/

	int mkdirp2( const char *, mode_t mode );

/*** misc get par ***/

	int setpar(int,char **),mstpar(),getpar();
	void endpar();
	char *shorten_path( char *, char * );

	void setupMT_create( Search_Window_Parameters *sp );

/*****************************/
/*** functional prototypes ***/
/*** mytime.h : timesubs.c ***/
/*****************************/
	void initialize_mytime( MyTime *t );
	MyTime *epoch2time( MyTime *t, double epoch );
	void parsestring( MyTime *t, char *str );
	char *MyTime2StandardString( MyTime *t, char *str );
	void WriteMyTime2STDOUT( MyTime *a );
	void WriteMyTime2STDERR( MyTime *a );

/************************************************************************************/
/*** begin program                                                                ***/
/************************************************************************************/

	strcpy( pathname, av[0] );
	shorten_path( pathname, progname );

/*** set some search parameter defaults ***/

	strcpy( sp.metadataWebservice_url, "https://service.iris.edu/fdsnws/station/1" );
	strcpy( sp.waveformWebservice_url, "https://service.iris.edu/irisws/timeseries/1" );
	strcpy( sp.responseWebservice_url, "https://service.iris.edu/irisws/sacpz/1" );
	strcpy( sp.curl_options, "/usr/bin/curl --max-time 60 --connect-timeout 60 -k " );

/*** MacOSX-Mojave ***/
	strcpy(  sp.move_executable, "/bin/mv" );
	strcpy( sp.unzip_executable, "/usr/bin/unzip" );
	strcpy( sp.sac2gmtmap_executable, "/Users/ichinose1/bin/sac2gmtmap" );
	strcpy( sp.setupMT_executable, "/Users/ichinose1/Work/mtinv.v4.0.0/bin/setupMT" );
	strcpy( sp.sacmerge_executable, "/Users/ichinose1/Work/mtinv.v4.0.0/bin/sacmerge" );

/**************************************************************/
/*** getpar ***/
/**************************************************************/

	setpar(ac,av);

	mstpar( "working_directory", "s", working_directory ); /** an existing directory for putting data ***/

	snprintf( logit, sizeof(logit),
		"pathname=%s progname=%s working_directory=%s",
		pathname, progname, working_directory );
	writelog( logit, "main()", verbose );

	getpar( "curl_options", "s", sp.curl_options );
	getpar( "url_metadata", "s", sp.metadataWebservice_url );
	getpar( "url_waveform", "s", sp.waveformWebservice_url );
	getpar( "url_response", "s", sp.responseWebservice_url );
	getpar( "move_executable"   "s", sp.move_executable );
	getpar( "unzip_executable", "s", sp.unzip_executable );
	getpar( "sac2gmtmap_executable", "s", sp.sac2gmtmap_executable );
	getpar( "sacmerge_executable", "s", sp.sacmerge_executable );

	getpar( "getwave", "b", &getwave );
	getpar( "getresp", "b", &getresp );
	getpar( "verbose", "b", &verbose );
	getpar( "map_gmt_stations", "b", &map_gmt_stations );

	getpar( "SLEEP_INTERVAL_TIME_SEC", "d", &SLEEP_INTERVAL_TIME_SEC );

/*******************************************/
/*** define the time window              ***/
/*** start_time 2021-01-01T12:33:06.999  ***/
/*** start-time is the reference time    ***/
/*******************************************/
	initialize_mytime( &(sp.st) );
	mstpar( "start_time", "s", sp.start_string );
	parsestring( &(sp.st), sp.start_string );
	/* WriteMyTime2STDERR( &(sp.st) ); */

	snprintf( logit, sizeof(logit), "start_string = %s", sp.start_string );
	writelog( logit, "main()", verbose );

/*******************************************/
/*** Three ways to get data time-windows ***/
/*******************************************/

/*********************************************************************/
/*** 1. use a duration in seconds (from start_time + duration_sec) ***/
/*********************************************************************/

	sp.WaveFormDurationSec = -999;
	sp.preCutTimeSec  = 0;
	sp.postCutTimeSec = 0;

	getpar( "duration_sec",  "f",  &(sp.WaveFormDurationSec) );
	if( sp.WaveFormDurationSec > 0 )
	{
		epoch2time( &(sp.et), (sp.st.epoch + sp.WaveFormDurationSec) );
		/* WriteMyTime2STDERR( &(sp.et) ); */
	}
	snprintf( logit, sizeof(logit), "dur=%f\n", sp.WaveFormDurationSec );
	writelog( logit, "main()", verbose );

/*************************************************************/
/*** 2. use start_time and pre_time_sec post_time_sec      ***/
/***     t_start=(pre_time_sec - start_time)               ***/
/***     t_stop=(start_time + post_time_sec)               ***/
/***     NOTE!: This resets start_time                     ***/
/*************************************************************/

	if( sp.WaveFormDurationSec == -999 )
	{
		getpar( "pre_time_sec",  "f",  &(sp.preCutTimeSec) );
		getpar( "post_time_sec", "f", &(sp.postCutTimeSec) );
		if( ( sp.preCutTimeSec != 0 ) && ( sp.postCutTimeSec != 0 ) )
		{
			sp.WaveFormDurationSec = sp.preCutTimeSec + sp.postCutTimeSec;
			if( sp.postCutTimeSec > 0 )
				epoch2time( &(sp.et), (sp.st.epoch + sp.postCutTimeSec ) );
			if( sp.preCutTimeSec > 0 )
				epoch2time( &(sp.st), (sp.st.epoch - sp.preCutTimeSec) );
		}	
	}

/*************************************************************/
/*** 3. start_time + end_time                              ***/
/*************************************************************/

	if( ( sp.WaveFormDurationSec == -999 ) && ( sp.preCutTimeSec == 0 ) && ( sp.postCutTimeSec == 0 ) )
	{
		initialize_mytime( &(sp.et) );
		mstpar( "end_time",   "s", sp.end_string );
		parsestring( &(sp.et), sp.end_string );
		/* WriteMyTime2STDERR( &(sp.et) ); */

		sp.WaveFormDurationSec = sp.et.epoch - sp.st.epoch;
	}

/**** check for extreme conditions ***/

	if( sp.WaveFormDurationSec > 48*3600 || sp.WaveFormDurationSec <= 0 )
	{
		snprintf( logit, sizeof(logit), "FATAL ERROR: sp.WaveFormDurationSec=%g is <= zero or more than 48 hours, reset in parfile or in code",
			sp.WaveFormDurationSec  );
		writelog( logit, "main()", verbose );
		exit(-1);
	}

	MyTime2StandardString( &(sp.st), sp.start_string );
	MyTime2StandardString( &(sp.et), sp.end_string );

	snprintf( logit, sizeof(logit),
		"start=%s epoch=%lf end=%s epoch=%lf dur=%g pre=%g post=%g\n",
		sp.start_string,
		sp.st.epoch,
		sp.end_string,
		sp.et.epoch,
		sp.WaveFormDurationSec,
		sp.preCutTimeSec,
		sp.postCutTimeSec );
	writelog( logit, "main()", verbose );

/*** searching single-station or all-net.sta.chan in an area ****/
/*** define spatial search window ****/
/*** search radius circle lat/lon + radius ****/
/*** search box  lon1,lon0,lat1,lat0 ***/

	strcpy( sp.search_type, "single_station" );
	getpar( "search_type", "s", sp.search_type );

	sp.single_station = 0;
	sp.box_area       = 0;
	sp.circle_area    = 0;
	sp.box_minlat = -90;
	sp.box_maxlat = +90;
	sp.box_minlon = -180;
	sp.box_maxlon = +180;
	sp.clat = 0;
	sp.clon = 0;
	sp.radiuskm = 10000;

	if( strcmp( sp.search_type, "box" )    == 0 )
	{
		sp.box_area = 1;
	}
	else if( strcmp( sp.search_type, "circle" ) == 0 )
	{
		sp.circle_area = 1;
	}
	else if( strcmp( sp.search_type, "single_station" ) == 0 )
	{
		sp.single_station = 1;
	}
	else
	{
		sp.single_station = 1;
	}

/** **/
/*** Defaults: chan="BH?,HH?,EH?" ***/
/** **/
	strcpy( sp.chan, "BH?,HH?,EH?" );
	strcpy( sp.sta, " " );
	strcpy( sp.net, " " );
	strcpy( sp.loc, " " );

	if( sp.single_station )
	{
		mstpar( "sta", "s", &(sp.sta) );
		mstpar( "net", "s", &(sp.net) );
		mstpar( "loc", "s", &(sp.loc) );
		getpar( "chan", "s", &(sp.chan) );
	}
	else if( sp.box_area )
	{
		mstpar( "minlat", "f", &(sp.box_minlat) );
		mstpar( "maxlat", "f", &(sp.box_maxlat) );
		mstpar( "minlon", "f", &(sp.box_minlon) );
		mstpar( "maxlon", "f", &(sp.box_maxlon) );
		getpar( "chan", "s", &(sp.chan) );
	}
	else if( sp.circle_area )
	{
		mstpar( "clat", "f", &(sp.clat) );
		mstpar( "clon", "f", &(sp.clon) );
		getpar( "radiuskm", "f",  &(sp.radiuskm) );
		sp.radiusdeg = sp.radiuskm / 111.13;
		/* getpar( "radiusdeg", "f", &(sp.radiusdeg) ); */
		getpar( "chan", "s", &(sp.chan) );
	}

/*** run setupMT.c ***/
	sp.isetupMT = 0;
	getpar( "setupMT", "b", &(sp.isetupMT) );
	if( sp.isetupMT )
	{
		mstpar( "evla", "f", &(sp.evla) );
		mstpar( "evlo", "f", &(sp.evlo) );
		mstpar( "evdp", "f", &(sp.evdp) );
		mstpar( "ot",   "s", sp.origintime_string );
		mstpar( "vmod", "s", sp.velocity_model );
		getpar( "setupMT_executable", "s", sp.setupMT_executable );
	}

	endpar();

	snprintf( logit, sizeof(logit),
	"search_type=%s single_station=%d box_area=%d circle_area=%d NSLC=%s.%s.%s.%s box(W/E/N/S)=%g/%g/%g/%g cir(lat/lon/rad)=%g/%g/%g\n",
		sp.search_type,
		sp.single_station,
		sp.box_area, 
		sp.circle_area,
		sp.net, sp.sta, sp.loc, sp.chan,
		sp.box_minlon, sp.box_maxlon, sp.box_minlat, sp.box_maxlat,
		sp.clat, sp.clon, sp.radiuskm );
	writelog( logit, "main()", verbose );

/****************************************************************************/
/*** create the working directories for saving data and response files    ***/
/****************************************************************************/

	if( chdir( working_directory ) != 0 )
	{
		fprintf( stderr, "%s: no working directory %s\n", 
			progname, working_directory );
		exit(-1);
	}

/***************************************************/
/*** doChannelQuery does station too in xml      ***/
/***************************************************/

	doChannelQuery_noEvent( &sp, station_xml_filename, verbose );

	snprintf( logit, sizeof(logit), "calling readStationXML() %s", station_xml_filename );
	writelog( logit, "main()", verbose );

/******************************************************************/
/*** read the sta-channel XML file into the station list object ***/
/******************************************************************/

	sl = calloc( 1, sizeof(StationList) );
	if( readStationXML( station_xml_filename, sl, &(sp.st), verbose ) )
	{
		snprintf( logit, sizeof(logit), "success readStationXML()" );
		writelog( logit, "main()", verbose );
        	writeStation( sl );
	}

/***************************************************/
/*** do sac data waveform requeset               ***/
/***************************************************/

	sprintf( datadir, "%s/Data", working_directory );
	mkdirp2( datadir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
	chdir( datadir );

	snprintf( logit, sizeof(logit), "calling doWaveformQuery datadir=%s", datadir );
	writelog( logit, "main()", verbose );

	doWaveformQuery_noEvent( &sp, sl, getwave, map_gmt_stations, SLEEP_INTERVAL_TIME_SEC, verbose );

/********************************************************/
/*** create the rdseed.stations in the Data directory ***/
/********************************************************/

	snprintf( logit, sizeof(logit), "calling create_station_file" );
	writelog( logit, "main()", verbose );

	create_station_file( sl );

/********************************************************/
/**** use sac2gmtmap to create a map of the seismic stations ***/
/********************************************************/

/***************************************************/
/*** do sac polezero response request            ***/
/***************************************************/
	
	sprintf( respdir, "%s/Resp", working_directory );
	mkdirp2( respdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
	chdir( respdir );

	snprintf( logit, sizeof(logit), "calling doResponseQuery" );
	writelog( logit, "main()", verbose );

	doResponseQuery_noEvent( &sp, sl, getresp, SLEEP_INTERVAL_TIME_SEC, verbose );

	if(sp.isetupMT)
	{
		sprintf( datadir, "%s/dev", working_directory );
		mkdirp2( datadir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
		chdir( datadir );
		setupMT_create( &sp );
	}
	
	free(sl);
	snprintf( logit, sizeof(logit), "Bye Bye, ALL DONE!" );
	writelog( logit, "main()", verbose );

	exit(0);

} /*** end of main() ***/


/*****************************************************/
/*** doChannelQuery_noEvent()                      ***/
/*****************************************************/

void doChannelQuery_noEvent( Search_Window_Parameters *sp, char *filename, int verbose )
{
	char url_string[4096];
	char command_line[8196];

/*** write to log file ***/
	char logit[1024];
	void writelog( char *message, char *routine, int verbose );

	sprintf( sp->xml_filename, "channel.xml" );
	strcpy( filename, sp->xml_filename );


/*** modified so that all searches require a channel ***/

	if( sp->circle_area )
	{
sprintf( url_string, "%s/query?starttime=%s&endtime=%s&latitude=%g&longitude=%g&maxradius=%g&nodata=404&channel=%s&level=channel&format=xml&matchtimeseries=TRUE&includerestricted=FALSE&includeavailability=FALSE&includecomments=FALSE",
		sp->metadataWebservice_url,
		sp->start_string,
		sp->end_string,
		sp->clat,
		sp->clon,
		sp->radiusdeg,
		sp->chan );
	}

	if( sp->box_area )
	{
sprintf( url_string, "%s/query?starttime=%s&endtime=%s&channel=%s&level=channel&format=xml&maxlat=%g&minlon=%g&maxlon=%g&minlat=%g&matchtimeseries=TRUE&includerestricted=false&includeavailability=FALSE&includecomments=false&nodata=404",
		sp->metadataWebservice_url,
		sp->start_string,
		sp->end_string,
		sp->chan,
		sp->box_maxlat,
		sp->box_minlon,
		sp->box_maxlon,
		sp->box_minlat );
	}

	if( sp->single_station )
	{
sprintf( url_string, "%s/query?net=%s&sta=%s&loc=%s&channel=%s&starttime=%s&endtime=%s&matchtimeseries=TRUE&level=channel&format=xml&includerestricted=FALSE&includecomments=FALSE&nodata=404",
		sp->metadataWebservice_url,
		sp->net,
		sp->sta,
		sp->loc,
		sp->chan,
		sp->start_string,
		sp->end_string );
	}

	sprintf( command_line, "%s -o %s \"%s\"", sp->curl_options, sp->xml_filename, url_string );
	
	snprintf( logit, sizeof(logit), "%s", command_line );
        writelog( logit, "doChannelQuery()", verbose );

	system( command_line );

} /*** end of doChannelQuery_noEvent() ***/







/*****************************************************/
/*** doWaveformQuery_noEvent() ***/
/*****************************************************/

typedef struct { char fn[256]; } FileNameList;

void doWaveformQuery_noEvent( Search_Window_Parameters *sp, StationList *sl, int getwave, 
	int map_gmt_stations, int SLEEP_INTERVAL_TIME_SEC, int verbose )
{
	int i, j, k;
	char url_string[4096];
	char command_line[8196];
	int status = 1;

/*** webservice only accepts null location code as "--" ***/
	char loc[8];

	int number_of_files = 0;
	FILE *fp;

	int ifile, ncount = 0;
	FileNameList *sacfilelist_from_unzip;
	FileNameList *getsacfilenames( char *inputfilename, int *ncount, FileNameList *sacfilelist_from_unzip );

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
                        Search_Window_Parameters *sp,
                        int verbose );

/*** begin subroutinue ***/

	for( i = 0; i < sl->numNets; i++ )
	{
		for( j = 0; j < sl->n[i].nstation; j++ )
		{
			sl->n[i].s[j].usedata = 1;

			for( k = 0; k < sl->n[i].s[j].nchannel; k++ )
			{
				/***   yr.jday.hr.min.sec.msec.net.sta.loc.chan.sac ***/
				sprintf( sl->n[i].s[j].c[k].sacfilename,
					"%4d.%03d.%02d.%02d.%02d.%04d.%s.%s.%s.%s.SAC",
					sp->st.year,
					sp->st.jday,
					sp->st.hour,
					sp->st.min,
					sp->st.isec,
					sp->st.msec,
					sl->n[i].code,
					sl->n[i].s[j].code,
					sl->n[i].s[j].c[k].locationCode,
					sl->n[i].s[j].c[k].code );

				if( strcmp( sl->n[i].s[j].c[k].locationCode, "" ) == 0 )
					strcpy( loc, "--" );
				else
					strcpy( loc, sl->n[i].s[j].c[k].locationCode );


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
					sp->start_string, 
					sp->WaveFormDurationSec );
	
	/***) add -s for slient ***/
	/*** change output file from sl->n[i].s[j].c[k].sacfilename to sac.zip file ***/
	/****/
				sprintf( command_line, "%s -o %s \"%s\"", sp->curl_options, "sac.zip", url_string );
	
				snprintf( logit, sizeof(logit), "%s", command_line );
				writelog( logit, "doWaveformQuery()", verbose );

				if(getwave)
				{
					system( command_line );
					sleep(SLEEP_INTERVAL_TIME_SEC);

				/************/
				/*** use unzip to list the number of files in the zip file ***/
				/*** unzip_executable=/usr/bin/unzip ***/
				/*** writes out zipinfo header to sac.txt file ***/
				/*** open sac.txt file and read first integer otherwise assume default 1 file ***/
				/************/
					sprintf( command_line, "%s -Zt sac.zip > sac.count.txt", sp->unzip_executable );
					system( command_line );
					
					sprintf( command_line, "%s -Z1 sac.zip > sac.files.txt", sp->unzip_executable );
					system( command_line );

					sprintf( command_line, "%s sac.zip", sp->unzip_executable );
					system( command_line );

					number_of_files = 1; /*** default ***/
					if( (fp = fopen("sac.count.txt", "r")) == NULL )
					{
						snprintf( logit, sizeof(logit), "cannot open file sac.count.txt" );
						writelog( logit, "doWaveformQuery()", verbose );
					}
					else
					{
						fscanf(fp, "%d", &number_of_files);
						fclose(fp);
						system( "/bin/rm -f sac.count.txt" );
					}
					snprintf( logit, sizeof(logit), "number_of_files = %d", number_of_files );
					writelog( logit, "doWaveformQuery()", verbose );


				/*** format options: output(deprecared)=sacbl SAC binary little-endian / change to format=sac.zip ***/
				/*  AZ.BZN..BHE.M.2020.172.112427.SAC  --->   2020.172.11.24.27.0440.AZ.BZN..BHE.SAC */
				/**** after unzipping file then move single file to proper sacfile name else use ****/
				/***  sacmerge on multiple sacfiles, and remove the individual SAC files ***/


					sacfilelist_from_unzip = getsacfilenames( "sac.files.txt", &ncount, sacfilelist_from_unzip );
					system( "/bin/rm -f sac.files.txt" );

					for( ifile = 0; ifile < ncount; ifile++ )
					{
						snprintf( logit, sizeof(logit), "sacfilelist_from_unzip = %d of %d %s",
							(ifile+1), ncount, sacfilelist_from_unzip[ifile].fn );
						writelog( logit, "doWaveformQuery()", verbose );
					}

					if( number_of_files == 1 )
					{
					/*
					  sprintf( command_line, "%s %s.%s.%s.%s.?.%4d.%03d.%02d%02d%02d.SAC %s",
						sp->move_executable,
						sl->n[i].code,
						sl->n[i].s[j].code,
						sl->n[i].s[j].c[k].locationCode,
						sl->n[i].s[j].c[k].code,
						sp->st.year,
                                		sp->st.jday,
                                		sp->st.hour,
                                		sp->st.min,
                                		sp->st.isec,
						sl->n[i].s[j].c[k].sacfilename );
					*/
					/*** /usr/bin/mv NET.STA.LOC.CHAN.M.YYYY.JJJ.HHMMSS.SAC YYYY.JJJ.HH.MM.SS.UUUU.NET.STA.LOC.CHAN.SAC ***/
						sprintf( command_line, "%s %s %s", 
							sp->move_executable,
							sacfilelist_from_unzip[0].fn,
							sl->n[i].s[j].c[k].sacfilename );

						system( command_line );
					}
					else
					{
					/*** sacmerge ***/
					 			/****  net.sta.loc.chan.M.year.jday.hhmmss.SAC ***/
								/****  AZ.BZN..BHE.M.2020.172.112427.SAC  ****/
						sprintf( command_line, "%s %s.%s.%s.%s.?.%4d.???.??????.SAC",
							sp->sacmerge_executable, 
                                                	sl->n[i].code,
                                                	sl->n[i].s[j].code,
                                                	sl->n[i].s[j].c[k].locationCode,
                                                	sl->n[i].s[j].c[k].code,
                                                	sp->st.year );

					  	system( command_line );

					/*** mv out.sac sacfilename ***/
						sprintf( command_line, "%s out.sac %s",
							sp->move_executable,
							sl->n[i].s[j].c[k].sacfilename );

						system( command_line );
	
					/*** rm unzip SAC from zip ***/

						sprintf( command_line, "/bin/rm -f %s.%s.%s.%s.?.%4d.???.??????.SAC",
							sl->n[i].code,
							sl->n[i].s[j].code,
							sl->n[i].s[j].c[k].locationCode,
							sl->n[i].s[j].c[k].code,
							sp->st.year );

						system( command_line );
					}

				/*** read the SAC file back in and populate some header values ***/

					status = FixSacHeader( sl->n[i].s[j].c[k].sacfilename,
						sl->n[i].s[j].latitude,
						sl->n[i].s[j].longitude,
						sl->n[i].s[j].elevation,
						sl->n[i].s[j].c[k].depth,
						-12345.0, /*evla*/
						-12345.0, /*evlo*/
						-12345.0, /*evdp*/
						sl->n[i].s[j].c[k].azimuth,
						sl->n[i].s[j].c[k].dip,
						sp,
						verbose );

				/*** sac file had read ERROR, probably no data return from web server ***/
				/**** so the station may have good BHx channels but no HHx chans ***/
				/***
					if( status == 0 )
						sl->n[i].s[j].usedata = 0;
					else
						sl->n[i].s[j].usedata = 1;
				****/
					
					if( status == 0 )
						sl->n[i].s[j].c[k].usechan = 0;
					else
						sl->n[i].s[j].c[k].usechan = 1;

					snprintf( logit, sizeof(logit), "sacfilename=%s status=%d (usedata=0(no)/1(yes))",
						sl->n[i].s[j].c[k].sacfilename, status );
						
					writelog( logit, "doWaveformQuery()", verbose );

					if( status == 1 ) 
					{
						system( "/bin/rm sac.zip" );
						/* debug */
						/*
						sprintf( command_line, "%s sac.zip %s.zip",
							sp->move_executable,
							sl->n[i].s[j].c[k].sacfilename );
						system( command_line );
						*/
						
					}
					else
					{
						sprintf( command_line, "%s sac.zip %s.bad.zip", 
							sp->move_executable,
							sl->n[i].s[j].c[k].sacfilename );
                                        	system( command_line );
					}
					
				} /*** getwave true ***/

			} /* k-channel */

		} /* j-station */

	} /* i-network */

	if( map_gmt_stations )
	{
		sprintf( command_line, "%s -gmt5 -include-sta-label *Z.SAC", sp->sac2gmtmap_executable );
		system( command_line );
	}

} /*** end of doWaveformQuery() ***/

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
                        Search_Window_Parameters *sp,
                        int verbose )
{
	FILE *fp;
	Sac_Header s;
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

	if( (data = (float *)readsac( &s, sacfilename, verbose )) == NULL )
	{
		return 0;
	}
	else
	{
		snprintf( logit, sizeof(logit), "open file %s", sacfilename );
		writelog( logit, "FixSacHeader()", verbose );
	}

/*** add these header values ***/
	
	if( s.stla   == -12345 ) s.stla   = stla;
        if( s.stlo   == -12345 ) s.stlo   = stlo;
        if( s.stel   == -12345 ) s.stel   = stel;
        if( s.stdp   == -12345 ) s.stdp   = stdp;
        if( s.evla   == -12345 ) s.evla   = evla;
        if( s.evlo   == -12345 ) s.evlo   = evlo;
        if( s.evdp   == -12345 ) s.evdp   = evdp;

/*** dip = -90 vertical -> cmpinc = 0 ***/
/*** dip = 0 horizontal -> cmpinc = 90 ***/
/*** first change inc = dip + 90, dont change azimuth requires no conversion ***/

        if( s.cmpaz  == -12345 ) s.cmpaz  = cmpaz;
        if( s.cmpinc == -12345 ) s.cmpinc = cmpinc + 90; /*** inc = dip + 90 ***/

/*** do some az and dip error checking ***/
/*** case here is both az=0 and dip=0 NULL return from XML file ***/

        if( ( s.kcmpnm[2] == 'Z' ) && ( cmpaz == 0 ) && ( cmpinc == 0 ) )
        {
                s.cmpaz = 0;
                s.cmpinc= 0;
        }

        if( ( (s.kcmpnm[2] == 'N') || (s.kcmpnm[2] == '1') ) && ( ( cmpaz == 0 ) && ( cmpinc == 0 ) ) )
        {
                s.cmpaz = 0;
                s.cmpinc = 90;
        }

        if( ( (s.kcmpnm[2] == 'E') || (s.kcmpnm[2] == '2') ) && ( ( cmpaz == 0 ) && ( cmpinc == 0 ) ) )
        {
                s.cmpaz = 90;
                s.cmpinc = 90;
        }

/*** cases where horizontal component but dip is not 0(horizontal) some cases -90? ***/

        if( ( (s.kcmpnm[2] == 'N') || (s.kcmpnm[2] == '1') ) && ( cmpinc != 0 ) )
                s.cmpinc = 90;
        if( ( (s.kcmpnm[2] == 'E') || (s.kcmpnm[2] == '2') ) && ( cmpinc != 0 ) )
                s.cmpinc = 90;

/*** write SAC file back out ***/

        fp = fopen( sacfilename, "wb" );
        fwrite( &s, sizeof(Sac_Header), 1, fp );
        fwrite( data, s.npts * sizeof(float), 1, fp );
        fclose(fp);
        free(data);

/*** check for full waveform query return not just small segment, this stops/hangs/crashes sacdata2inv.c ***/

	TotalTimeDuration = 1;

	if( fabs( s.e - s.b ) < TotalTimeDuration || ( s.npts < ( TotalTimeDuration / s.delta ) ) )
        {
		snprintf( logit, sizeof(logit), "%s: Time Duration (s.e-s.b)=%g < TotalTimeDuration = %g  .OR.   npts=%d < ( TotalTimeDuration / s.delta=%g ) = %g",
			sacfilename,
                        (s.e - s.b),
                        TotalTimeDuration,
                        s.npts,
                        s.delta,
                        ( TotalTimeDuration / s.delta ) );
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
/*** doResponseQuery_noEvent() ***/
/**************************************************************************************************************************************/
void doResponseQuery_noEvent( 
	Search_Window_Parameters *sp,
	StationList *sl, 
	int getresp, int SLEEP_INTERVAL_TIME_SEC, int verbose )
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
			for( k = 0; k < sl->n[i].s[j].nchannel; k++ )
			{
				if( sl->n[i].s[j].c[k].usechan == 0 )
				{
					snprintf( logit, sizeof(logit), "i=%d j=%d k=%d usechan = %d",
						i, j, k, sl->n[i].s[j].c[k].usechan );
					writelog( logit, "doResponseQuery", verbose );
					continue;
				}

				sprintf( sacpzfilename, "SAC_PZs_%s_%s_%s_%s_%04d.%03d.%02d.%02d.%02d.%04d_",
					sl->n[i].code,
					sl->n[i].s[j].code,
					sl->n[i].s[j].c[k].code,
					sl->n[i].s[j].c[k].locationCode,
					sp->st.year,
					sp->st.jday,
					sp->st.hour,
					sp->st.min,
					sp->st.isec,
					sp->st.msec );
				
				if( strcmp( sl->n[i].s[j].c[k].locationCode, "" ) == 0 )
					strcpy( loc, "--" );
				else
					strcpy( loc, sl->n[i].s[j].c[k].locationCode );

				sprintf( url_string,
				  "%s/query?network=%s&station=%s&location=%s&channel=%s&starttime=%s&endtime=%s",
					sp->responseWebservice_url,
					sl->n[i].code,
					sl->n[i].s[j].code,
					loc,
					sl->n[i].s[j].c[k].code,
					sp->start_string,
					sp->end_string );

				sprintf( command_line, "%s -o %s \"%s\"", sp->curl_options, sacpzfilename, url_string );

				snprintf( logit, sizeof(logit), "%s", command_line );
				writelog( logit, "doResponseQuery()", verbose );
				
				if(getresp)
				{
					system( command_line );
					sleep(SLEEP_INTERVAL_TIME_SEC);
				}

			
			} /*** loop over k-channel ***/

		} /*** loop over j-station ***/

	} /*** loop over i-network ***/

} /*** end of doResponseQuery() ***/


void setupMT_create( Search_Window_Parameters *sp )
{
	char command_line[8196];
	FILE *fp;

	fp = fopen( "setup.csh", "w" );

	fprintf( fp, "#!/bin/csh\n" );
	fprintf( fp, "###\n" );
	fprintf( fp, "###\n" );
	fprintf( fp, "###\n" );

	fprintf( fp, "cat >! exclude_sta.txt << EOF\n" );
	fprintf( fp, "EOF\n" );

	fprintf( fp, "%s -exclude_sta ./exclude_sta.txt -depth %g -ev %g %g -velocity_model %s -origintime \"%s\" -realtime -gmt5 ../Data/*.?HZ.SAC\n",
		sp->setupMT_executable,
		sp->evdp,
		sp->evla,
		sp->evlo,
		sp->velocity_model,
		sp->origintime_string );
	fclose(fp);

	chmod( "setup.csh", S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH );

	system( "setup.csh" );

} /*** setupMT_create() ***/


FileNameList *getsacfilenames( char *inputfilename, int *ncount, FileNameList *sacfilelist_from_unzip )
{
	FILE *fp;
	int i = 0;
	int status;

	sacfilelist_from_unzip = calloc( 1, sizeof(FileNameList) );

	fp  = fopen( inputfilename, "r" );
	do {
		sacfilelist_from_unzip = realloc( sacfilelist_from_unzip, (i+1)*sizeof(FileNameList) );
		status = fscanf( fp, "%s", sacfilelist_from_unzip[i].fn );
		i++;
	}
	while( status > 0 );

	*ncount = i - 1;
	fclose(fp);

	return (FileNameList *)sacfilelist_from_unzip;
}
