#!/bin/csh
### 48.7584° N    125.2579° E     2020-02-07 00:12:40 UTC 10.0 km mb4.5   Northeastern China
### 41.3343° N	129.0307° E	2017-09-03 03:30:01 UTC	0.0 km	mb6.3	North Korea

/bin/rm -fr Data Resp getdata.????????.log channel.xml getdata.par

set wdir=(`pwd`)
echo $wdir

cat >! getdata.par << EOF
working_directory=${wdir}
curl_options="/usr/bin/curl -k -s --max-time 60 --connect-timeout 60 "
start_time=2020-02-07T00:00:00
####
#### Three ways to get data time-windows
####
#### 1. use a duration in seconds (from start_time + duration_sec)
####
duration_sec=1800
#
####
#### 2. t_start=(pre_time_sec - start_time) 
####    t_stop=(start_time + post_time_sec)
####    NOTE!: This resets start_time
####
# pre_time_sec=900
# post_time_sec=1800
####
#### 3. start_time + end_time
####
# end_time=2020-02-07T00:15:00
#
#search_type=single_station net=IC sta=MDJ loc=00 chan=BHZ
search_type=circle clat=41.3343 clon=129.0307 radiuskm=1200
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
EOF

getdata par=getdata.par 
