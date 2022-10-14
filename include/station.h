
/*** Response ***/

typedef struct {
        double Real;
        double Imaginary; 
        int number;
        char number_string[8];
} Complex_PolesZeros;

typedef struct {

/*** InstrumentSensitivity ***/
        double InstrumentSensitivity; /*** Value ***/
        float InstrumentSensitivity_Frequency;  /*** Frequency ***/
        char InputUnits[64];   /*** Name m/s ***/
        char OutputUnits[64];  /*** Name counts ***/

/*** Stage number="1" ***/
        char Stage_Number[8];

        /*** PolesZeros ***/
        char Stage_One_InputUnits[64]; /*** name m/s ***/
        char Stage_One_OutputUnits[64]; /*** name V ***/

        char PzTransferFunctionType[64];
        double NormalizationFactor;
        float NormalizationFrequency;

        int Number_Poles;
        int Number_Zeros;
        Complex_PolesZeros poles[30];
        Complex_PolesZeros zeros[30];
        double PolesZeros_Constant; /*** InstrumentSensitivity * NormalizationFactor ***/

        double StageGain;
        float StageGain_Frequency;

} Response;

/*** channel ***/

typedef struct {
	char code[8];		/*** BHZ, BHN, BHE, BH2, BH1 ***/
	char locationCode[8];	/*** "", 00, 10, 60 ***/
	char startDate_string[48];
        char endDate_string[48];
	MyTime *startDate;
	MyTime *endDate;
	float depth;
	float azimuth;	
	float dip;
	float samplerate;
	char sensor_description[128];

/*** response ***/
	float InstrumentSensitivity;
	float frequency;
	char inputUnits[32];
	char outputUnits[32];

/*** custom ***/
	int usechan;
				/* year.jda.hr.mn.se.msec.net.sta.loc.chan.SAC */
				/* 12345678901234567890123456789012345678901234567890 */
				/*          1         2         3         4         5 */
	char sacfilename[64];  /* YYYY.JJJ.HH.MM.SS.MMMM.XX.YYYY.ZZ.WWW.SAC */

	Response r;

} Channel;

typedef struct {
	char code[16];		/*** MDJ ***/
	char startDate_string[48];
        char endDate_string[48];
	MyTime *startDate;
	MyTime *endDate;
	float latitude;
	float longitude;
	float elevation;
	char name[128];
	int usedata;
	int has_vertical, has_northsouth, has_eastwest;

	Channel *c;
	int nchannel;
} Station;

typedef struct {
	char code[8];		/*** IC ***/
	char startDate_string[48];
	char endDate_string[48];
	MyTime *startDate;
	MyTime *endDate;
	char Description[512];

	Station *s;
	int nstation;
} Network;

typedef struct {
	Network *n;
	int numNets;
} StationList;

/******************************************/
/* StationList sl                         */
/* BHZ =  sl.n[i].s[j].c[k].code          */
/* 00  =  sl.n[i].s[j].c[k].locationCode  */
/* MDJ =  sl.n[i].s[j].code               */ 
/* IC  =  sl.n[i].code                    */
/*                                        */
/* IC.MDJ.00.BHZ                          */
/******************************************/

