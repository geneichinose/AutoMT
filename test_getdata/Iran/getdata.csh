#!/bin/csh

### 30.8469° N	51.3838° E	2021-02-17 18:35:35 UTC	10.0 km	Mww5.4	Northern And Central Iran

/bin/rm -fr Data Resp getdata.????????.log channel.xml getdata.par

set wdir=(`pwd`)
echo $wdir

cat >! getdata.par << EOF
working_directory=${wdir}
#
curl_options="/usr/bin/curl -k -s --max-time 60 --connect-timeout 60 "
start_time=2021-02-17T18:35:35
#
# duration_sec=900
# end_time=2020-07-01T00:15:00
#
pre_time_sec=300
post_time_sec=900
#
# search_type=single_station
# net=IC
# sta=MDJ
# loc=00
# chan=BHZ
#
# search_type=circle clat=38.172 clon=-118.0674 radiuskm=200
#
search_type=box minlon=38.0 maxlon=77.0 minlat=20.0 maxlat=40.0
#
getwave
getresp
verbose
map_gmt_stations
#
url_metadata="https://service.iris.edu/fdsnws/station/1"
url_waveform="https://service.iris.edu/irisws/timeseries/1"
url_response="https://service.iris.edu/irisws/sacpz/1"
EOF

getdata par=getdata.par 
