/*** QuakeML XML type schema header file ***/

/* requires mytime.h */
/* #include "mytime.h" */

typedef struct {
	char text[256];
	char type[256];
} EventDescription;

typedef struct {
	float value;
	float uncertainty;
	float lowerUncertainty;
	float upperUncertainty;
	float confidenceLevel;
}Quantity_Float;

typedef struct {
	int value;
	int uncertainty;
	int lowerUncertainty;
	int upperUncertainty;
	float confidenceLevel;
} Quantity_Int;

typedef struct {
	char agencyID[128];
	char author[128];
	char version[128];
	char timestring[32];
	Mytime creationTime;
} creationInfo;

typedef struct {
	char text[1024];
	char resourceID[128];
	creationInfo cInfo;
} Comment;

typedef struct {
	char publicID[256];
	char preferredOriginID[256];
	char preferredMagnitudeID[256];

	char type[256];
	int itype;
	char typeCertainty[256];
	int itypeCertainty;

	Comment com;
	EventDescription eventDescription;
	creationInfo cInfo;
} Event;

static char *EventDescriptionType[] = {
	" ",			/* 0 */
	"felt report",		/* 1 */
	"Flinn-Engdahl region"	/* 2 */
	"local time",		/* 3 */
	"tectonic summary",	/* 4 */
	"nearest cities",	/* 5 */
	"earthquake name",	/* 6 */
	"region name" };	/* 7 */
	
static char *EventType[] = {
	" ",			/* 0 */
	"not existing",		/* 1 */
	"not reported",		/* 2 */
	"earthquake",		/* 3 */
	"anthropogenic event",	/* 4 */
	"collapse",		/* 5 */
	"cavity collapse",	/* 6 */
	"mine collapse",	/* 7 */
	"building collapse",	/* 8 */
	"explosion",		/* 9 */
	"accidental explosion",	/* 10 */
	"chemical explosion",	/* 11 */
	"controlled explosion",	/* 12 */
	"experimental explosion",	/* 13 */
	"industrial explosion",	/* 14 */
	"mining explosion",	/* 15 */
	"quarry blast",		/* 16 */
	"road cut",		/* 17 */
	"blasting levee",	/* 18 */
	"nuclear explosion",	/* 19 */
	"induced or triggered event", /* 20 */
	"rock burst",		/* 21 */
	"reservoir loading",	/* 22 */
	"fluid injection",	/* 23 */
	"fluid extraction",	/* 24 */
	"crash",		/* 25 */
	"plane crash",		/* 26 */
	"train crash",	/* 27 */
	"boat crash",	/* 28 */
	"other event",	/* 29 */
	"atmospheric event",	/* 30 */
	"sonic boom",	/* 31 */
	"sonic blast",	/* 32 */
	"acoustic noise",	/* 33 */
	"thunder",	/* 34 */
	"avalanche",	/* 35 */
	"snow avalanche",	/* 36 */
	"debris avalanche",	/* 37 */
	"hydroacoustic event",	/* 38 */
	"ice quake",		/* 39 */
	"slide",		/* 40 */
	"landslide",		/* 41 */
	"rockslide",		/* 42 */
	"meteorite", 		/* 43 */
	"volcanic eruption" };	/* 44 */

static char *EventTypeCertainty[] = {
	" ",
	"known",
	"suspected" };

static char *EvaluationMode[] = {
	" ",
	"manual", 
	"automatic" };

static char *EvaluationStatus[] = {
	" ",		/* 0 */
	"preliminary",	/* 1 */
	"confirmed",	/* 2 */
	"reviewed",	/* 3 */
	"rejected" };	/* 4 */

static char *OriginDepthType[] = {
	" ",			/* 0 */
	"from location",	/* 1 */
	"from moment tensor",  	/* 2 */
	"from modeling of broad-band P waveforms", /* 3 */
	"constrained by depth phases",	/* 4 */
	"constrained by direct phases", /* 5 */
	"constrained by depth and direct phases", /* 6 */
	"operator assigned",		/* 7 */
	"other" }			/* 8 */

static char *OriginType[] = {
	" ",		/* 0 */
	"hypocenter",	/* 1 */
	"centroid",	/* 2 */
	"amplitude",	/* 3 */
	"macroseismic", /* 4 */
	"rupture start", /* 5 */
	"rupture end" }; /* 6 */

typedef struct {
	int associatedPhaseCount;
	int usedPhaseCount;
	int associatedStationCount;
	int usedStationCount;
	int depthPhaseCount;
	float standardError;
	float azimuthGap;
	float secondaryAzimuthGap;
	char groundTruthLevel[128];
	float minimumDistance;
	float maximumDistance;
	float medianDistance;
} OriginQuality;

typedef struct {
	float semiMajorAxisLength;
	float semiMinorAxisLength;
	float semiIntermediateAxisLength;
	float majorAxisPlunge;
	float majorAxisAzimuth;
	float majorAxisRotation;
} ConfidenceEllipsoid;

typedef struct {
	char timestring[32];
	Mytime ot;

	Quantity_Float latitude;
	Quantity_Float longitude;
	Quantity_Float depth;

	char depthType[128]; /* OriginDepthType */
	int idepthType;

	int timeFixed;
	int epicenterFixed;
	char methodID[128];
	char earthModelID[128];
	OriginQuality quality;

	char type[128]; /* OriginType */
	int itype;

	char region[128];

	char evaluationMode[128];	/* EvaluationMode */
	int ievalMode;

	char evaluationStatus[128];	/* EvaluationStatus */
	int ievalStat;

	Comment com;
	creationInfo cinfo;
} Origin;

typedef struct {
	float horizontalUncertainty;
	float minHorizontalUncertainty;
	float maxHorizontalUncertainty;
	float azimuthMaxHorizontalUncertainty;
	ConfidenceEllipsoid confidenceEllipsoid;
	int preferredDescription;
	float confidenceLevel;
} OriginUncertainty;

typedef struct {
	Quantity_Flat mag;
	char type[32];
	char originID[128];
	char methodID[128];
	int stationCount;
	float azimuthalGap;
	char evaluationMode[128];
	int ievalMode;
	char evaluationStatus[128];
	int ievalStat;
	Comment com;
	CreationInfo cInfo;
} Magnitude;

