
Amfeltec USB Device driver
==========================

1. Hardware installation

	Connect the Amfeltec USB device into an available USB port

2. DAHDI installation
  
2.1 Download and install Dahdi release

      * Download Dahdi-Linux and Dahdi-tools from www.asterisk.org
      * cd /usr/src
      * tar xfz dahdi-linux<ver>.tar.gz
      * cd dahdi-linux-<ver>
      * make all
      * make install
      * make config

2.2 Make necessary changes in /etc/dahdi/system.conf file


3. Amfeltec USB driver installation

3.1 Untar release

      * tar xfz amfeltec_usb-<ver>.tgz
      * cd amfeltec_usb-<ver>
  
3.2 Compile and install Amfeltec USB driver 

      * make DAHDI_DIR=/usr/src/dahdi-linux-<ver>
      * make install  
      * make boot

3.3 Start driver using Dahdi Startup Script and confirm that Amfeltec USB driver (amf_usb) is loaded


      * service dahdi start 
		
	  (/etc/init.d/dahdi start)  

3.4 Confirm that Amfeltec USB analog span is registered with DAHDI

      * service dahdi status

  	  (/etc/init.d/dahdi status)  


4.0 FreePBX installation notes: see User Manual

