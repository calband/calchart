#!/bin/csh
rm -f octree100.data
foreach i ( $argv )
gzip -dc $i | qrt2fbm | fboctree -a octree100.data
end

rm -f test.list
foreach i ( $argv )
gzip -dc $i | qrt2fbm | fboctree -G octree100.data > `echo $i | sed 's/qrt.gz/gif/'`
rm -f $i
echo `echo $i | sed 's/qrt.gz/gif/'` >> test.list
end

rm -f octree100.data
rm -f test.fli
fbm2fli -rx640 -ry400 test.list test.fli

foreach i ( `cat test.list` )
rm -f $i
end
rm -f test.list
