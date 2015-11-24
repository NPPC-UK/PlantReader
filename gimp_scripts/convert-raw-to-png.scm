 ; Copyright (C) 2015		Aberystwyth University	
 
 ; This program is free software: you can redistribute it and/or modify
 ; it under the terms of the GNU General Public License as published by
 ; the Free Software Foundation, either version 3 of the License, or
 ; (at your option) any later version.

 ; This program is distributed in the hope that it will be useful,
 ; but WITHOUT ANY WARRANTY; without even the implied warranty of
 ; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ; GNU General Public License for more details.

 ; You should have received a copy of the GNU General Public License
 ; along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ; convert-raw-to-png.scm
 ; Author: Katie Awty-Carroll (kah36@aber.ac.uk)
 ; 
 ; Accepts as parameters the image file path and details for cropping  
 ; the image. White balances, crops and adjusts the brightness/contrast 
 ; of the image to reduce its file size. Saves the file as a PNG.
 ; The input file must be a RAW file.

(define (convert-raw-to-png filename new-filename crop_x crop_y offset_x offset_y)
	 	(catch (gimp-quit 1)
    	(let* ( (image (car (file-ufraw-load RUN-NONINTERACTIVE filename filename)))
    	(drawable (car (gimp-image-get-active-layer image))))
		(gimp-levels-stretch drawable)	; White balance
		(gimp-image-crop image crop_x crop_y offset_x offset_y) ; Crop
		(gimp-brightness-contrast drawable 25 30) ; Increase brightness and contrast
    	(file-png-save-defaults RUN-NONINTERACTIVE image drawable new-filename filename)
		(gimp-quit 0))))
