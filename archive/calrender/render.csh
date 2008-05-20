#!/bin/csh
#
#  Copyright (C) 1995  Garrick Brian Meeker
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

foreach i ( $argv )
cat calband.pov $i calband.pov1 >! test.pov
povray -w320 -h200 -itest.pov -o`echo $i | sed 's/pov/qrt/'` +fd -a -v
gzip -f9 `echo $i | sed 's/pov/qrt/'`
rm -f $i
end

rm -f test.pov
