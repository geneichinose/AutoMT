#!/bin/csh

### 38.172° N	118.0674° W	2021-02-06 23:10:16 UTC	9.03 km	Mwr3.7	California-Nevada Border Region

/bin/rm -fr Data Resp getdata.????????.log channel.xml getdata.par

set wdir=(`pwd`)
echo $wdir

cat >! getdata.par << EOF
working_directory=${wdir}
#
curl_options="/usr/bin/curl -k -s --max-time 60 --connect-timeout 60 "
start_time=2021-02-06T23:10:16
#
# duration_sec=900
# end_time=2020-07-01T00:15:00
#
pre_time_sec=300
post_time_sec=1200
#
# search_type=single_station
# net=IM
# sta=NV31
# loc="*"
# chan="HH?,BH?"
# 
### Nevada,MCR
search_type=circle clat=38.6 clon=-118.5 radiuskm=41
#
#### Iran/MidEast/Asia
# search_type=box minlon=38.0 maxlon=77.0  minlat=20.0 maxlat=40.0
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
