#!/bin/csh

### query all earthquakes within the last 3days 
###
### 1days=86400 2days=172800 3days=259200sec 4days=345600 5days=432000 7days=604800
###
set ST=(`/Users/ichinose1/bin/now time sec_offset=-604800`)
set ET=(`/Users/ichinose1/bin/now time` )
set UD=(`/Users/ichinose1/bin/now time sec_offset=-604800`)
echo $ST
echo $ET
echo $UD
set MINMAG=3.5

### Asia
#set MINLAT=-20
#set MAXLAT=+90
#set MINLON=-20
#set MAXLON=170

### Whole World
set MINLAT=-90
set MAXLAT=+90
set MINLON=-180
set MAXLON=180

set FORMAT=text

### get list of realtime events, compare with previous list or just request an updated list since last query?
###

curl -o events.isc.txt "http://www.isc.ac.uk/fdsnws/event/1/query?starttime=${ST}&endtime=${ET}&minlatitude=${MINLAT}&maxlatitude=${MAXLAT}&minlongitude=${MINLON}&maxlongitude=${MAXLON}&minmagnitude=${MINMAG}&maxmagnitude=10&includeallmagnitudes=FALSE&format=isf&limit=9999"

curl -o events.isc.xml "http://www.isc.ac.uk/fdsnws/event/1/query?starttime=${ST}&endtime=${ET}&minlatitude=${MINLAT}&maxlatitude=${MAXLAT}&minlongitude=${MINLON}&maxlongitude=${MAXLON}&minmagnitude=${MINMAG}&maxmagnitude=10&includeallmagnitudes=FALSE&format=xml&limit=9999"

curl -o events.usgs.xml "http://earthquake.usgs.gov/fdsnws/event/1/query?method=query&format=xml&starttime=${ST}&endtime=${ET}&minmagnitude=${MINMAG}&maxmagnitude=10&minlatitude=${MINLAT}&maxlatitude=${MAXLAT}&minlongitude=${MINLON}&maxlongitude=${MAXLON}&limit=2000&orderby=time&updatedafter=${UD}"

curl -o events.usgs.txt "http://earthquake.usgs.gov/fdsnws/event/1/query?format=text&starttime=${ST}&endtime=${ET}&minmagnitude=${MINMAG}&maxmagnitude=10&minlatitude=${MINLAT}&maxlatitude=${MAXLAT}&minlongitude=${MINLON}&maxlongitude=${MAXLON}&limit=2000&orderby=time&updatedafter=${UD}"

curl -o events.iris.txt "http://service.iris.edu/fdsnws/event/1/query?start=${ST}&end=${ET}&minmag=${MINMAG}&maxmag=10&minlat=${MINLAT}&maxlat=${MAXLAT}&minlon=${MINLON}&maxlon=${MAXLON}&limit=10000&orderby=time&updatedafter=${UD}&includeallmagnitudes=FALSE&includeallorigins=FALSE&format=${FORMAT}"

curl -o events.iris.xml "http://service.iris.edu/fdsnws/event/1/query?start=${ST}&end=${ET}&minmag=${MINMAG}&maxmag=10&minlat=${MINLAT}&maxlat=${MAXLAT}&minlon=${MINLON}&maxlon=${MAXLON}&limit=10000&orderby=time&updatedafter=${UD}&includeallmagnitudes=FALSE&includeallorigins=FALSE&format=xml"

evexml xml=events.iris.xml >! xml.out
evexml xml=events.usgs.xml >> xml.out
evexml xml=events.isc.xml >> xml.out

### get list of realtime stations with waveforms?
### Don't need, use station web service with matchtimeseries=TRUE option
###
# curl -k -o nets.txt "https://service.iris.edu/irisws/virtualnetwork/1/query?code=_REALTIME&format=xml&starttime=${ST}&endtime=${ET}&definition=false"
# cat nets.txt

### Get list of stations near a lat/lon (event), first get station level list
###  level=network,station,channel (response not supported)
###  nodata=404 (error code 404), 204 (empty file)
###
set EVLA=42.1
set EVLO=129.1
set MAXRAD=12.0
set NODATA=404
set LEVEL=station
set FORMAT=text

curl -k -o sta.txt "https://service.iris.edu/fdsnws/station/1/query?start=${ST}&end=${ET}&latitude=${EVLA}&longitude=${EVLO}&maxradius=${MAXRAD}&nodata=${NODATA}&channel=BH*&level=${LEVEL}&format=${FORMAT}&matchtimeseries=TRUE&includerestricted=FALSE&includeavailability=FALSE&includecomments=FALSE"
cat sta.txt

set FORMAT=xml
curl -k -o sta.xml "https://service.iris.edu/fdsnws/station/1/query?start=${ST}&end=${ET}&latitude=${EVLA}&longitude=${EVLO}&maxradius=${MAXRAD}&nodata=${NODATA}&channel=BH*&level=${LEVEL}&format=${FORMAT}&matchtimeseries=TRUE&includerestricted=FALSE&includeavailability=FALSE&includecomments=FALSE"

./read_stachan_xml sta.xml

### then get a channel level list for data query
###
set LEVEL=channel
set FORMAT=text

curl -k -o chan.txt "https://service.iris.edu/fdsnws/station/1/query?start=${ST}&end=${ET}&latitude=${EVLA}&longitude=${EVLO}&maxradius=${MAXRAD}&nodata=${NODATA}&channel=BH*&level=${LEVEL}&format=${FORMAT}&matchtimeseries=TRUE&includerestricted=FALSE&includeavailability=FALSE&includecomments=FALSE"

cat chan.txt

set FORMAT=xml
curl -k -o chan.xml "https://service.iris.edu/fdsnws/station/1/query?start=${ST}&end=${ET}&latitude=${EVLA}&longitude=${EVLO}&maxradius=${MAXRAD}&nodata=${NODATA}&channel=BH*&level=${LEVEL}&format=${FORMAT}&matchtimeseries=TRUE&includerestricted=FALSE&includeavailability=FALSE&includecomments=FALSE"

./read_stachan_xml chan.xml

### From channel level station query, get the waveform data
###
set NET=IC
set STA=BJT
set LOC=00
set CHAN=BHZ
set ST=2016-10-17T07:14:49
set ET=2016-10-17T08:14:49

set sacfilename=2016.10.17.07.14.49.${NET}.${STA}.${LOC}.${CHAN}.SAC
set sacpzfilename=${NET}.${STA}.${LOC}.${CHAN}.sacpz

/bin/rm -rf ${sacfilename} ${sacpzfilename}
curl -k -o ${sacfilename} "https://service.iris.edu/irisws/timeseries/1/query?net=${NET}&sta=${STA}&loc=${LOC}&cha=${CHAN}&starttime=${ST}&endtime=${ET}&output=sacbl"
curl -k -o ${sacpzfilename} "https://service.iris.edu/irisws/sacpz/1/query?net=${NET}&sta=${STA}&loc=${LOC}&cha=${CHAN}&start=${ST}&end=${ET}"
