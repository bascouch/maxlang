/*

maxlang scene folder reader and writer
snapshot writer
backup writer


*/

outlets = 7;
setoutletassist(6,"folder path");
setoutletassist(5,"number of files in folder");
setoutletassist(4,"file size (in bytes)");
setoutletassist(3,"file type");
setoutletassist(2,"name");
setoutletassist(1,"index");
setoutletassist(0,"content");

var WORKINGDIR = "";
var SNAPGDIR = "";
var BACKUPDIR = "";

var SCENES_LOADED = {};

function workingdir(d)
{
	WORKINGDIR = d;
	SNAPGDIR = d+'/snapshots';
	BACKUPDIR = d+'/backups';
	// make snapshots and backups folder
	
}

function reload()
{
	SCENES_LOADED = {};
	var f = new Folder(WORKINGDIR);
	var index = 1;
	f.typelist = ["TEXT"]
	outlet(6,f.pathname);
	outlet(5,f.count);
	f.reset();
	while (!f.end) {
		var thefile = new File(f.pathname + "/" + f.filename);
		if (thefile.isopen) {
			var content = thefile.readstring(1000000);
			SCENES_LOADED[index]=content;
			
			outlet(4,thefile.eof);
			outlet(2,f.filename);
			outlet(1,index);
			outlet(0,content);
			thefile.close();
			index=index+1;
		} else {
			outlet(4,0);
		}

		
		f.next();
	}
	f.close();
	
}

function reloadfile(s)
{
	var f = new Folder(WORKINGDIR);
	var index = 1;
	f.typelist = ["TEXT"]
	outlet(6,f.pathname);
	outlet(5,f.count);
	f.reset();
	while (!f.end) {
		var thefile = new File(f.pathname + "/" + f.filename);
		if (thefile.isopen) {
			var content = thefile.readstring(1000000);
			if(f.filename === s)
			{
				SCENES_LOADED[index]=content;
			
				outlet(4,thefile.eof);
				outlet(2,f.filename);
				outlet(1,index);
				outlet(0,content);
			}
			thefile.close();
			index=index+1;
		} else {
			outlet(4,0);
		}

		
		f.next();
	}
	f.close();
	
}

function scenesel(s)
{
	if(s in SCENES_LOADED)
	{
			outlet(0,SCENES_LOADED[s]);
	}
	
}
