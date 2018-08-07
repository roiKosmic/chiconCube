# chiconCube

##Pre-requisite
Arduino code for the chicon Cube smart lamp. 
You need some external libraries to run this code :
* shiftPWDM available at https://github.com/elcojacobs/ShiftPWM
* WifiLibrary available in official arduino IDE

You also need a chic'on server account at http://www.chicon.fr/ (deprecated - please run you own chicon server) or run your own chic'on server source code on ChiconServer Repository

##Instructions
Follow this instructable to build your own chicon Cube

##Configuration
All configuration variables are in the config.h file. Please update accordingly
 * Serial Number is given upon request on your Chic'on account page
String sNumber ="";

 * Magic Number is given when you add a device on your chic'on account
String magicNumber = "";

 * Your Wifi configuration 
static const char ssid[] = ""; //ssid 
static const char pass[] = ""; //WPA password

* Chic'on Server configuration
Leave unchange if you are using the online www.chicon.fr (deprecated please use your own chic'on server) If you run your own Chic'on sever update accordingly. Make sure your server name is resolvable.

char server[] = "www.chicon.fr";
static const String WEBSERVICE_URL = "/chicon/webServices/hdwWS.php";
static char HTTP_REQUEST[] = " HTTP/1.1\r\nHost:www.chicon.fr\r\n\r\n";

Note: Chic'on Cube retrieves IP, gateway and DNS information from DHCP. You must have a running DHCP server on your wifi Network.

## Going Further
 * Check the this repo wiki to go further and build your own Chic'on compatible lamp.
