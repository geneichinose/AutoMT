#!/bin/csh

### get the sacpaz
###
/usr/bin/curl -o SAC_PZs_IU_ANMO_BHZ_00_2021-03-11T00:00:00_2021-03-12T00:00:00  \
 "https://service.iris.edu/irisws/sacpz/1/query?net=IU&sta=ANMO&loc=00&cha=BHZ&starttime=2021-03-11T00:00:00&endtime=2021-03-12T00:00:00"

### get the stationXML
###
/usr/bin/curl -o test.xml \
 "https://service.iris.edu/fdsnws/station/1/query?net=IU&sta=ANMO&loc=00&cha=BHZ&starttime=1991-03-01T00:00:00&endtime=2021-03-12T00:00:00&level=response&format=xml&includecomments=true&nodata=404"

# /usr/bin/curl -o test2.xml \
# "https://service.iris.edu/fdsnws/station/1/query?net=IU&sta=*&loc=*&cha=BH?,HH?&starttime=1991-03-01T00:00:00&endtime=2021-03-12T00:00:00&level=response&format=xml&includecomments=true&nodata=404"

