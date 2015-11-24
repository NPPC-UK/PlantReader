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

 # stop_imaging.sh
 # Author: Katie Awty-Carroll (kah36@aber.ac.uk)
 # 
 # Finds the process ID's of all the scripts used to capture images 
 # from the SLR cameras, and kills the processes. If run during an 
 # experiment, this could result in incomplete data, or communication 
 # issues with cameras on restarting.

rfid_pid=$(ps -ef | grep "[r]ead_rfid" | awk '{print $2}')
camera_pid=$(ps -ef | grep "[c]amera_script.py" | awk '{print $2}')
run_pid=$(ps -ef | grep "[r]un_imaging.sh" | awk '{print $2}')

if [[ $run_pid ]]; then
	echo "Killing run_imaging.sh"
	kill $run_pid
else
	echo "run_imaging.sh was not running"
fi
if [[ $camera_pid ]]; then
	echo "Killing camera_script.py"
	kill $camera_pid
else
	echo "camera_script.py was not running"
fi
if [[ $rfid_pid ]]; then
	echo "Killing read_rfid"
	kill $rfid_pid
else
	echo "read_rfid was not running"
fi

echo "Done"
