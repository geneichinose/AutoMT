#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include "/Users/ichinose1/Work/mxml-2.10/mxml.h"
#include "../include/mytime.h"

#define STRING_NODE 0
#define FLOAT_NODE  1
#define INTEGER_NODE 2

int main(int ac, char **av)
{
	int i, j, k;
	int eventcount;
	FILE *fp;

	float m2km = 0.001;

/*** test stuff ***/
	char description[128];
	float longitude, latitude, depth, magval;
	int npha;
	MyTime *ot;
	char timestring[32];

/*** xml stuff ***/

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

/*** functions ***/

	MyTime *getTimeNode(  mxml_node_t *node_val, char *timestring );

	char *getStringNode(  mxml_node_t *node_val, char *out );
	float getFloatNode(   mxml_node_t *node_val );
	int   getIntegerNode( mxml_node_t *node_val );
	void print_node( mxml_node_t *node_tag, mxml_node_t *node_val, int type, float multscale );

/*** start ***/ 

	ot = calloc(1,sizeof(MyTime));

	if( (fp = fopen(av[1], "r")) == NULL )
	{	
		fprintf( stderr, "cannot open file %s\n", av[1] );
		exit(-1);
	}
	tree = mxmlLoadFile( NULL, fp, MXML_OPAQUE_CALLBACK );
	fclose(fp);

	eventcount = 0;

/***************************/
/*** event node         ***/
/***************************/
	k = 0;
	for( 	event = mxmlFindElement( tree, tree, "event", NULL, NULL, MXML_DESCEND );
		event != NULL;
		event = mxmlFindElement( event, tree, "event", NULL, NULL, MXML_DESCEND ) )
	{

	/*** get some attributes in event tag ***/

		fprintf( stdout, "catalog:eventid=(%s)\n", mxmlElementGetAttr( event, "catalog:eventid") );

		fprintf( stdout, "publicID=(%s)\n", mxmlElementGetAttr( event, "publicID") );

		node = mxmlFindElement( event, event, "preferredOriginID", NULL, NULL, MXML_DESCEND_FIRST );
		print_node( node, node, STRING_NODE, 0 );

	/*** get the next description child ***/

		desc = mxmlFindElement( event, event, "description", NULL, NULL, MXML_DESCEND_FIRST );
		node = mxmlFindElement( desc, desc, "type", NULL, NULL, MXML_DESCEND );
		fprintf( stdout, "%s %s=(%s) ",
                        mxmlGetElement( desc ),
                        mxmlGetElement( node ), 
			mxmlGetOpaque( node ) );

		node = mxmlFindElement( desc, desc, "text", NULL, NULL, MXML_DESCEND );
		print_node( node, node, STRING_NODE, 0 );

		getStringNode( node, description );
		fprintf( stdout, "getStringNode: description.text = %s\n", description );

		node = mxmlFindElement( event, event, "type", NULL, NULL, MXML_DESCEND_FIRST );
		print_node( node, node, STRING_NODE, 0 );

		node = mxmlFindElement( event, event, "creationInfo", NULL, NULL, MXML_DESCEND_FIRST );
		agency = mxmlFindElement( node, node, "agencyID", NULL, NULL, MXML_DESCEND_FIRST );
		print_node( agency, agency, STRING_NODE, 0 );

		time = mxmlFindElement( node, node, "creationTime", NULL, NULL, MXML_DESCEND_FIRST );
		print_node( time, time, STRING_NODE, 0 );

		fprintf( stdout, "\n" );

/***************************/
/*** origin node         ***/
/***************************/

		j = 0;
		for( 	origin = mxmlFindElement( event, event, "origin", NULL, NULL, MXML_DESCEND );
			origin != NULL;
			origin = mxmlFindElement( origin, event, "origin", NULL, NULL, MXML_DESCEND ) )
		{
			fprintf( stdout, "k=%d j=%d ", k, j );

			fprintf( stdout, "publicID=(%s) ", mxmlElementGetAttr( origin, "publicID") );

		/*** origin - time, longitude, latitude, depth (value, uncertainty) , depthType ***/
			/*** timeFixed, epicenterFixed, methodID, earthModelID, quality, type, region, evaluationMode, 
				evaluationStatus, comment, creationInfo ***/

			time = mxmlFindElement( origin, origin, "time", NULL, NULL, MXML_DESCEND_FIRST );
			node = mxmlFindElement( time, time, "value", NULL, NULL, MXML_DESCEND );
			print_node( time, node, STRING_NODE, 0 );		
			
			ot = getTimeNode( node, timestring );
			fprintf( stdout, "timestring=%s ", timestring );
			WriteMyTime2STDOUT( ot );

			lon = mxmlFindElement( origin, origin, "longitude", NULL, NULL, MXML_DESCEND_FIRST );
                        node = mxmlFindElement( lon, lon, "value", NULL, NULL, MXML_DESCEND );
			print_node( lon, node, FLOAT_NODE, 1 );
 			
			longitude = getFloatNode( node );

			lat = mxmlFindElement( origin, origin, "latitude", NULL, NULL, MXML_DESCEND_FIRST );
                        node = mxmlFindElement( lat, lat, "value", NULL, NULL, MXML_DESCEND );
			print_node( lat, node, FLOAT_NODE, 1 );

			latitude = getFloatNode( node );

			dep = mxmlFindElement( origin, origin, "depth", NULL, NULL, MXML_DESCEND_FIRST );
			node = mxmlFindElement( dep, dep, "value", NULL, NULL, MXML_DESCEND );
			print_node( dep, node, FLOAT_NODE, m2km );

			depth = getFloatNode( node ) * m2km;

			node = mxmlFindElement( dep, dep, "uncertainty", NULL, NULL, MXML_DESCEND );
			print_node( node, node, FLOAT_NODE, m2km );

		/*** originUncertainty - horizontalUncertainty minHorizontalUncertainty maxHorizontalUncertainty 
				azimuthMaxHorizontalUncertainty confidenceEllipsoid confidenceLevel preferredDescription ***/
			/*** units in meters and degrees N, preferredDescription=horizontal uncertainty, uncertainty ellipse, 
				confidence ellipsoid ***/

		/*** quality - associatedStationCount, usedPhaseCount, standardError, azimuthalGap, minimumDistance ***/

			qual = mxmlFindElement( origin, origin, "quality", NULL, NULL, MXML_DESCEND_FIRST );

			node = mxmlFindElement( qual, qual, "associatedStationCount", NULL, NULL, MXML_DESCEND );
			print_node( node, node, INTEGER_NODE, 1 );
	
			node = mxmlFindElement( qual, qual, "usedPhaseCount", NULL, NULL, MXML_DESCEND );
			print_node( node, node, INTEGER_NODE, 1 );

			npha = getIntegerNode( node );

			fprintf( stdout, "getFloatNode: latitude = %g longitude = %g depth = %g npha = %d \n",
				latitude, longitude, depth, npha );

			node = mxmlFindElement( qual, qual, "standardError", NULL, NULL, MXML_DESCEND );
			print_node( node, node, FLOAT_NODE, 1 );

			node = mxmlFindElement( qual, qual, "azimuthalGap", NULL, NULL, MXML_DESCEND );
			print_node( node, node, FLOAT_NODE, 1 );

			node = mxmlFindElement( qual, qual, "minimumDistance", NULL, NULL, MXML_DESCEND );
			print_node( node, node, FLOAT_NODE, 1 );

			node = mxmlFindElement( origin, origin, "evaluationMode", NULL, NULL, MXML_DESCEND_FIRST );
			print_node( node, node, STRING_NODE, 0 );

			agency = mxmlFindElement( origin, origin, "creationInfo", NULL, NULL, MXML_DESCEND_FIRST );

			node = mxmlFindElement( agency, agency, "agencyID", NULL, NULL, MXML_DESCEND );
			print_node( node, node, STRING_NODE, 0 );

			node = mxmlFindElement( agency, agency, "creationTime", NULL, NULL, MXML_DESCEND );
			print_node( node, node, STRING_NODE, 0 );

			node = mxmlFindElement( agency, agency, "author", NULL, NULL, MXML_DESCEND );
			print_node( node, node, STRING_NODE, 0 );

			fprintf( stdout, "\n" );
			j++;
		} /*** loop over j - origins ***/
	
	/*****************/
	/*** magnitude ***/
	/*****************/

		i = 0;
		for(    magnitude = mxmlFindElement( event, event, "magnitude", NULL, NULL, MXML_DESCEND );
                        magnitude != NULL;
                        magnitude = mxmlFindElement( magnitude, event, "magnitude", NULL, NULL, MXML_DESCEND ) )
                {
                        fprintf( stdout, "k=%d i=%d ", k, i );
	
			magtype = mxmlFindElement( magnitude, magnitude, "type", NULL, NULL, MXML_DESCEND_FIRST );
			print_node( magtype, magtype, STRING_NODE, 0 );

			mag = mxmlFindElement( magnitude, magnitude, "mag", NULL, NULL, MXML_DESCEND );
                        node = mxmlFindElement( mag, mag, "value", NULL, NULL, MXML_DESCEND );
			print_node( mag, node, FLOAT_NODE, 1 );

			magval = getFloatNode( node );

			fprintf( stdout, " getFloatNode : magval = %g\n", magval );

			node = mxmlFindElement( mag, mag, "uncertainty", NULL, NULL, MXML_DESCEND );
			print_node( node, node, FLOAT_NODE, 1 );

			node = mxmlFindElement( magnitude, magnitude, "stationCount", NULL, NULL, MXML_DESCEND_FIRST );
			print_node( node, node, INTEGER_NODE, 1 );
	
			node = mxmlFindElement( magnitude, magnitude, "evaluationMode", NULL, NULL, MXML_DESCEND_FIRST );
			print_node( node, node, STRING_NODE, 0 );

			agency = mxmlFindElement( magnitude, magnitude, "creationInfo", NULL, NULL, MXML_DESCEND_FIRST );

                        node = mxmlFindElement( agency, agency, "agencyID", NULL, NULL, MXML_DESCEND );
			print_node( node, node, STRING_NODE, 0 );

                        node = mxmlFindElement( agency, agency, "creationTime", NULL, NULL, MXML_DESCEND );
			print_node( node, node, STRING_NODE, 0 );

			node = mxmlFindElement( agency, agency, "author", NULL, NULL, MXML_DESCEND );
			print_node( node, node, STRING_NODE, 0 );

                        fprintf( stdout, "\n" );
                        i++;

                } /*** loop over i - magnitudes ***/

		fprintf( stdout, "\n" );
		k++;
		eventcount++;

	} /*** loop over k - events ***/

	fprintf( stdout, "counted %d events\n", eventcount );

	mxmlDelete(tree);
}
