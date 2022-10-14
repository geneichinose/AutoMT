
typedef struct {
        char agency[128];
        char author[128];
        char creationTime_string[48];
        MyTime *creationTime;
} CreationInfo;

typedef struct {
	float magnitude;
	char magtype[12];
	float uncertainty;
	int stationCount;
	char evaluationMode[128];
	CreationInfo cinfo;
} Magnitude;

typedef struct 
{
	char publicID[512];
	char preferredOriginID[512];
	char originTime_string[48];
	MyTime *origintime;

	float longitude;
	float latitude;
	float depth;

	int associatedStationCount;
	int usedPhaseCount;
	float standardError;
	float azimuthalGap;
	float minimumDistance;
	char evaluationMode[128];

	Magnitude *mag;
	int nmags;

	CreationInfo cinfo;

	int grn;
	char grname[128];
} Origin;

typedef struct {  
	long evid;  /*** convert eventid string to number in xmlsubs.c ****/
	char publicID[512];
	char preferredOriginID[512];
	char description[512];
	char type[128]; /*event*/
	char eventid[32];
	CreationInfo cinfo;
	Origin *o;
	int norigins;
} Event;

typedef struct {
        Event *ev;
        int nevents;
        MyTime updatedTime;  /* time webservice query */
        char author[256]; /* ISC, USGS, IRIS */
        char status[256]; /* current, new, old */
        CreationInfo cinfo;
} EventList;

