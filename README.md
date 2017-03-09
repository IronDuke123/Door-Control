# Door-Control
This is code and possibly some hardware info to control shop garage doors using arduino code and some relays over the shop network.. 

Was asked to come up with a solution for the writers to be able to see and to also open the 3 shop doors easily and quickly.. With the remodel it became difficult to see the doors with the new counter location.. Also relocating the switches to operate the doors could become troublesome..

For viewing 2 doors that were difficult to see  simply installed a cheap vga camera and used an old monitor I mounted on the wall.. Power was provided by the 12v power supply I am already using for remote control of the doors in the service section of the shop.. 

For remote controls of the doors I created a very simply web page served by an ardunio and used a bank of 9 relays to mimic the door control switches.. All the writer has to do is open or keep open the web page and click on which door and whether he wants to open/stop, or close the door.. 

I also created a small program on each writers computer that uses keyboard shortcuts and simply sends the corrosponding wget command to operate the door, no web page needed.. 
hold down cntrl/alt and use the number pad to operate the doors.. 

This has been running for 3 years with no issues, seamless and very easy to use.. 

Please bear with me, or ask if you don't see what you need or if something seems to be missing.. First time using github..
