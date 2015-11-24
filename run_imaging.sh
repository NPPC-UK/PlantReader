#!/bin/bash

 # Copyright (C) 2015		Aberystwyth University	
 
 # This program is free software: you can redistribute it and/or modify
 # it under the terms of the GNU General Public License as published by
 # the Free Software Foundation, either version 3 of the License, or
 # (at your option) any later version.

 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.

 # You should have received a copy of the GNU General Public License
 # along with this program.  If not, see <http://www.gnu.org/licenses/>.

 # run_imaging.sh
 # Author: Katie Awty-Carroll (kah36@aber.ac.uk)
 # 
 # Runs camera_script.py on an endless loop, and redirects output to
 # out.txt.

while true; do
python3 ./camera_script.py >> ./output/out.txt 2>&1
done

