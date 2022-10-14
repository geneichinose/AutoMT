#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include "/Users/ichinose1/Work/mxml-2.10/mxml.h"

#define STRING_NODE 0
#define FLOAT_NODE  1
#define INTEGER_NODE 2

int main(int ac, char **av)
{
	int i, j, k;
	int netcount, stacount, chancount;
	FILE *fp;
	mxml_node_t *tree, *node, *node1;
	mxml_node_t *net, *sta, *chan, *sensor, *resp;

	void print_node( mxml_node_t *node_tag, mxml_node_t *node_val, int type, float multscale );

/*** start ***/

	if( (fp = fopen(av[1], "r")) == NULL )
	{
		fprintf( stderr, "cannot open file %s\n", av[1] );
		exit(-1);
	}
	tree = mxmlLoadFile( NULL, fp, MXML_OPAQUE_CALLBACK );
	fclose(fp);

/***************************/
/*** Network nodes       ***/
/***************************/
	netcount = 0;
	stacount = 0;
	chancount = 0;

	k = 0;
	for(	net = mxmlFindElement( tree, tree, "Network", NULL, NULL, MXML_DESCEND );
		net != NULL;
		net = mxmlFindElement( net, tree, "Network", NULL, NULL, MXML_DESCEND ) )
	{
		fprintf( stdout, "k=%d %s=(%s) start=(%s) ", 
			k,
			mxmlGetElement( net ), 
			mxmlElementGetAttr( net, "code"),
			mxmlElementGetAttr( net, "startDate") );
			/* mxmlElementGetAttr( sta, "endDate") ); */

		node = mxmlFindElement( net, net, "Description", NULL, NULL, MXML_DESCEND );
		fprintf( stdout, "%s=(%s)\n", 
			mxmlGetElement( node ),
			mxmlGetOpaque( node ) );	
	
	/***************************/
	/*** Station nodes       ***/
	/***************************/
		
		i = 0;
		for(	sta = mxmlFindElement( net, net, "Station", NULL, NULL, MXML_DESCEND ); 
			sta != NULL; 
			sta = mxmlFindElement( sta, net, "Station", NULL, NULL, MXML_DESCEND ) )
		{
			fprintf( stdout, "i=%d %s=(%s) start=(%s) ",
				i,
                		mxmlGetElement( sta ),
				mxmlElementGetAttr( sta, "code"), 
				mxmlElementGetAttr( sta, "startDate") );
				/* mxmlElementGetAttr( sta, "endDate") ); */

			node = mxmlFindElement( sta, sta, "Latitude", NULL, NULL, MXML_DESCEND );
			print_node( node, node, FLOAT_NODE, 1 );

			node = mxmlFindElement( sta, sta, "Longitude", NULL, NULL, MXML_DESCEND );
			print_node( node, node, FLOAT_NODE, 1 );

			node = mxmlFindElement( sta, sta, "Elevation", NULL, NULL, MXML_DESCEND );
			print_node( node, node, FLOAT_NODE, 1 );
		/*
			node = mxmlFindElement( sta, sta, "Site", NULL, NULL, MXML_DESCEND );
                	fprintf( stdout, "%s ", mxmlGetElement( node ) );
		*/
			node = mxmlFindElement( sta, sta, "Name", NULL, NULL, MXML_DESCEND );
			print_node( node, node, STRING_NODE, 0 );
                
			fprintf( stdout, "\n" );

		/***************************/	
		/*** Channel Information ***/
		/***************************/

			j = 0;
			for(	chan = mxmlFindElement( sta, sta, "Channel", NULL, NULL, MXML_DESCEND );
				chan != NULL;
				chan = mxmlFindElement( chan, sta, "Channel", NULL, NULL, MXML_DESCEND ) )
			{

				fprintf( stdout, "\tj=%d %s=(%s) loc=(%s) start=(%s) ",
					j,
					mxmlGetElement( chan ),
                                	mxmlElementGetAttr( chan, "code" ),
					mxmlElementGetAttr( chan, "locationCode" ),
					mxmlElementGetAttr( chan, "startDate") );

				node = mxmlFindElement( chan, chan, "Depth", NULL, NULL, MXML_DESCEND );
				print_node( node, node, FLOAT_NODE, 1 );

				node = mxmlFindElement( chan, chan, "Azimuth", NULL, NULL, MXML_DESCEND );
				print_node( node, node, FLOAT_NODE, 1 );

				node = mxmlFindElement( chan, chan, "Dip", NULL, NULL, MXML_DESCEND );
				print_node( node, node, FLOAT_NODE, 1 );

				node = mxmlFindElement( chan, chan, "SampleRate", NULL, NULL, MXML_DESCEND );
				print_node( node, node, FLOAT_NODE, 1 );

				sensor = mxmlFindElement( chan, chan, "Sensor", NULL, NULL, MXML_DESCEND );
				fprintf( stdout, "%s ", mxmlGetElement( sensor ) ); 

				node = mxmlFindElement( sensor, sensor, "Description", NULL, NULL, MXML_DESCEND );
				print_node( node, node, STRING_NODE, 0 );

				resp = mxmlFindElement( chan, chan, "Response", NULL, NULL, MXML_DESCEND );
                                fprintf( stdout, "%s ", mxmlGetElement( resp ) );

				node = mxmlFindElement( resp, resp, "InstrumentSensitivity", NULL, NULL, MXML_DESCEND );
                                fprintf( stdout, "%s ", mxmlGetElement( node ) );

				node = mxmlFindElement( resp, resp, "Value", NULL, NULL, MXML_DESCEND );
				print_node( node, node, FLOAT_NODE, 1 );

				node = mxmlFindElement( resp, resp, "Frequency", NULL, NULL, MXML_DESCEND );
				print_node( node, node, FLOAT_NODE, 1 );

				node1 = mxmlFindElement( resp, resp, "InputUnits", NULL, NULL, MXML_DESCEND );
				node = mxmlFindElement( node1, node1, "Name", NULL, NULL, MXML_DESCEND );
				print_node( node, node, STRING_NODE, 0 );

				node1 = mxmlFindElement( resp, resp, "OutputUnits", NULL, NULL, MXML_DESCEND );
				node = mxmlFindElement( node1, node1, "Name", NULL, NULL, MXML_DESCEND );
				print_node( node, node, STRING_NODE, 0 );

				fprintf(stdout, "\n");
				chancount++;
				j++;

			} /*** loop over j-channels ***/

			stacount++;
			i++;

		} /*** loop over i-stations ***/

		fprintf(stdout, "\n");

		k++;
		netcount++;

	} /*** loop over networks ***/

	fprintf( stdout, "total networks %d, total Stations %d, total chan=%d\n", 
		netcount, stacount, chancount );

	mxmlDelete(tree);
}
