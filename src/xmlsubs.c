#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include "/Users/ichinose1/Work/mxml-2.10/mxml.h"

#include "../include/mytime.h"
#include "../include/event.h"
#include "../include/station.h"

#define STRING_NODE 0
#define FLOAT_NODE  1
#define INTEGER_NODE 2
#define FLOAT_NULL -999999999.0
#define INTEGER_NULL -999999999

char progname[256];

MyTime *getTimeNode(  mxml_node_t *node_val, char *timestring )
{
	char stringbuffer[65];
	MyTime *t;

	void initialize_mytime( MyTime *t );
	void parsestring( MyTime *t, char *str );
	void WriteMyTime2STDOUT( MyTime *t );
        void WriteMyTime2STDERR( MyTime *t );

	if( mxmlGetOpaque( node_val ) != NULL )
	{

	/*** allocate and initialize memory ***/
		t = (MyTime *)calloc( 1, sizeof(MyTime) );
		initialize_mytime( t );

	/*** copy the string from the xml node ***/
		strcpy( stringbuffer, mxmlGetOpaque( node_val ) );

	/*** parse the string ***/
		parsestring( t, stringbuffer );
		/* WriteMyTime2STDOUT( &t ); */
		/* WriteMyTime2STDERR( &t ); */

	/*** return the results in both string form and MyTime struction ***/
	/***
		strcpy( timestring, stringbuffer );
	***/

	/*** return rebuilt time string in my own format ***/

		sprintf( timestring, "%4d-%02d-%02dT%02d:%02d:%05.2f",
			t->year,
			t->month,
			t->mday,
			t->hour,
			t->min,	
			t->fsec );

		return(MyTime *)t;
	
	}

	strcpy( stringbuffer, " " );
	return(MyTime *)NULL;
}

char *getStringNode( mxml_node_t *node_val, char *out )
{
	if( mxmlGetOpaque( node_val ) != NULL )
	{
        	strcpy( out, mxmlGetOpaque( node_val ) );
        	return (char *) mxmlGetOpaque( node_val );
	}
	strcpy( out, "" );
	return(char *)NULL;
}

double getDoubleNode( mxml_node_t *node_val )
{
	if( mxmlGetOpaque( node_val ) != NULL )
	{
		return (double) atof( mxmlGetOpaque( node_val ) );
	}
	return FLOAT_NULL;
}

float getFloatNode( mxml_node_t *node_val )
{
	if( mxmlGetOpaque( node_val ) != NULL )
	{
        	return (float) atof( mxmlGetOpaque( node_val ) );
	}
	return FLOAT_NULL;
}

int getIntegerNode( mxml_node_t *node_val )
{
	if( mxmlGetOpaque( node_val ) != NULL )
	{
        	return (int) atoi( mxmlGetOpaque( node_val ) );
	}
	return INTEGER_NULL;
}

void print_node( mxml_node_t *node_tag, mxml_node_t *node_val, int type, float multscale )
{
        if( mxmlGetElement( node_tag ) != NULL && mxmlGetOpaque( node_val ) != NULL )
        {
                if( type == STRING_NODE )
                {
                        fprintf( stdout, "%s=(%s) ",
                                mxmlGetElement( node_tag ),
                                mxmlGetOpaque( node_val ) );
                }
                else if( type == FLOAT_NODE )
                {
                        fprintf( stdout, "%s=%g ",
                                mxmlGetElement( node_tag ),
                                atof( mxmlGetOpaque( node_val ) ) * multscale );
                }
                else if( type == INTEGER_NODE )
                {
                        fprintf( stdout, "%s=%d ",
                                mxmlGetElement( node_tag ),
                                atoi( mxmlGetOpaque( node_val ) ) * (int)multscale );
                }
        }
}

/**************************************************************************************************************/
/**** G.Ichinose Feb 28, 2019 ****/
/**** added time Mytime t of the origin to select the correct time epoch of the channel start/end dates ***/
/**** G.Ichinose Oct 13, 2022 ****/
/**** SeisComp3 does not have sequential zero numbers, add support for that, otherwise zeros=0 ***/
/**************************************************************************************************************/

int readStationXML( char *filename, StationList *sl, MyTime *t, int verbose )
{
	FILE *fp;
	int i, j, k, ik, ikik;
	int netcount, stacount, chancount;
	
	char value_int_string[8];
	mxml_node_t *tree;
	mxml_node_t *net, *sta, *chan, *sensor, *resp;

/*** sometimes the nodes extends down several layers in XML tags ****/

	mxml_node_t *node, *node1, *node2, *node3;

/*** write to log file ***/
        char logit[1024];
        void writelog( char *message, char *routine, int verbose );

/*** functions ***/

        MyTime *getTimeNode(  mxml_node_t *node_val, char *timestring );
        char *getStringNode(  mxml_node_t *node_val, char *out );
        float getFloatNode(   mxml_node_t *node_val );
	double getDoubleNode( mxml_node_t *node_val );
        int   getIntegerNode( mxml_node_t *node_val );
        void print_node( mxml_node_t *node_tag, mxml_node_t *node_val, int type, float multscale );

/**** mytime.h timesubs.c functions ****/

	void parsestring( MyTime *t, char *str );
	char *MyTime2StandardString2( MyTime *t );
	int mytime_between( MyTime *t0, MyTime *t1, MyTime *t );

	char null_date_string[32];
	const char *tmp_constant_string;

	strcpy( null_date_string, "2049-12-31T23:59:59.999" );

/*** start ***/

        if( (fp = fopen(filename, "r")) == NULL )
        {
                snprintf( logit, sizeof(logit), "cannot open file %s", filename );
		writelog( logit, "readStationXML", verbose );
                exit(-1);
        }
	snprintf( logit, sizeof(logit), "opening file %s", filename );
	writelog( logit, "readStationXML", verbose );

        tree = mxmlLoadFile( NULL, fp, MXML_OPAQUE_CALLBACK );
        fclose(fp);

/***************************/
/*** Network node        ***/
/***************************/
	netcount = 0;
	stacount = 0;
	chancount = 0;

	sl->numNets = 0;

	sl->n = calloc(1,sizeof(Network));

	for(	k = 0,
		net = mxmlFindElement( tree, tree, "Network", NULL, NULL, MXML_DESCEND );
		net != NULL;
		net = mxmlFindElement( net, tree, "Network", NULL, NULL, MXML_DESCEND ) )
	{

	/*** Network code ***/
		sl->n = realloc( sl->n, (k+1)*sizeof(Network));
		strcpy( sl->n[k].code, mxmlElementGetAttr( net, "code") );

	/** throw out temporary deployment networks **/

		if(  strcmp( sl->n[k].code , "PY" ) == 0 || strcmp( sl->n[k].code , "AZ" ) == 0 )
		{
			snprintf( logit, sizeof(logit), "net=%d skipping net=%s %s\n", k, sl->n[k].code, sl->n[k].Description );
			writelog( logit, "readStationXML", verbose );
			continue;		
		}

	/*** startDate ***/
		strcpy( sl->n[k].startDate_string, mxmlElementGetAttr( net, "startDate") );
		sl->n[k].startDate = calloc(1,sizeof(MyTime));
                parsestring( sl->n[k].startDate, sl->n[k].startDate_string );

	/*** endDate ***/
		tmp_constant_string = mxmlElementGetAttr( net, "endDate");
		sl->n[k].endDate = calloc(1,sizeof(MyTime));

		if( tmp_constant_string == NULL )
		{
			parsestring( sl->n[k].endDate, null_date_string );
		}
		else
		{
			parsestring( sl->n[k].endDate, sl->n[k].endDate_string );
		}

	/*** description ***/
		node = mxmlFindElement( net, net, "Description", NULL, NULL, MXML_DESCEND );
                getStringNode( node, sl->n[k].Description ); 

        /***************************/
        /*** Station nodes       ***/
        /***************************/
		sl->n[k].nstation = 0;

		sl->n[k].s = calloc(1,sizeof(Station));

		for(	i = 0,
			sta = mxmlFindElement( net, net, "Station", NULL, NULL, MXML_DESCEND );
			sta != NULL;
			sta = mxmlFindElement( sta, net, "Station", NULL, NULL, MXML_DESCEND ) )
		{

		/*** Station code ***/
			sl->n[k].s = realloc( sl->n[k].s, (i+1)*sizeof(Station));
			strcpy( sl->n[k].s[i].code, mxmlElementGetAttr( sta, "code") );

/************************************************************************************************************************************/
/*** throw out stations that are always bad ***/
/************************************************************************************************************************************/
			if(	( strcmp( sl->n[k].s[i].code, "NV32" ) == 0 ) ||
				( strcmp( sl->n[k].s[i].code, "NV33" ) == 0 ) 
			)
			{
				snprintf( logit, sizeof(logit), "netk=%d stai=%d skipping net=%s sta=%s\n",
					k, i, sl->n[k].code, sl->n[k].s[i].code ); 
				writelog( logit, "readStationXML", verbose );
				break;
			}
/************************************************************************************************************************************/

		/*** startDate ***/
			strcpy( sl->n[k].s[i].startDate_string, mxmlElementGetAttr( sta, "startDate") );
                	sl->n[k].s[i].startDate = calloc(1,sizeof(MyTime));
                	parsestring( sl->n[k].s[i].startDate, sl->n[k].s[i].startDate_string );

		/*** endDate ***/
		/*** not all tags have endDates, therefore convert NULL dates far into the future ***/
			tmp_constant_string = mxmlElementGetAttr( sta, "endDate");
			sl->n[k].s[i].endDate = calloc(1,sizeof(MyTime));

			if( tmp_constant_string == NULL )
				parsestring( sl->n[k].s[i].endDate, null_date_string );
			else
				parsestring( sl->n[k].s[i].endDate, sl->n[k].s[i].endDate_string );

		/*** Latitude ***/
			node = mxmlFindElement( sta, sta, "Latitude", NULL, NULL, MXML_DESCEND );
			sl->n[k].s[i].latitude = getFloatNode( node );

		/*** Longitude ***/
			node = mxmlFindElement( sta, sta, "Longitude", NULL, NULL, MXML_DESCEND );
			sl->n[k].s[i].longitude = getFloatNode( node );

		/*** Elevation ***/
			node = mxmlFindElement( sta, sta, "Elevation", NULL, NULL, MXML_DESCEND );
			sl->n[k].s[i].elevation = getFloatNode( node );

		/*** Site Name ***/
			node = mxmlFindElement( sta, sta, "Site", NULL, NULL, MXML_DESCEND );
			node = mxmlFindElement( sta, sta, "Name", NULL, NULL, MXML_DESCEND );
			getStringNode( node, sl->n[k].s[i].name );

			snprintf( logit, sizeof(logit), "netk=%d stai=%d net=%s sta=%s Latitude=%g Longitude=%g Elevation=%g Name=%s",
				k, i, sl->n[k].code, sl->n[k].s[i].code, 
				sl->n[k].s[i].latitude,
				sl->n[k].s[i].longitude,
				sl->n[k].s[i].elevation,
				sl->n[k].s[i].name );

			writelog( logit, "readStationXML", verbose );

                /***************************/
                /*** Channel Information ***/
                /***************************/
			sl->n[k].s[i].nchannel       = 0;
			sl->n[k].s[i].has_vertical   = 0;
			sl->n[k].s[i].has_northsouth = 0;
			sl->n[k].s[i].has_eastwest   = 0;
			sl->n[k].s[i].usedata        = 0;

			sl->n[k].s[i].c = calloc(1,sizeof(Channel));

			for( 	j = 0,
				chan = mxmlFindElement( sta, sta, "Channel", NULL, NULL, MXML_DESCEND );
				chan != NULL;
				chan = mxmlFindElement( chan, sta, "Channel", NULL, NULL, MXML_DESCEND ) )
			{
				sl->n[k].s[i].c = realloc( sl->n[k].s[i].c, (j+1)*sizeof(Channel) );

			/*** channel code "component code" ***/
				strcpy( sl->n[k].s[i].c[j].code, mxmlElementGetAttr( chan, "code") );

			/*** channel location code ***/
				strcpy( sl->n[k].s[i].c[j].locationCode, mxmlElementGetAttr( chan, "locationCode" ) );

			/*** channel start date ***/
                        	strcpy( sl->n[k].s[i].c[j].startDate_string, mxmlElementGetAttr( chan, "startDate") );
                        	sl->n[k].s[i].c[j].startDate = calloc(1,sizeof(MyTime));
				parsestring( sl->n[k].s[i].c[j].startDate, sl->n[k].s[i].c[j].startDate_string );

			/*** channel end date ***/
			/*** not all tags have endDates, therefore convert NULL dates far into the future ***/

				tmp_constant_string = mxmlElementGetAttr( chan, "endDate");

				sl->n[k].s[i].c[j].endDate = calloc(1,sizeof(MyTime));

                        	if( tmp_constant_string == NULL )
				{
					parsestring( sl->n[k].s[i].c[j].endDate, null_date_string );
				}
				else
				{
					strcpy( sl->n[k].s[i].c[j].endDate_string, mxmlElementGetAttr( chan, "endDate") );
					parsestring( sl->n[k].s[i].c[j].endDate, sl->n[k].s[i].c[j].endDate_string );
				}
	
			/*** debug start and end dates ***/
			/***
				snprintf( logit, sizeof(logit), "startDate=%s(%s) endDate=%s(%s) ot=%s", 
					MyTime2StandardString2(sl->n[k].s[i].c[j].startDate),
					sl->n[k].s[i].c[j].startDate_string,
					MyTime2StandardString2(sl->n[k].s[i].c[j].endDate),
					sl->n[k].s[i].c[j].endDate_string,
					MyTime2StandardString2(t) );
				writelog( logit, "readStationXML", verbose );
			***/

			/***************************************************************************************************************/
			/*** if date not between then skip? ***/
			/*** return_value =  mytime_between( t0, t1, t ) if time t is beween t0 and t1 then return 1 else return 0 ***/
			/***************************************************************************************************************/

				if( mytime_between( sl->n[k].s[i].c[j].startDate, sl->n[k].s[i].c[j].endDate, t ) == 0 ) 
				{
				  snprintf( logit, sizeof(logit),
					"net=%d sta=%d chan=%d net=%s station=%s chan=%s(loc=%s) event ot=%s outside channel startDate=%s endDate=%s skip! %s",
						k,
						i,
						j,
						sl->n[k].code,
						sl->n[k].s[i].code,
						sl->n[k].s[i].c[j].code,
						sl->n[k].s[i].c[j].locationCode,
						MyTime2StandardString2(t),
						MyTime2StandardString2(sl->n[k].s[i].c[j].startDate),
						MyTime2StandardString2(sl->n[k].s[i].c[j].endDate),
						tmp_constant_string );

					writelog( logit, "readStationXML", verbose );

					continue;
				}
				else
				{
				  snprintf( logit, sizeof(logit),
                                        "net=%d sta=%d chan=%d net=%s station=%s chan=%s(loc=%s) event ot=%s within channel startDate=%s endDate=%s keep! %s",
                                                k,
                                                i,
                                                j,
                                                sl->n[k].code,
                                                sl->n[k].s[i].code,
                                                sl->n[k].s[i].c[j].code,
                                                sl->n[k].s[i].c[j].locationCode,
                                                MyTime2StandardString2(t),
                                                MyTime2StandardString2(sl->n[k].s[i].c[j].startDate),
                                                MyTime2StandardString2(sl->n[k].s[i].c[j].endDate),
						tmp_constant_string );

                                        writelog( logit, "readStationXML", verbose );
				}
			/*** skip channel Latitude Longitude Elevation - see Station level ***/

			/*** channel emplacement depth ***/
				node = mxmlFindElement( chan, chan, "Depth", NULL, NULL, MXML_DESCEND );
				sl->n[k].s[i].c[j].depth = getFloatNode( node );

			/*** channel orientation - Azimuth and Dip ***/
				node = mxmlFindElement( chan, chan, "Azimuth", NULL, NULL, MXML_DESCEND );
				sl->n[k].s[i].c[j].azimuth = getFloatNode( node );

				node = mxmlFindElement( chan, chan, "Dip", NULL, NULL, MXML_DESCEND );
				sl->n[k].s[i].c[j].dip = getFloatNode( node );

			/*** skip Channel Type CONTINUOUS GEOPHYSICAL ***/

			/*** channel sample rate ***/
				node = mxmlFindElement( chan, chan, "SampleRate", NULL, NULL, MXML_DESCEND );
				sl->n[k].s[i].c[j].samplerate = getFloatNode( node );

			/*** skip ClockDrift = 0 ***/

			/*** channel sensor type ***/
				sensor = mxmlFindElement( chan, chan, "Sensor", NULL, NULL, MXML_DESCEND );
				
			/*** channel description ***/
				node = mxmlFindElement( sensor, sensor, "Description", NULL, NULL, MXML_DESCEND );
				getStringNode( node, sl->n[k].s[i].c[j].sensor_description );

/*****************************************************************************************************************************************/
			/*****/
			/*** the rest of xml file is optional and not needed in AutoMT because ****/
			/***  another query is performed to get TEXT SACPAZ using IRIS webservice ***/
			/*** NOTE: Should check every node for NULL since there is no guarentee any of this exists ***/
			/*****/
/*****************************************************************************************************************************************/

				resp = mxmlFindElement( chan, chan, "Response", NULL, NULL, MXML_DESCEND );

				if( resp != NULL )
				{

			/*** InstrumentSensitivity ***/
					node1 = mxmlFindElement( resp, resp, "InstrumentSensitivity", NULL, NULL, MXML_DESCEND );
					node  = mxmlFindElement( node1, node1, "Value", NULL, NULL, MXML_DESCEND );
					sl->n[k].s[i].c[j].InstrumentSensitivity = getFloatNode( node );
					sl->n[k].s[i].c[j].r.InstrumentSensitivity = getDoubleNode( node );

					 snprintf( logit, sizeof(logit), "Response.InstrumentSensitivity.Value = %lf(%e)",
						sl->n[k].s[i].c[j].r.InstrumentSensitivity,
						sl->n[k].s[i].c[j].r.InstrumentSensitivity );
					 writelog( logit, "readStationXML", verbose );

				/*** InstrumentSensitivity_Frequency ***/
					node = mxmlFindElement( node1, node1, "Frequency", NULL, NULL, MXML_DESCEND );
					sl->n[k].s[i].c[j].frequency = getFloatNode( node );
					sl->n[k].s[i].c[j].r.InstrumentSensitivity_Frequency = getFloatNode( node );
					
					 snprintf( logit, sizeof(logit), "Response.InstrumentSensitivity.Frequency = %g",
						sl->n[k].s[i].c[j].r.InstrumentSensitivity_Frequency );
					 writelog( logit, "readStationXML", verbose );

				/*** InstrumentSensitivity InputUnits ***/
					node2 = mxmlFindElement( node1, node1, "InputUnits", NULL, NULL, MXML_DESCEND );
					node  = mxmlFindElement( node2, node2, "Name", NULL, NULL, MXML_DESCEND );
					getStringNode( node, sl->n[k].s[i].c[j].inputUnits );
					getStringNode( node, sl->n[k].s[i].c[j].r.InputUnits );

					 snprintf( logit, sizeof(logit), "Response.InstrumentSensitivity.InputUnits.Name = %s",
						sl->n[k].s[i].c[j].r.InputUnits );
					 writelog( logit, "readStationXML", verbose );

				/*** InstrumentSensitivity OutputUnits ***/
					node2 = mxmlFindElement( node1, node1, "OutputUnits", NULL, NULL, MXML_DESCEND );
                                	node  = mxmlFindElement( node2, node2, "Name", NULL, NULL, MXML_DESCEND );
					getStringNode( node, sl->n[k].s[i].c[j].outputUnits );
					getStringNode( node, sl->n[k].s[i].c[j].r.OutputUnits );


					 snprintf( logit, sizeof(logit), "Response.InstrumentSensitivity.OutputUnits = %s",
						sl->n[k].s[i].c[j].r.OutputUnits );
					 writelog( logit, "readStationXML", verbose );

			/*** done with InstrumentSensitivity ****/

			/*** Response Stage number="1" ***/
		
					/*** I should add some sort of null value for stage number string ***/
					/** strcpy( sl->n[k].s[i].c[j].r.Stage_Number, "-9" ); ***/

					node1 = mxmlFindElement( resp, resp, "Stage", NULL, NULL, MXML_DESCEND );
					if( node1 != NULL )
					{
                                	 strcpy( sl->n[k].s[i].c[j].r.Stage_Number, mxmlElementGetAttr( node1, "number") );
					 snprintf( logit, sizeof(logit), "Response.Stage.number = %s", sl->n[k].s[i].c[j].r.Stage_Number );
					 writelog( logit, "readStationXML", verbose );
					}

			/*** Stage Number = 1 is the paz ***/

					if( strcmp( sl->n[k].s[i].c[j].r.Stage_Number, "1" ) == 0 )
					{
						node2 = mxmlFindElement( node1, node1, "PolesZeros", NULL, NULL, MXML_DESCEND );

						if( node2 != NULL )
						{

					/*** Response . Stage . PolesZeros . InputUnits . name ***/
						node3 = mxmlFindElement( node2, node2, "InputUnits", NULL, NULL, MXML_DESCEND );
						node  = mxmlFindElement( node3, node3, "Name", NULL, NULL, MXML_DESCEND );
						getStringNode( node, sl->n[k].s[i].c[j].r.Stage_One_InputUnits );

						 snprintf( logit, sizeof(logit), "Response.Stage.PolesZeros.InputUnits.Name = %s",
							sl->n[k].s[i].c[j].r.Stage_One_InputUnits );
						 writelog( logit, "readStationXML", verbose );

					/*** Response . Stage . PolesZeros . OutputUnits . name ***/
						node3 = mxmlFindElement( node2, node2, "OutputUnits", NULL, NULL, MXML_DESCEND );
						node  = mxmlFindElement( node3, node3, "Name", NULL, NULL, MXML_DESCEND );
						getStringNode( node, sl->n[k].s[i].c[j].r.Stage_One_OutputUnits );
			
						 snprintf( logit, sizeof(logit), "Response.Stage.PolesZeros.OutputUnits.Name = %s",
                                                        sl->n[k].s[i].c[j].r.Stage_One_OutputUnits );
                                                 writelog( logit, "readStationXML", verbose );

					/*** Response . Stage . PolesZeros . PzTransferFunctionType ***/
						node  = mxmlFindElement( node2, node2, "PzTransferFunctionType", NULL, NULL, MXML_DESCEND );
                                                getStringNode( node, sl->n[k].s[i].c[j].r.PzTransferFunctionType );

						 snprintf( logit, sizeof(logit), "Response.Stage.PolesZeros.PzTransferFunctionType = %s",
                                                        sl->n[k].s[i].c[j].r.PzTransferFunctionType );
                                                 writelog( logit, "readStationXML", verbose );

					/*** Response . Stage . PolesZeros . NormalizationFactor ***/
						node  = mxmlFindElement( node2, node2, "NormalizationFactor", NULL, NULL, MXML_DESCEND );
						sl->n[k].s[i].c[j].r.NormalizationFactor = getDoubleNode( node );
			
						 snprintf( logit, sizeof(logit), "Response.Stage.PolesZeros.NormalizationFactor = %lf(%e)",
							sl->n[k].s[i].c[j].r.NormalizationFactor,
							sl->n[k].s[i].c[j].r.NormalizationFactor );
						 writelog( logit, "readStationXML", verbose );

					/*** Response . Stage . PolesZeros . NormalizationFrequency ***/	
						node  = mxmlFindElement( node2, node2, "NormalizationFrequency", NULL, NULL, MXML_DESCEND );
						sl->n[k].s[i].c[j].r.NormalizationFrequency = getFloatNode( node );

						 snprintf( logit, sizeof(logit), "Response.Stage.PolesZeros.NormalizationFrequency = %g",
							sl->n[k].s[i].c[j].r.NormalizationFrequency );
						 writelog( logit, "readStationXML", verbose );

					/*** Response . Stage . PolesZeros . Zero Real Imaginary ***/


				/**** so SeisComp3 Obspy does not enumerate zeros that are at the origin of the complex plane ***/
				/*** apply same logic to poles just in case ***/
				/*** assumes zeros and poles are less than 30 ***/

						ik = 0; /*** counts loop over 30 ***/
						ikik = 0; /*** counts not null ***/

						sl->n[k].s[i].c[j].r.Number_Zeros = 0;
						while( ik < 30 )
						{
							sprintf( value_int_string, "%d", ik );

						/*** debug ****/
						/***
							fprintf( stdout, "%s: %s: %s: NSLC=(%s.%s.%s.%s) ik=%d value_int_string=%s\n",
								progname, __FILE__, __func__, 
								sl->n[k].code,
                                                		sl->n[k].s[i].code,
								sl->n[k].s[i].c[j].locationCode,
                                                		sl->n[k].s[i].c[j].code,
								ik, value_int_string );
						***/
							node3 = mxmlFindElement( node2, node2, "Zero", "number", value_int_string, MXML_DESCEND );
						
						/**** in case, Zero number value_int_string is non sequential, loop through all 30 ****/
							/* if( node3 == NULL ) break; */

							if( node3 != NULL )
							{
								strcpy( sl->n[k].s[i].c[j].r.zeros[ikik].number_string, mxmlElementGetAttr( node3, "number") );

								sl->n[k].s[i].c[j].r.zeros[ikik].number = atoi( sl->n[k].s[i].c[j].r.zeros[ikik].number_string );
	
							/*** debug ****/
							/***
								fprintf( stdout, "%s: %s: %s: NSLC=(%s.%s.%s.%s) ik=%d ikik=%d Zero number=(%s)%d \n",
                                                                	progname, __FILE__, __func__,
                                                                	sl->n[k].code,
                                                                	sl->n[k].s[i].code,
                                                                	sl->n[k].s[i].c[j].locationCode,
                                                                	sl->n[k].s[i].c[j].code,
                                                                	ik, 
									ikik, 
									sl->n[k].s[i].c[j].r.zeros[ikik].number_string,
									sl->n[k].s[i].c[j].r.zeros[ikik].number );
							***/

								node = mxmlFindElement( node3, node3, "Real", NULL, NULL, MXML_DESCEND );
								sl->n[k].s[i].c[j].r.zeros[ikik].Real = getDoubleNode( node );

								node = mxmlFindElement( node3, node3, "Imaginary", NULL, NULL, MXML_DESCEND );
								sl->n[k].s[i].c[j].r.zeros[ikik].Imaginary = getDoubleNode( node );
						
							 	snprintf( logit, sizeof(logit), "ikik=%d Zero %s %d Real=%lf Imaginary=%lf",
									ikik, 
									sl->n[k].s[i].c[j].r.zeros[ikik].number_string,
									sl->n[k].s[i].c[j].r.zeros[ikik].number,
									sl->n[k].s[i].c[j].r.zeros[ikik].Real,
									sl->n[k].s[i].c[j].r.zeros[ikik].Imaginary );
							 	writelog( logit, "readStationXML", verbose );

								ikik++; /*** increment count for non nulls ****/
							}
						
							ik++; /*** increment count for next node to test ***/
						}
						sl->n[k].s[i].c[j].r.Number_Zeros = ikik;
						
						 snprintf( logit, sizeof(logit), "r.Number_Zeros = %d", sl->n[k].s[i].c[j].r.Number_Zeros );
						 writelog( logit, "readStationXML", verbose );
					
					/*** Response . Stage . PolesZeros . Pole Real Imaginary  ***/

						ik = 0; /*** counts loop over 30 ***/
                                                ikik = 0; /*** counts not null ***/

                                                sl->n[k].s[i].c[j].r.Number_Poles = 0;

						while( ik < 30 )
						{
							sprintf( value_int_string, "%d", ik );

							node3 = mxmlFindElement( node2, node2, "Pole", "number", value_int_string, MXML_DESCEND );
							/* if( node3 == NULL ) break; */
							if( node3 != NULL )
							{
								strcpy( sl->n[k].s[i].c[j].r.poles[ikik].number_string, mxmlElementGetAttr( node3, "number") );

								sl->n[k].s[i].c[j].r.poles[ikik].number = atoi( sl->n[k].s[i].c[j].r.poles[ikik].number_string );
							
								node = mxmlFindElement( node3, node3, "Real", NULL, NULL, MXML_DESCEND );
								sl->n[k].s[i].c[j].r.poles[ikik].Real = getDoubleNode( node );
			
								node = mxmlFindElement( node3, node3, "Imaginary", NULL, NULL, MXML_DESCEND );
								sl->n[k].s[i].c[j].r.poles[ikik].Imaginary = getDoubleNode( node );
							
								snprintf( logit, sizeof(logit), "ikik=%d Poles %s %d Real=%lf Imaginary=%lf",
                                                                	ikik, 
                                                                	sl->n[k].s[i].c[j].r.poles[ikik].number_string,
                                                                	sl->n[k].s[i].c[j].r.poles[ikik].number,
                                                                	sl->n[k].s[i].c[j].r.poles[ikik].Real,
                                                                	sl->n[k].s[i].c[j].r.poles[ikik].Imaginary );
                                                         	writelog( logit, "readStationXML", verbose );

								ikik++; /*** increment count for non nulls ****/
							}
							ik++; /*** increment count for next node to test ***/
						}
						sl->n[k].s[i].c[j].r.Number_Poles = ikik;

						 snprintf( logit, sizeof(logit), "r.Number_Poles = %d", sl->n[k].s[i].c[j].r.Number_Poles );
                                                 writelog( logit, "readStationXML", verbose );

						}  /*** end Response . State . PolesZeros ***/

					/*** Response . State . Decimation -- skip ***/
							
					/*** Response . Stage . StageGain . Value ***/
						node2 = mxmlFindElement( node1, node1, "StageGain", NULL, NULL, MXML_DESCEND );
						node  = mxmlFindElement( node2, node2, "Value", NULL, NULL, MXML_DESCEND );
						sl->n[k].s[i].c[j].r.StageGain = getDoubleNode( node );

						 snprintf( logit, sizeof(logit), "Response.Stage.StageGain.Value = %lf",
							sl->n[k].s[i].c[j].r.StageGain );
						 writelog( logit, "readStationXML", verbose );

					/*** Response . Stage . PolesZeros . StageGain . Frequency ****/
						node  = mxmlFindElement( node2, node2, "Frequency", NULL, NULL, MXML_DESCEND );
						sl->n[k].s[i].c[j].r.StageGain_Frequency = getDoubleNode( node );

						 snprintf( logit, sizeof(logit), "Response.Stage.StageGain.Frequency = %lf",
							sl->n[k].s[i].c[j].r.StageGain_Frequency );
						 writelog( logit, "readStationXML", verbose );

				/*** NOTE: Should check every node for NULL since there is no guarentee any of this exists ***/

					} /*** if response stage = 1 exists  ***/
					
				} /*** if response exists ***/

	/***********************************************************************************************************************************************/
	/**** done with reading StationXML net sta chan resp nodes ****/
	/***********************************************************************************************************************************************/

			/*** check to ensure station has 3-components/channels BH? or HH? broadband highgain 3-C ***/
	
				if( sl->n[k].s[i].c[j].code[0] == 'B' || sl->n[k].s[i].c[j].code[0] == 'H' )			
				{
					if( sl->n[k].s[i].c[j].code[2] == 'Z' )
					{
						sl->n[k].s[i].has_vertical = 1;
					}
					else if( sl->n[k].s[i].c[j].code[2] == 'N' || sl->n[k].s[i].c[j].code[2] == '1' )
					{
						sl->n[k].s[i].has_northsouth = 1;
					}
					else if	( sl->n[k].s[i].c[j].code[2] == 'E' || sl->n[k].s[i].c[j].code[2] == '2' )
					{
						sl->n[k].s[i].has_eastwest   = 1;
					}
					else
					{
						snprintf( logit, sizeof(logit), "unknown component code = %s net = %s station = %s",
							sl->n[k].s[i].c[j].code, 
							sl->n[k].code,
							sl->n[k].s[i].code );
                        			writelog( logit, "readStationXML", verbose );
					}
				}
				else
				{
					snprintf( logit, sizeof(logit), "unknown channel code = %s net = %s station = %s",
						sl->n[k].s[i].c[j].code,
						sl->n[k].code,
						sl->n[k].s[i].code );
					writelog( logit, "readStationXML", verbose );
				}

			/***
				if( strcmp( sl->n[k].s[i].c[j].code, "BHZ" ) == 0 ) sl->n[k].s[i].has_vertical = 1;
				if( strcmp( sl->n[k].s[i].c[j].code, "BHN" ) == 0 ) sl->n[k].s[i].has_northsouth = 1;
				if( strcmp( sl->n[k].s[i].c[j].code, "BHE" ) == 0 ) sl->n[k].s[i].has_eastwest   = 1;
				if( strcmp( sl->n[k].s[i].c[j].code, "BH1" ) == 0 ) sl->n[k].s[i].has_northsouth = 1;
				if( strcmp( sl->n[k].s[i].c[j].code, "BH2" ) == 0 ) sl->n[k].s[i].has_eastwest   = 1;
	
				if( strcmp( sl->n[k].s[i].c[j].code, "HHZ" ) == 0 ) sl->n[k].s[i].has_vertical = 1;
                                if( strcmp( sl->n[k].s[i].c[j].code, "HHN" ) == 0 ) sl->n[k].s[i].has_northsouth = 1;
                                if( strcmp( sl->n[k].s[i].c[j].code, "HHE" ) == 0 ) sl->n[k].s[i].has_eastwest   = 1;
                                if( strcmp( sl->n[k].s[i].c[j].code, "HH1" ) == 0 ) sl->n[k].s[i].has_northsouth = 1;
                                if( strcmp( sl->n[k].s[i].c[j].code, "HH2" ) == 0 ) sl->n[k].s[i].has_eastwest   = 1;
			***/

				chancount++;
				j++;
				sl->n[k].s[i].nchannel++;

			} /*** loop over j-channels ***/

		/*** check to ensure station has 3-components/channels ***/

		/*** 3/2/2021 stop checking for 3-cmps. use as long as station has valid 1-channel  ***/

			sl->n[k].s[i].usedata = 1;

			if( sl->n[k].s[i].nchannel == 0 )
			{
				sl->n[k].s[i].usedata = 0;

				snprintf( logit, sizeof(logit), 
                        "%s: (%s.%s.%s) NOT 3-compoment station nchan=%d **********************",
                                        progname, 
                                        sl->n[k].code, 
                                        sl->n[k].s[i].code,
                                        sl->n[k].s[i].c[k].locationCode,
                                        sl->n[k].s[i].nchannel );
                                writelog( logit, "readStationXML", verbose );

			}

		/*
			if( sl->n[k].s[i].nchannel < 3 )
			{
				sl->n[k].s[i].usedata = 0;

				snprintf( logit, sizeof(logit), 
			"%s: (%s.%s.%s) NOT 3-compoment station nchan=%d **********************",
					progname, 
					sl->n[k].code, 
					sl->n[k].s[i].code,
					sl->n[k].s[i].c[k].locationCode,
					sl->n[k].s[i].nchannel );
				writelog( logit, "readStationXML", verbose );
			}
		*/
			snprintf( logit, sizeof(logit), "net=%d sta=%d done with net = %s station = %s ( usedata = %d : 0=no/1=yes )",
				k,
				i,
				sl->n[k].code, 
                                sl->n[k].s[i].code, 
				sl->n[k].s[i].usedata  );

			writelog( logit, "readStationXML", verbose );

			stacount++;
			i++;
			sl->n[k].nstation++;

		} /*** loop over i-stations ***/

		netcount++;
		k++;
		sl->numNets++;

	} /*** loop over k-networks ***/

	snprintf( logit, sizeof(logit),
		"total networks %d, total Stations %d, total chan=%d",
                netcount, 
		stacount,
		chancount );

	writelog(logit, "readStationXML", verbose );

        mxmlDelete(tree);

	return 1;

} /*** readStationXML ***/



int readEventsXML( char *filename, EventList *e, int verbose )
{
	FILE *fp;
	int i, j, k;
	int origincount, eventcount;

	float m2km = 0.001; /*** 1000 m per km ***/

	mxml_node_t *tree, *node;
	mxml_node_t *event;
	        mxml_node_t *desc;
                mxml_node_t *origin;
                        mxml_node_t *time, *lat, *lon, *dep, *qual, *eval;
                mxml_node_t *magnitude;
                        mxml_node_t *mag, *magtype, *stacount, *agency;
                mxml_node_t *preferredOriginID;
                mxml_node_t *type;
                mxml_node_t *creationInfo;

/*** write to log file ***/
        char logit[1024];
        void writelog( char *message, char *routine, int verbose );

/*** functions ***/

        MyTime *getTimeNode(  mxml_node_t *node_val, char *timestring );
        char *getStringNode(  mxml_node_t *node_val, char *out );
        float getFloatNode(   mxml_node_t *node_val );
        int   getIntegerNode( mxml_node_t *node_val );
        void print_node( mxml_node_t *node_tag, mxml_node_t *node_val, int type, float multscale );

/*** start ***/

        if( (fp = fopen(filename, "r")) == NULL )
        {
                snprintf( logit, sizeof(logit), "cannot open file %s", filename );
		writelog( logit, "readEventsXML", verbose );
        }
        tree = mxmlLoadFile( NULL, fp, MXML_OPAQUE_CALLBACK );
        fclose(fp);

/***************************/
/*** event node         ***/
/***************************/

	e->ev = calloc( 1, sizeof(Event) );
	for(    k = 0, eventcount = 0,
		event = mxmlFindElement( tree, tree, "event", NULL, NULL, MXML_DESCEND );
                event != NULL;
                event = mxmlFindElement( event, tree, "event", NULL, NULL, MXML_DESCEND ) )
        {
		e->ev = realloc( e->ev, (k+1)*sizeof(Event) );

		strcpy( e->ev[k].eventid, mxmlElementGetAttr( event, "catalog:eventid") );

		e->ev[k].evid = strtol( e->ev[k].eventid, NULL, 10 );

		strcpy( e->ev[k].publicID, mxmlElementGetAttr( event, "publicID") );

		node = mxmlFindElement( event, event, "preferredOriginID", NULL, NULL, MXML_DESCEND_FIRST );
		getStringNode( node, e->ev[k].preferredOriginID );

		desc = mxmlFindElement( event, event, "description", NULL, NULL, MXML_DESCEND_FIRST );
		node = mxmlFindElement( desc, desc, "type", NULL, NULL, MXML_DESCEND );
		node = mxmlFindElement( desc, desc, "text", NULL, NULL, MXML_DESCEND );
		getStringNode( node, e->ev[k].description );

		node = mxmlFindElement( event, event, "type", NULL, NULL, MXML_DESCEND_FIRST );
		getStringNode( node, e->ev[k].type );

		node = mxmlFindElement( event, event, "creationInfo", NULL, NULL, MXML_DESCEND_FIRST );
		agency = mxmlFindElement( node, node, "agencyID", NULL, NULL, MXML_DESCEND_FIRST );
		getStringNode( agency, e->ev[k].cinfo.agency );

		time = mxmlFindElement( node, node, "creationTime", NULL, NULL, MXML_DESCEND_FIRST );
		e->ev[k].cinfo.creationTime = calloc(1,sizeof(MyTime));
		e->ev[k].cinfo.creationTime = getTimeNode( time, e->ev[k].cinfo.creationTime_string );

	/***************************/
	/*** origin node         ***/
	/***************************/

		e->ev[k].o = calloc(1,sizeof(Origin));
		for(    j = 0, origincount = 0,
			origin = mxmlFindElement( event, event, "origin", NULL, NULL, MXML_DESCEND );
			origin != NULL;
			origin = mxmlFindElement( origin, event, "origin", NULL, NULL, MXML_DESCEND ) )
		{
			e->ev[k].o = realloc( e->ev[k].o, (j+1)*sizeof(Origin) );

			strcpy( e->ev[k].o[j].publicID, mxmlElementGetAttr( origin, "publicID") );

			time = mxmlFindElement( origin, origin, "time", NULL, NULL, MXML_DESCEND_FIRST );
			node = mxmlFindElement( time, time, "value", NULL, NULL, MXML_DESCEND );

			e->ev[k].o[j].origintime = calloc(1,sizeof(MyTime));
			e->ev[k].o[j].origintime = getTimeNode( node, e->ev[k].o[j].originTime_string );

			lon = mxmlFindElement( origin, origin, "longitude", NULL, NULL, MXML_DESCEND_FIRST );
			node = mxmlFindElement( lon, lon, "value", NULL, NULL, MXML_DESCEND );
			e->ev[k].o[j].longitude = getFloatNode( node );

			lat = mxmlFindElement( origin, origin, "latitude", NULL, NULL, MXML_DESCEND_FIRST );
                        node = mxmlFindElement( lat, lat, "value", NULL, NULL, MXML_DESCEND );
                        e->ev[k].o[j].latitude = getFloatNode( node );

                        dep = mxmlFindElement( origin, origin, "depth", NULL, NULL, MXML_DESCEND_FIRST );
                        node = mxmlFindElement( dep, dep, "value", NULL, NULL, MXML_DESCEND );

			e->ev[k].o[j].depth = getFloatNode( node );

			if( e->ev[k].o[j].depth > 0 ) 
				e->ev[k].o[j].depth *= m2km;
			else
				e->ev[k].o[j].depth = 0;

			qual = mxmlFindElement( origin, origin, "quality", NULL, NULL, MXML_DESCEND_FIRST );

			node = mxmlFindElement( qual, qual, "associatedStationCount", NULL, NULL, MXML_DESCEND );
			e->ev[k].o[j].associatedStationCount = getIntegerNode( node );

			node = mxmlFindElement( qual, qual, "usedPhaseCount", NULL, NULL, MXML_DESCEND );
			e->ev[k].o[j].usedPhaseCount = getIntegerNode( node );

			node = mxmlFindElement( qual, qual, "standardError", NULL, NULL, MXML_DESCEND );
			e->ev[k].o[j].standardError = getFloatNode( node );

			node = mxmlFindElement( qual, qual, "azimuthalGap", NULL, NULL, MXML_DESCEND );
			e->ev[k].o[j].azimuthalGap = getFloatNode( node );

			node = mxmlFindElement( qual, qual, "minimumDistance", NULL, NULL, MXML_DESCEND );
			e->ev[k].o[j].minimumDistance = getFloatNode( node );
	
			node = mxmlFindElement( origin, origin, "evaluationMode", NULL, NULL, MXML_DESCEND_FIRST );
			getStringNode( node, e->ev[k].o[j].evaluationMode );

			agency = mxmlFindElement( origin, origin, "creationInfo", NULL, NULL, MXML_DESCEND_FIRST );
			
                        node = mxmlFindElement( agency, agency, "agencyID", NULL, NULL, MXML_DESCEND );
			getStringNode( node, e->ev[k].o[j].cinfo.agency );
	
                        node = mxmlFindElement( agency, agency, "creationTime", NULL, NULL, MXML_DESCEND );
			e->ev[k].o[j].cinfo.creationTime = calloc(1,sizeof(MyTime));
			e->ev[k].o[j].cinfo.creationTime = getTimeNode( node, e->ev[k].o[j].cinfo.creationTime_string );

                        node = mxmlFindElement( agency, agency, "author", NULL, NULL, MXML_DESCEND );
			getStringNode( node, e->ev[k].o[j].cinfo.author );
			
        /*****************/
        /*** magnitude ***/
        /*****************/
			e->ev[k].o[j].mag = calloc(1,sizeof(Magnitude));

			for(	i = 0, 
				magnitude = mxmlFindElement( event, event, "magnitude", NULL, NULL, MXML_DESCEND );
				magnitude != NULL;
				magnitude = mxmlFindElement( magnitude, event, "magnitude", NULL, NULL, MXML_DESCEND ) )
			{
				e->ev[k].o[j].mag = realloc( e->ev[k].o[j].mag, (i+1) * sizeof(Magnitude) );

				magtype = mxmlFindElement( magnitude, magnitude, "type", NULL, NULL, MXML_DESCEND_FIRST );
				getStringNode( magtype, e->ev[k].o[j].mag[i].magtype );

                        	mag = mxmlFindElement( magnitude, magnitude, "mag", NULL, NULL, MXML_DESCEND );
                        	node = mxmlFindElement( mag, mag, "value", NULL, NULL, MXML_DESCEND );
                        	e->ev[k].o[j].mag[i].magnitude = getFloatNode( node );

                        	node = mxmlFindElement( mag, mag, "uncertainty", NULL, NULL, MXML_DESCEND );
				e->ev[k].o[j].mag[i].uncertainty = getFloatNode( node );

                        	node = mxmlFindElement( magnitude, magnitude, "stationCount", NULL, NULL, MXML_DESCEND_FIRST );
                        	e->ev[k].o[j].mag[i].stationCount = getIntegerNode( node );

                        	node = mxmlFindElement( magnitude, magnitude, "evaluationMode", NULL, NULL, MXML_DESCEND_FIRST );
				getStringNode( node, e->ev[k].o[j].mag[i].evaluationMode );

                        	agency = mxmlFindElement( magnitude, magnitude, "creationInfo", NULL, NULL, MXML_DESCEND_FIRST );

                        	node = mxmlFindElement( agency, agency, "agencyID", NULL, NULL, MXML_DESCEND );
				getStringNode( node, e->ev[k].o[j].mag[i].cinfo.agency );

                        	node = mxmlFindElement( agency, agency, "creationTime", NULL, NULL, MXML_DESCEND );
				e->ev[k].o[j].mag[i].cinfo.creationTime = calloc(1,sizeof(MyTime));
				e->ev[k].o[j].mag[i].cinfo.creationTime = getTimeNode( node, e->ev[k].o[j].mag[i].cinfo.creationTime_string );

                        	node = mxmlFindElement( agency, agency, "author", NULL, NULL, MXML_DESCEND );
				getStringNode( node, e->ev[k].o[j].mag[i].cinfo.author );
	
                        	i++;

			} /*** loop over i-magnitudes ***/
			e->ev[k].o[j].nmags = i;
	
			j++;
                        origincount++;

                } /*** loop over j-origins ***/
                e->ev[k].norigins = origincount;

		k++;
		eventcount++;

	} /*** loop over k-events ***/

	e->nevents = eventcount;

	snprintf( logit, sizeof(logit), "counted %d events", eventcount );
	writelog( logit, "readEventsXML", verbose );
	
	mxmlDelete(tree);

	return 1;

} /* readEventsXML */
