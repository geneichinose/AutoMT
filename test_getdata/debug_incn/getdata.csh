#!/bin/csh

### 2021/02/07T18h37m32.79 40.495 127.109 North Korea mb3.4 ml 3.4 IDC_REB:IDC_SEL 
### 2021-02-07T18:37:34.00 40.67  127.45  24 ml 3.2

/bin/rm -fr Data Resp dev getdata.????????.log channel.xml getdata.par

set wdir=(`pwd`)
echo $wdir

cat >! getdata.par << EOF
working_directory=${wdir}
#
curl_options="/usr/bin/curl -k -s --max-time 60 --connect-timeout 60 "
start_time=2021-02-07T18:37:34
#
# duration_sec=900
# end_time=2020-07-01T00:15:00
#
pre_time_sec=900
post_time_sec=1800
#
search_type=single_station
net=IU
sta=INCN,MAJO
loc="*"
chan="HH?,BH?"
# 
### 2021-02-07T183734_KMA_North_Korea
# search_type=circle clat=40.67 clon=127.45 radiuskm=1500
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

getdata par=getdata.par setupMT evla=40.67 evlo=127.45 evdp=24 ot=2021-02-07T18:37:34.00 vmod=wus
