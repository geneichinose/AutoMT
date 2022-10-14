#!/bin/csh

/bin/rm -f SAC_PZs_*
/bin/rm -f  xml2sacpaz.py.out

xml2sacpaz.py ../input/*.xml >& xml2sacpaz.py.out

/bin/rm -rf ../sacpz_python
mkdir ../sacpz_python
mv SAC_PZs_* ../sacpz_python
