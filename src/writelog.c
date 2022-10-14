#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "mytime.h"

char progname[256];
char working_directory[256];

#define FMTSTRING "%s| %s| %s-%02d %02d:%02d:%02.0f| %s\n"

void writelog( char *message, char *routine, int verbose )
{
	FILE *fp;
	char logfilename[512];

	extern char progname[256];
	extern char working_directory[256];

	MyTime *now;
	void initialize_mytime( MyTime *t );
	MyTime *mylocaltime( MyTime *t );
	void WriteMyTime2STDOUT( MyTime *t );

	now = calloc(1,sizeof(MyTime));
	initialize_mytime(now);
	now = mylocaltime(now);

	sprintf( logfilename, "%s/%s.%4d%02d%02d.log", 
		working_directory,
		progname, 
		now->year,
		now->month,
		now->mday );

	if( (fp = fopen( logfilename, "a" )) == NULL )
	{
		fprintf( stderr, "%s: cannot open file %s\n",
			progname, logfilename );
		exit(-1);
	}

	fprintf( fp, FMTSTRING,
		progname,
                routine,
                now->cmonth,
                now->mday,
                now->hour,
                now->min,
                now->fsec,
                message );
	fflush(fp);

 	if( verbose )
	{
		fprintf( stderr, FMTSTRING,
			progname,
                	routine,
                	now->cmonth,
                	now->mday,
                	now->hour,
                	now->min,
                	now->fsec,
	               message );
		fflush(stderr);
	}

	free(now);
	fclose(fp);
}
