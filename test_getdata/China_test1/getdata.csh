#!/bin/csh
### 48.7584° N    125.2579° E     2020-02-07 00:12:40 UTC 10.0 km mb4.5   Northeastern China

/bin/rm -fr Data Resp getdata.????????.log channel.xml getdata.par

set wdir=(`pwd`)
echo $wdir

cat >! getdata.par << EOF
working_directory=${wdir}
#
curl_options="/usr/bin/curl -k -s --max-time 60 --connect-timeout 60 "
start_time=2020-02-07T00:00:00
duration_sec=86400
#
#end_time=2020-07-01T00:15:00
#pre_time_sec=900
#post_time_sec=1800
#
search_type=single_station net=IC sta=MDJ loc=00 chan=BHZ
#
#search_type=circle clat=40.1 clon=138.1 radiuskm=1000
#
#search_type=box minlat=32.0 minlon=-121.0  maxlat=42.0 maxlon=-119.0
#
getwave
getresp
verbose
map_gmt_stations
#
url_metadata="https://service.iris.edu/fdsnws/station/1"
url_waveform="https://service.iris.edu/irisws/timeseries/1"
url_response="https://service.iris.edu/irisws/sacpz/1"
#
EOF

getdata par=getdata.par net=IC sta="HIA,BJT,MDJ,SSE,XAN,ENH"  loc="*" chan="BH?,HH?"
getdata par=getdata.par net=KS sta="*"  loc="*" chan="BH?,HH?"
getdata par=getdata.par net=KG sta="*"  loc="*" chan="*"
getdata par=getdata.par net=IU sta=INCN loc="*" chan="BH?,HH?"
getdata par=getdata.par net=JP sta="JTU,JSU,JNU,JHS,JWT,JSD" loc="*" chan="BH?,HH?"
