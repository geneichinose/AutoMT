
typedef struct {

/** event **/
        char webserve_agency_name[12];
        char webserve_url[256];
        char start_string[48], end_string[48];
        float duration_days;
        MyTime st, et;
        float minlat, minlon;
	float maxlat, maxlon;
        float minmag, maxmag;
        int limit_return;
        char xml_filename[256];

/** sta,chan **/
        char metadataWebservice_url[256];
        char waveformWebservice_url[256];
        char responseWebservice_url[256];

        float maxDistMTdeg;
        float minimumWaveformDurationSec; /* 1800 sec */
        float lowestPhaseVelocity;

	float preCutTime;
	float postCutTime;
	float WaveFormDuration; /* 1800 sec */
} Search_Parameters;

