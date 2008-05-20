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
fbm2fli -rx320 -ry200 test.list test.fli

foreach i ( `cat test.list` )
rm -f $i
end
rm -f test.list
