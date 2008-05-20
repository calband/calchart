#!/bin/csh
foreach i ( $argv )
cat calband.pov $i calband.pov1 > test.pov
povray -w640 -h480 -itest.pov -o`echo $i | sed 's/pov/tga/'` +ft -a -v
end
