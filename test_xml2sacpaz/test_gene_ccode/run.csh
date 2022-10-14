#!/bin/csh

###cleanup
/bin/rm -f SAC_PZs_*
/bin/rm -f xml2sacpaz.????????.log

xml2sacpaz xml=../input/Package_1664845181885_0.xml

rm -rf ../sacpz_gene_ccode
mkdir ../sacpz_gene_ccode
mv SAC_PZs_* ../sacpz_gene_ccode
