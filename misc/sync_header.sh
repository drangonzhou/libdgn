#!/bin/bash

# Copyright (C) 2011 ~ 2019 drangon zhou <drangon.zhou (at) gmail.com>
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.

# sync header file reference here

function sync_header
{
	sdir=$1
	hds=$(cd ../$sdir ; ls *.h)
	for hd in $hds ; do
		if [ -f ../dgn/$hd ] ; then
			continue
		fi
		echo "#include \"../$sdir/$hd\"" > ../dgn/$hd
	done
}

sync_header dgnbase
# sync_header dgnsup
# sync_header dgnext
# sync_header dgncxx

