# PlantReader
Plant imaging with SLR cameras and RFID tag reader

PlantReader is a collection of scripts used to automate high quality imaging of our plants. PlantReader uses the unique code that each plant has stored in its RFID tag to capture an image (or multiple images) of that plant and store
them appropriately. It consists of a C program which uses the libnfc library to read from the correct locations on the RFID tags and check that plant codes are valid and in our database, as well as a Python script to interface with gphoto2 and coordinate the C script in order to capture the images at the right moment. In addition there are bash scripts to automate running and checking. 

Each experiment we run has a configuration file which the Python script uses to determine how images for that experiment should be captured. This includes parameters for automated image processing as well as file names and file 
storage locations.

Some processing of the images can also be carried out using the GIMP. This involves running a short macro which can auto white balance, crop, adjust brightness/contrast and save the image as a PNG. ExifTool can also be used to extract
EXIF data from RAW files and store it as a comment in the PNG files.

List of libraries:

libnfc			- http://nfc-tools.org/index.php?title=Libnfc
gphoto2			- http://gphoto.sourceforge.net/
ExifTool		- http://www.sno.phy.queensu.ca/~phil/exiftool/
ConfigParser 	- https://docs.python.org/3/library/configparser.html#module-configparser
libpq			- http://www.postgresql.org/docs/9.1/static/libpq.html

-----Full list of scripts used as part of SLR camera image capture

camera_script.py  
Main Python 3 coordinator script. Runs read_rfid to fetch a plant name. Creates directories for images, sets up cameras and issues instructions to cameras with gphoto2. Sends file paths of captured images to server for processing. 
  
read_rfid  
C program that reads from RFID tags. Will return the full experiment and plant ID from the tag as an output (e.g. AB1-01234). Must be compiled from main.c, mifare.c and db_connect.c using the correct libraries.

run_imaging.sh  
Bash script that runs camera_script.py on an infinite loop. As the RFID reader itself is used as a detector, read_rfid can constantly be waiting for the next tag.

check_imaging.sh  
Bash script run by cron that checks if camera_script.py is still running, and starts it if not. Also checks the file sizes of output files.

stop_imaging.sh  
Bash script that can be run to halt imaging. Finds the process ID's of relevent processes and kills them.

---On server:

process_raw_image.sh  
Bash script that takes the path of a .NEF file as a command line parameter. Uses EXIF.py to extract the data from the raw file before using GIMP to process it and then deleting it once a PNG has been created.

convert-raw-to-png.scm  
GIMP macro script to automate the processing of a raw image. Accepts the image path and cropping details as command line parameters. White balances the image, crops it and adjusts the brightness and contrast before saving the file as a PNG.

-----Additional files used

A config file must be created for each experiment to be captured. This must be placed in /config and be named after the experiment (e.g. CR6).

In /output there are two output files. One (rfid_out.txt) only records the time that each RFID tag was read and the details of that tag. The other (out.txt) records all other output from scripts. The size of these files is limited to 250kb.

A cron job is used to run check_imaging.sh. This takes the following format:  
*/2 * * * * /path/to/scripts/check_imaging.sh >> /path/to/scripts/output/out.txt 2>&1
