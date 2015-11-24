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

#Dimensions and offsets for image cropping
lv_x=4016
lv_y=6016
lv_offset_x=0
lv_offset_y=0

cv_x=1900
cv_y=3500
cv_offset_x=750
cv_offset_y=1500

rv_x=3400
rv_y=6016
rv_offset_x=0
rv_offset_y=0

img_path=$1

if [[ $img_path ]]; then
	img_ext=${img_path: -4}
else
	echo "No image specified"
	exit 1
fi

if [[ $img_ext != ".NEF" && $img_ext != ".nef" ]]; then
	echo "File is not a raw file"
	exit 1
else 
	img_png=${img_path%.*}".png"
	img_exif=${img_path%.*}".exif"
	python ./exif-py/EXIF.py -q $img_path > $img_exif
	
	if [[ $img_path == *"_lv_"* ]]; then 
		gimp -i -b "(convert-raw-to-png \"$img_path\" \"$img_png\" $lv_x $lv_y $lv_offset_x $lv_offset_y)"
	elif [[ $img_path == *"_cv_"* ]]; then
  		gimp -i -b "(convert-raw-to-png \"$img_path\" \"$img_png\" $cv_x $cv_y $cv_offset_x $cv_offset_y)"
	elif [[ $img_path == *"_rv_"* ]]; then
		gimp -i -b "(convert-raw-to-png \"$img_path\" \"$img_png\" $rv_x $rv_y $rv_offset_x $rv_offset_y)"
	fi
	rm $img_path
fi

exit 0
