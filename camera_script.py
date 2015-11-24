#!/usr/bin/env python3

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

 # camera_script.py 
 # Author: Katie Awty-Carroll (kah36@aber.ac.uk)
 # 
 # Python 3.2.3 script to detect attached cameras and take a picture with 
 # each one. This version assumes that all cameras are permanently switched 
 # on. Used with libgphoto2 2.5.8 compiled from source - versions prior to 
 # 2.5.6 may lack functionality with Nikon D3300 cameras, see http://www.gphoto.org/news/.
 # Camera parameters should be altered seperately with gphoto2. Note that for 
 # full functionality with gphoto2 cameras need to be in Manual mode and lenses 
 # in Manual/Auto.
 # This script requires the C program read_rfid to be in the same directory as 
 # it in order to run.

import sys
import time
import subprocess
import re
import os
from configparser import ConfigParser				

cameras = {}

#Find all attached cameras, and check that they match against the serial numbers in the config file.
def find_cameras(parser):
	
	#Only look for this particular model of camera
	def_camera = "Nikon DSC D3300"		

	try:
		output = subprocess.check_output(["gphoto2","--auto-detect"], universal_newlines=True)
	except subprocess.CalledProcessError as e:
		print("***ERROR***: Failed to run gphoto2 autodetect: {}".format(e))
		sys.exit(1)

	output = output.splitlines()

	for line in output:
		if re.search(def_camera, line):
			ports = line.split(":",1)
			usb_port = "usb:" + ports[1].strip()
			
			try:
				serial_output = subprocess.check_output(["gphoto2", "--port", usb_port, "--get-config","/main/status/serialnumber"], universal_newlines=True)
				serial_output = serial_output.splitlines()
				serial_line = serial_output[2].split(":",1)
				serial_num = serial_line[1].strip()
			
				for (camera_serial, camera_name) in parser.items("camera_list"):
					if camera_serial == serial_num:
						cameras[serial_num] = usb_port

			except subprocess.CalledProcessError as e:
				print("***WARNING***: Failed to fetch serial number for port {}: {}".format(usb_port, e))	
				
	if not cameras:
		print("***ERROR***: No cameras detected")
		sys.exit(1)
	else:
		print("Number of cameras found: {}".format(len(cameras)))	

#Send a command to server for each image, telling it to be processed
def process_images(paths):
	
	print("Processing {} images".format(len(paths)))
	for image in paths:
		if os.path.exists(image):
			try:
				process_string = "ssh user@server \'setsid process_raw_image.sh " + image + " >> /dev/null 2>&1\'"
				subprocess.Popen([process_string], shell=True)
			except (OSError, ValueError) as e:
				print("***WARNING***: Failed to run processing on image {}: {}".format(image, e))	

#Take a picture with each camera for each angle in the configuration file.
def take_pictures(directory_path, parser):
	
	commands = []
	image_paths = []

	angle_delay = float(parser.get("timing","angle_delay"))
	for (angle_name, angle) in parser.items("angles"):
		for camera in cameras:
				cam_pos = parser.get("camera_list", camera)
				file_name = directory_path + "/" + parser.get("image_naming", "config") + "_" + parser.get("image_naming", "modality") + "_" + cam_pos + "_" + angle + "-" + parser.get("image_naming", "n1") + "-" + parser.get("image_naming", "n2") + "-" + parser.get("image_naming", "n3")
				whole_path = file_name + ".NEF"
				image_paths.append(whole_path)
				commands.append(["gphoto2", "--port", cameras[camera], "--capture-image-and-download", "--force-overwrite","--filename", whole_path])
		
		try:		
			processes = [subprocess.Popen(command) for command in commands]
		except (OSError, ValueError) as e:
			print("***WARNING***: Failed to run command {}: {}".format(command, e))			
			
		for process in processes:
			process.wait()
		
		process_images(image_paths)

		#Reset arrays for the next set of images to be collected
		commands[:] = [] 
		image_paths[:] = []

#Using config file, first check whether the main path specified is present. If not, check if an alternate path has been specified, and use that if it exists.
def check_for_drives(parser):
		
	main_drive = parser.get("drives", "main_drive")

	if not os.path.exists(main_drive):
		print("Cannot find {}".format(main_drive))
		if parser.has_option("drives", "alt_drive"):
			alt_drive = parser.get("drives", "alt_drive")
			print("Checking backup space on {}".format(alt_drive))
			if not os.path.exists(alt_drive):
				print("{} cannot be accessed either, images cannot be saved".format(alt_drive))
				sys.exit(1)
			else:
				drive_use = subprocess.check_output(["df","-h",alt_drive], universal_newlines=True)
				drive_use = drive_use.split()
				drive_use = int(drive_use[11].strip("%"))
				if drive_use > 95:
					print("***ERROR***: Backup drive is full, cannot save images")
					sys.exit(1)
				else:	
					print("***WARNING***: Using backup drive, which is {}% full".format(drive_use))
					return alt_drive
		else:
			print("No alternate drive. Images cannot be saved")	
			sys.exit(1)
	else:
		print("Found {}".format(main_drive))
	
		return main_drive

#Checks that the correct drive is mounted and runs the C script read_rfid which returns the name of the next plant according to its RFID tag. Call take_pictures() to capture images.
def main():
	
	plant_name = None		
	parser = ConfigParser()
	
	print(time.strftime("%Y-%m-%d %H:%M:%S"))
	print("Attempting to read from RFID tag...")

	if not os.path.exists("./read_rfid"):
		print("***ERROR***: C script is not present")
		sys.exit(1)

	try:
		plant_name = subprocess.check_output(["./read_rfid"],universal_newlines=True)
	except subprocess.CalledProcessError as e:
		print("***ERROR***: Problem running C script: {} See \"output/rfid_out.txt\"".format(e))
		sys.exit(1)

	if plant_name == None:
		print("***ERROR***: No output from C script")
		sys.exit(1)

	print("Successfully read from tag {}".format(plant_name))
	
	plant_details = plant_name.split("-",1)
	experiment_name = plant_details[0]

	config_file_path = "./config/" + experiment_name
	
	if not os.path.exists(config_file_path):
		print("***ERROR***: Config file is not present")
		sys.exit(1)

	parser.read(config_file_path)

	if parser.getboolean("misc", "capture_empty_carts") == False and plant_details[1][0] == "E":
		print("Cart is empty, image will not be captured")
		sys.exit(1)

	print("Checking directory paths...")
	drive_path = check_for_drives(parser)

	print("Finding cameras...")	
	find_cameras(parser)
	
	num_cams_expected = len(parser.options("camera_list"))
	if len(cameras) != num_cams_expected:
		print("***WARNING***: Incorrect number of cameras found. Expected {} camera(s), found {}".format(num_cams_expected, len(cameras)))

	date = time.strftime("%Y-%m-%d")

	dir_path = drive_path + experiment_name + "/" + plant_name + "/" + date

	print("Checking for correct directory structure and creating if needed...")

	if not os.path.exists(dir_path):
		try:
			os.makedirs(dir_path)
			os.chmod(dir_path, 0o755)
			print("Image will be saved in: {}".format(dir_path))
		except OSError as e:
			print("***ERROR***: Could not create directory path, cannot save image: {}".format(e))
			sys.exit(1)
	
	print("Waiting...")
	time.sleep(float(parser.get("timing","delay"))) 
	print("Taking pictures...")
	take_pictures(dir_path, parser)	

	print("Done")

if __name__ == '__main__':
    main()














