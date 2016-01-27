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

 # process_raw_image.sh
 # Author: Katie Awty-Carroll (kah36@aber.ac.uk)
 # 
 # Script to convert raw NEF files from Nikon D3300 into PNG's, then auto 
 # white balance/crop them with Gimp. Also extracts EXIF data using the 
 # Python exifread library.

#Fetch command line arguments
img_path=$1
crop_x=$2
crop_y=$3
offset_x=$4
offset_y=$5
brightness=$6
contrast=$7
if_balance=$8

#Check if the image exists
if [[ $img_path ]]; then
	img_ext=${img_path: -4}
	if [ ! -f $img_path ]; then
		echo "Image does not exist"
		exit 1
	fi
else
	echo "No image specified"
	exit 1
fi

#Check that the image is the correct type (NEF's are Nikon raw image files)
if [[ $img_ext != ".NEF" && $img_ext != ".nef" ]]; then
	echo "File is not a raw file"
	exit 1
else 
	#Set up new file names
	img_png=${img_path%.*}".png"
	img_exif=${img_path%.*}".exif"
	
	#Extract the EXIF data and store it to a text file (PNG's don't retain EXIF data)
	python ./exif-py/EXIF.py -q $img_path > $img_exif
	
	#Use the GIMP to perform adjustments to the image, and save the new PNG
	gimp -i -b "(convert-raw-to-png \"$img_path\" \"$img_png\" $crop_x $crop_y $offset_x $offset_y $brightness $contrast $if_balance )"

	#Remove the old NEF image to save space
	rm $img_path
fi

exit 0
