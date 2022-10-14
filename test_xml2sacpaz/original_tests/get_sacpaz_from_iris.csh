#!/bin/csh

/usr/bin/curl -o GE.StationXML.xml \
 "https://service.iris.edu/fdsnws/station/1/query?net=GE&sta=*&loc=*&cha=BH?,HH?&starttime=1991-03-01T00:00:00&endtime=2022-10-13T00:00:00&level=response&format=xml&includecomments=true&nodata=404"

/usr/bin/curl -o PL.StationXML.xml \
 "https://service.iris.edu/fdsnws/station/1/query?net=PL&sta=*&loc=*&cha=BH?,HH?&starttime=1991-03-01T00:00:00&endtime=2022-10-13T00:00:00&level=response&format=xml&includecomments=true&nodata=404"

/usr/bin/curl -o DK.StationXML.xml \
 "https://service.iris.edu/fdsnws/station/1/query?net=DK&sta=*&loc=*&cha=BH?,HH?&starttime=1991-03-01T00:00:00&endtime=2022-10-13T00:00:00&level=response&format=xml&includecomments=true&nodata=404"

/usr/bin/curl -o UP.StationXML.xml \
 "https://service.iris.edu/fdsnws/station/1/query?net=UP&sta=*&loc=*&cha=BH?,HH?&starttime=1991-03-01T00:00:00&endtime=2022-10-13T00:00:00&level=response&format=xml&includecomments=true&nodata=404"
