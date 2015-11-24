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

 # check_imaging.sh
 # Author: Katie Awty-Carroll (kah36@aber.ac.uk)
 # 
 # Checks the size of the output files and reduces it if needed. Also
 # checks if the run_imaging script is running, and starts it if not.

rfid_out_size=$(ls -l --block-size=1K ./output/rfid_out.txt | grep "[r]fid_out.txt" | awk '{print $5}')
out_size=$(ls -l --block-size=1K ./output/out.txt | grep "[o]ut.txt" | awk '{print $5}')

if [[ $rfid_out_size > 250 ]]; then
	touch ./output/rfid_temp
	tail -n 1500 ./output/rfid_out.txt > ./output/rfid_temp
	rm ./output/rfid_out.txt
	mv ./output/rfid_temp ./output/rfid_out.txt
fi
if [[ $out_size > 250 ]]; then 
	touch ./output/out_temp
	tail -n 1500 ./output/out.txt > ./output/out_temp
	rm ./output/out.txt
	mv ./output/out_temp ./output/out.txt
fi

ps -ef | grep "[r]un_imaging.sh"

if [[ $? -eq 1 ]]; then
	setsid ./run_imaging.sh >> ./output/out.txt 2>&1 &
fi

