char* firebase_Root  = "https://[YOURDBNAME].firebaseio.com/";
char* firebase_Secret = "[YOURSECRETKEY]";
char* AP_Names = "Deployment Blinker";
char* firebase_Organization = "[ORGINIZATION]";
char* firebase_SystemName = "[SystemToMonitor]";

/*
	Expected firebase layout 
	https://[YOURDBNAME].firebaseio.com/[ORGINIZATION]/[SystemToMonitor]
	
	The message that it process in this location should look like this JSON : 
	{
		"eventDate" : "2020-02-27T20:18:28.0639589Z",
		"systemName" : "[SystemToMonitor]"
	}

*/
