#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include "../include/mytime.h"
#include "../include/event.h"
#include "../include/station.h"

char progname[256];

void writeStation( StationList *sl )
{
	int i, j, k;

	for( i = 0; i < sl->numNets; i++ )
	{
		/* 
		fprintf( stdout, "%s: %s: %s: net=%s start=%4d-%02d-%02d end=%4d-%02d-%02d desc=(%s)\n",
			progname, __FILE__, __func__, 
			sl->n[i].code,
			sl->n[i].startDate->year,
			sl->n[i].startDate->month,
			sl->n[i].startDate->mday,
			sl->n[i].endDate->year,
			sl->n[i].endDate->month,
			sl->n[i].endDate->mday,
			sl->n[i].Description );
		fflush(stdout);
		*/

		for( j = 0; j < sl->n[i].nstation; j++ )
		{
			fprintf( stdout, "%s: %s: %s: sta=%s start=%4d-%02d-%02d end=%4d-%02d-%02d lat=%g lon=%g elev=%g name=(%s)\n",
				progname, __FILE__, __func__, 
				sl->n[i].s[j].code,
				sl->n[i].s[j].startDate->year,
				sl->n[i].s[j].startDate->month,
				sl->n[i].s[j].startDate->mday,
				sl->n[i].s[j].endDate->year,
				sl->n[i].s[j].endDate->month,
				sl->n[i].s[j].endDate->mday,
				sl->n[i].s[j].latitude,
				sl->n[i].s[j].longitude,
				sl->n[i].s[j].elevation,
				sl->n[i].s[j].name );
			fflush(stdout);

			for( k = 0; k < sl->n[i].s[j].nchannel; k++ )
			{
				fprintf( stdout,
"%s: %s: %s:  %s.%s.%s.%s %4d-%02d-%02d %4d-%02d-%02d depth=%g azi=%g dip=%g sr=%g desc=(%s) sens=%g f=%g in=(%s) out=(%s)\n",
					progname, __FILE__, __func__, 
					sl->n[i].code, 
					sl->n[i].s[j].code,
					sl->n[i].s[j].c[k].locationCode,
					sl->n[i].s[j].c[k].code,
					sl->n[i].s[j].c[k].startDate->year,
					sl->n[i].s[j].c[k].startDate->month,
					sl->n[i].s[j].c[k].startDate->mday,
					sl->n[i].s[j].c[k].endDate->year,
					sl->n[i].s[j].c[k].endDate->month,
					sl->n[i].s[j].c[k].endDate->mday,
					sl->n[i].s[j].c[k].depth,
					sl->n[i].s[j].c[k].azimuth,
					sl->n[i].s[j].c[k].dip,
					sl->n[i].s[j].c[k].samplerate,
					sl->n[i].s[j].c[k].sensor_description,
					sl->n[i].s[j].c[k].InstrumentSensitivity,
					sl->n[i].s[j].c[k].frequency,
					sl->n[i].s[j].c[k].inputUnits,
					sl->n[i].s[j].c[k].outputUnits );
				fflush(stdout);
			}
		}
	}
}

void writeEventList( EventList *e )
{
	int i, j, k;

	for( k = 0; k < e->nevents; k++ )
	{
		fprintf( stdout,
	"%s: norigins=%d publicID=%s prefor=%s desc=%s type=%s agency=%s lddate=%s\n",
                        progname,
                        e->ev[k].norigins,
                        e->ev[k].publicID,
                        e->ev[k].preferredOriginID,
                        e->ev[k].description,
                        e->ev[k].type,
                        e->ev[k].cinfo.agency,
                        e->ev[k].cinfo.creationTime_string );

		fflush(stdout);

		for( j = 0; j < e->ev[k].norigins; j++ )
                {
                        fprintf( stdout,
"\t %4d/%02d/%02dT%02d:%02d:%05.2f%s lat=%g lon=%g z=%g nass=%d npha=%d err=%g azgap=%g mind=%g eval=%s eventid=%s agency=%s auth=%s lddate=%s ",
				e->ev[k].o[j].origintime->year,
                                e->ev[k].o[j].origintime->month,
                                e->ev[k].o[j].origintime->mday,
                                e->ev[k].o[j].origintime->hour,
                                e->ev[k].o[j].origintime->min,
                                e->ev[k].o[j].origintime->fsec,
                                e->ev[k].o[j].origintime->tzone,
                                e->ev[k].o[j].latitude,
                                e->ev[k].o[j].longitude,
                                e->ev[k].o[j].depth,
                                e->ev[k].o[j].associatedStationCount,
                                e->ev[k].o[j].usedPhaseCount,
                                e->ev[k].o[j].standardError,
                                e->ev[k].o[j].azimuthalGap,
                                e->ev[k].o[j].minimumDistance,
                                e->ev[k].o[j].evaluationMode,
				e->ev[k].eventid,
                                e->ev[k].o[j].cinfo.agency,
                                e->ev[k].o[j].cinfo.author,
                                e->ev[k].o[j].cinfo.creationTime_string );

			fflush(stdout);

			for( i = 0; i < e->ev[k].o[j].nmags; i++ )
                        {
                                fprintf( stdout,
	" magtype=%s mag=%g uncert=%g nsta=%d eval=%s agency=%s auth=%s lddate=%s ",
                                        e->ev[k].o[j].mag[i].magtype,
                                        e->ev[k].o[j].mag[i].magnitude,
                                        e->ev[k].o[j].mag[i].uncertainty,
                                        e->ev[k].o[j].mag[i].stationCount,
                                        e->ev[k].o[j].mag[i].evaluationMode,
                                        e->ev[k].o[j].mag[i].cinfo.agency,
                                        e->ev[k].o[j].mag[i].cinfo.author,
                                        e->ev[k].o[j].mag[i].cinfo.creationTime_string );

				fflush(stdout);

			} /*** loop over magnitudes ***/

			fprintf( stdout, "publicID=%s\n", 
				e->ev[k].o[j].publicID );
	
			fflush(stdout);

		} /*** loop over origins ***/

	} /*** loop over events ***/
}

void writeEvents2Log( EventList *e )
{
        int i, j, k;
	int verbose = 0;

/*** write to log file ***/
        char logit[1024];
        void writelog( char *line, char *routine, int verbose );

	float delaytime;
	MyTime *now;
        void initialize_mytime( MyTime *t );
	MyTime *myGMTtime( MyTime *t );
	double calcTimeAgo( MyTime *now, MyTime *evtime, char *return_type );

/*** begin ***/
	now = calloc(1,sizeof(MyTime));
	initialize_mytime( now );
	now = myGMTtime( now );
	
	for( k = 0; k < e->nevents; k++ )
        {
                for( j = 0; j < e->ev[k].norigins; j++ )
                {
			delaytime = calcTimeAgo( now, e->ev[k].o[j].origintime, "HRS" );

                        snprintf( logit, sizeof(logit),
        "%3d %4.1f-hrs %3s=%4.2f (%48s) %s lat=%8.3f lon=%9.3f z=%7.2f eventid=%s agency=%s auth=%s lddate=%s",
                                k,
				delaytime, 
                                e->ev[k].o[j].mag[0].magtype,
                                e->ev[k].o[j].mag[0].magnitude,
                                e->ev[k].description,
                                e->ev[k].o[j].originTime_string,
                                e->ev[k].o[j].latitude,
                                e->ev[k].o[j].longitude,
                                e->ev[k].o[j].depth,
				e->ev[k].eventid,
                                e->ev[k].o[j].cinfo.agency,
                                e->ev[k].o[j].cinfo.author,
                                e->ev[k].o[j].cinfo.creationTime_string );

                        writelog( logit, "writeEvents2Log()", verbose );
                }
        }
	free(now);
}

void writeEvent2File( Event *e, char *filename )
{
        int j;
        FILE *fp;

	float delaytime;
        MyTime *now;
        void initialize_mytime( MyTime *t );
        MyTime *myGMTtime( MyTime *t );
        double calcTimeAgo( MyTime *now, MyTime *evtime, char *return_type );

/*** begin ***/
        now = calloc(1,sizeof(MyTime));
        initialize_mytime( now );
        now = myGMTtime( now );

        fp = fopen( filename, "w" );
        for( j = 0; j < e->norigins; j++ )
        {

		delaytime = calcTimeAgo( now, e->o[j].origintime, "HRS" );

		fprintf( fp, 
        "age=%4.1f-hrs %3s=%4.2f (%48s) OT str=(%s) %4d-%02d-%02dT%02d:%02d:%06.3f epoch=%lf lat=%8.3f lon=%9.3f z=%7.2f eventid=%s evid=%ld agency=%s auth=%s lddate=%s\n",
			delaytime, 
                        e->o[j].mag[0].magtype,
                        e->o[j].mag[0].magnitude,
                        e->description,
                        e->o[j].originTime_string,
			e->o[j].origintime->year, e->o[j].origintime->month, e->o[j].origintime->mday,
			e->o[j].origintime->hour, e->o[j].origintime->min,   e->o[j].origintime->fsec,
			e->o[j].origintime->epoch,
                        e->o[j].latitude,
                        e->o[j].longitude,
                        e->o[j].depth,
			e->eventid, 
			e->evid,
                        e->o[j].cinfo.agency,
                        e->o[j].cinfo.author,
                        e->o[j].cinfo.creationTime_string );
        }
        fclose(fp);
	free(now);
}

/**************************************************************************************************************************************/
/*** create_station_file ***/
/**************************************************************************************************************************************/

/*
GNI IU 40.1480 44.7410  1609 "BH1 BH2 BHZ " "Garni, Armenia" 2010,280,00:00:00 2599,365,23:59:59
KBL IU 34.5408 69.0432  1920 "BHE BHN BHZ " "Kabul, Afghanistan" 2006,313,00:00:00 2599,365,23:59:59
*/

void create_station_file( StationList *sl )
{
	FILE *fp;
	int i, j, k;
	int verbose = 0;

/*** write to log file ***/
	char logit[1024];
	void writelog( char *message, char *routine, int verbose );

	if( (fp = fopen( "rdseed.stations", "a" )) == NULL )
	{
		snprintf( logit, sizeof(logit), "cannot open rdseed.stations for writting" );
		writelog( logit, "create_station_file()", verbose );
		exit(-1);
	}

	for( i = 0; i < sl->numNets; i++ )
	{
		for( j = 0; j < sl->n[i].nstation; j++ )
		{
			if( sl->n[i].s[j].usedata == 1 )
			{
				fprintf( fp, "%s %s %.4f %.4f %5.0f \"",
					sl->n[i].s[j].code,
					sl->n[i].code,
					sl->n[i].s[j].latitude,
					sl->n[i].s[j].longitude,
					sl->n[i].s[j].elevation );

				for( k = 0; k < sl->n[i].s[j].nchannel; k++ )
				{
					fprintf( fp, "%s%s ", 
						sl->n[i].s[j].c[k].code,
						sl->n[i].s[j].c[k].locationCode );
				}

				fprintf( fp, "\" \"%s\" %04d,%03d,%02d:%02d:%02d %04d,%03d,%02d:%02d:%02d\n",
					sl->n[i].s[j].name,
					sl->n[i].s[j].startDate->year,
					sl->n[i].s[j].startDate->jday,
					sl->n[i].s[j].startDate->hour,
					sl->n[i].s[j].startDate->min,
					sl->n[i].s[j].startDate->isec,
					sl->n[i].s[j].endDate->year,
					sl->n[i].s[j].endDate->jday,
					sl->n[i].s[j].endDate->hour,
					sl->n[i].s[j].endDate->min, 
					sl->n[i].s[j].endDate->isec );

			} /*** if usedata true ***/
		} /*** loop over j stations ***/
	} /*** loop over i networks ***/
	fclose(fp);

} /*** end of create_station_file() ***/


void write_sacpaz( StationList *sl )
{
	/*i-net, j-sta, k-chan, ik-polezero */
	int i, j, k, ik;
	FILE *fp;
	char sacpaz_filename[256];
	MyTime *now;
	char *MyTime2StandardString2( MyTime *t );
	MyTime *myGMTtime( MyTime *t );
	now = calloc(1,sizeof(MyTime));

	for( i = 0; i < sl->numNets; i++ )
	{
		for( j = 0; j < sl->n[i].nstation; j++ )
		{
			if( sl->n[i].s[j].usedata == 1 )
			{
				for( k = 0; k < sl->n[i].s[j].nchannel; k++ )
				{
					sprintf( sacpaz_filename,
			"SAC_PZs_%s_%s_%s_%s_%4d-%02d-%02dT%02d%02d%02d_%4d-%02d-%02dT%02d%02d%02d",
						sl->n[i].code,
						sl->n[i].s[j].code,
						sl->n[i].s[j].c[k].code,
						sl->n[i].s[j].c[k].locationCode,
						sl->n[i].s[j].c[k].startDate->year,
						sl->n[i].s[j].c[k].startDate->month,
						sl->n[i].s[j].c[k].startDate->mday,
						sl->n[i].s[j].c[k].startDate->hour,
						sl->n[i].s[j].c[k].startDate->min,
						sl->n[i].s[j].c[k].startDate->isec,
						sl->n[i].s[j].c[k].endDate->year,
						sl->n[i].s[j].c[k].endDate->month, 
						sl->n[i].s[j].c[k].endDate->mday, 
						sl->n[i].s[j].c[k].endDate->hour, 
						sl->n[i].s[j].c[k].endDate->min, 
						sl->n[i].s[j].c[k].endDate->isec );

					fprintf( stdout, "%s: opening file %s\n",
						progname, sacpaz_filename );

					if( (fp = fopen( sacpaz_filename, "w" )) == NULL )
					{
						fprintf( stderr, "%s: Cannot open file %s for writting\n", progname, sacpaz_filename );
						exit(-1);
					}

					fprintf( fp, "* **********************************\n" );
					fprintf( fp, "* NETWORK   (KNETWK): %s\n", sl->n[i].code );
					fprintf( fp, "* STATION    (KSTNM): %s\n", sl->n[i].s[j].code );
					fprintf( fp, "* LOCATION   (KHOLE): %s\n", sl->n[i].s[j].c[k].locationCode );
					fprintf( fp, "* CHANNEL   (KCMPNM): %s\n", sl->n[i].s[j].c[k].code );
					fprintf( fp, "* CREATED           : %s\n", MyTime2StandardString2( myGMTtime(now) ) );
					fprintf( fp, "* START             : %s\n", MyTime2StandardString2( sl->n[i].s[j].c[k].startDate ) );
					fprintf( fp, "* END               : %s\n", MyTime2StandardString2( sl->n[i].s[j].c[k].endDate ) );
					fprintf( fp, "* DESCRIPTION       : %s\n", sl->n[i].s[j].name );
					fprintf( fp, "* LATITUDE          : %g\n", sl->n[i].s[j].latitude );
					fprintf( fp, "* LONGITUDE         : %g\n", sl->n[i].s[j].longitude );
					fprintf( fp, "* ELEVATION         : %g\n", sl->n[i].s[j].elevation );
					fprintf( fp, "* DEPTH             : %g\n", sl->n[i].s[j].c[k].depth );
					fprintf( fp, "* DIP               : %g\n", sl->n[i].s[j].c[k].dip );
					fprintf( fp, "* AZIMUTH           : %g\n", sl->n[i].s[j].c[k].azimuth );
					fprintf( fp, "* SAMPLE RATE       : %g\n", sl->n[i].s[j].c[k].samplerate );
					fprintf( fp, "* INPUT UNIT        : %s\n", sl->n[i].s[j].c[k].r.InputUnits );
					fprintf( fp, "* OUTPUT UNIT       : %s\n", sl->n[i].s[j].c[k].r.OutputUnits );
					fprintf( fp, "* INSTTYPE          : %s\n", sl->n[i].s[j].c[k].sensor_description );
					fprintf( fp, "* INSTGAIN          : %e (%s) @ %g Hz\n",
						sl->n[i].s[j].c[k].r.StageGain,
						sl->n[i].s[j].c[k].r.Stage_One_OutputUnits,
						sl->n[i].s[j].c[k].r.StageGain_Frequency );
					fprintf( fp, "* COMMENT           : %s\n", sl->n[i].s[j].c[k].r.PzTransferFunctionType );
					fprintf( fp, "* SENSITIVITY       : %e (%s) @ %g Hz\n", 
						sl->n[i].s[j].c[k].r.InstrumentSensitivity,
						sl->n[i].s[j].c[k].r.OutputUnits,
						sl->n[i].s[j].c[k].r.InstrumentSensitivity_Frequency );
					fprintf( fp, "* A0                : %e @ %g Hz\n",
						sl->n[i].s[j].c[k].r.NormalizationFactor,
						sl->n[i].s[j].c[k].r.NormalizationFrequency );
					fprintf( fp, "* **********************************\n" );


				/*
					if( sl->n[i].s[j].c[k].r.Number_Zeros == 0 )
					{
						sl->n[i].s[j].c[k].r.Number_Zeros = 2;
						for( ik = 0; ik < sl->n[i].s[j].c[k].r.Number_Zeros; ik++ )
						{
							sl->n[i].s[j].c[k].r.zeros[ik].Real = 0;
							sl->n[i].s[j].c[k].r.zeros[ik].Imaginary = 0;
						}
					}
				*/
				/*** what to do with the zeros that are not at the origin of complex plane ***/
					
				/*** write out the zeros ****/
					fprintf( fp, "ZEROS %d\n", sl->n[i].s[j].c[k].r.Number_Zeros ); 
					for( ik = 0; ik < sl->n[i].s[j].c[k].r.Number_Zeros; ik++ )
					{
						fprintf( fp, "%+20.7e %+20.7e\n",
							sl->n[i].s[j].c[k].r.zeros[ik].Real,
							sl->n[i].s[j].c[k].r.zeros[ik].Imaginary );
					}

				/*** write out the poles ***/
					fprintf( fp, "POLES %d\n", sl->n[i].s[j].c[k].r.Number_Poles );

					for( ik = 0; ik < sl->n[i].s[j].c[k].r.Number_Poles; ik++ )
					{
						fprintf( fp, "%+20.7e %+20.7e\n",
							sl->n[i].s[j].c[k].r.poles[ik].Real,
							sl->n[i].s[j].c[k].r.poles[ik].Imaginary );
					}

				/*** write out the constant ***/
					fprintf( fp, "CONSTANT %+20.7e\n",
						( sl->n[i].s[j].c[k].r.InstrumentSensitivity * 
						  sl->n[i].s[j].c[k].r.NormalizationFactor ) );

					fclose(fp);
				}

			} /*** loop over k-channels ***/

		} /*** loop over j-stations ***/

	} /*** loop over i-networks ***/

	free(now);
}
