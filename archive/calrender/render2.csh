#!/bin/csh
foreach i ( $argv )
cat calband.pov $i calband.pov1 >! test.pov
povray -w640 -h400 -itest.pov -o`echo $i | sed 's/pov/qrt/'` +fd -a -v
gzip -f9 `echo $i | sed 's/pov/qrt/'`
rm -f $i
end

rm -f test.pov
