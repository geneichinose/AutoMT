
typedef struct {

/*** time search ***/
        char start_string[48];
	char   end_string[48];
        MyTime st, et;
	float preCutTimeSec;
	float postCutTimeSec;
	float WaveFormDurationSec;

/** area search net.sta,loc,chan **/
        char metadataWebservice_url[256];
        char waveformWebservice_url[256];
        char responseWebservice_url[256];
	char curl_options[128];

	char search_type[16]; /*** single, box, circle ****/
	int single_station;
	int box_area;
	int circle_area;
	float box_minlat, box_minlon;
	float box_maxlat, box_maxlon;
	float radiuskm, clat, clon, radiusdeg;

	char sta[128]; /*** can use comma delimited strings ***/
	char net[64];  /*** can use comma delimited strings ***/
	char chan[64]; /*** can use comma delimited strings ***/
	char loc[32]; /*** can use comma delimited strings ***/

	int isetupMT;
	float evla, evlo, evdp;
	MyTime origintime;
	char origintime_string[48];
	char velocity_model[48];
	char setupMT_executable[128]; /*** /Users/ichinose1/Work/mtinv.v4.0.0/bin/setupMT ****/
	char sacmerge_executable[128]; /*** /Users/ichinose1/Work/mtinv.v4.0.0/bin/sacmerge ***/

	char xml_filename[256];
	char move_executable[128];  /*** /bin/mv **/
	char unzip_executable[128]; /*** /usr/bin/zip **/
	char sac2gmtmap_executable[128]; /*** /Users/ichinose1/bin/sac2gmtmap ***/

} Search_Window_Parameters;

