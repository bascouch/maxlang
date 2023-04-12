
/*
	 max minilang for scene def
*/

// set up inlets/outlets/assist strings
outlets = 6;
setinletassist(0,"match/pattern/test/modifier");

setoutletassist(0,"chaindef out");
setoutletassist(1,"chain names");
setoutletassist(2,"scene name");
setoutletassist(3,"events def");
setoutletassist(4,"is a match (int)");
setoutletassist(5,"COMMENTS output ");

// global varables and code
var vpattern = ".*";
var vmodifier = "i";
var vregexp = new RegExp(vpattern,vmodifier);


var SCENES = {};
var lastscene = "undefined";

var LINES_regexp = new RegExp("\\n");

var COMMENTS_regexp = new RegExp("#[^\\n]*\\n","gi");

var CHAIN_regexp = new RegExp("CHAIN ");
var CHAINNAME_regexp = new RegExp("CHAIN (.*)\\n","g");
var SCENE_regexp = new RegExp("SCENE (.*)\\n");

var SECTION_regexp = new RegExp("(chain.*)*","g");

var VAR_regexp = new RegExp("$(\S*)");

var EVENTS_regexp = new RegExp("EVENTS");

/*

réunit tous les éléments de la liste d'entrée en 1 seul string

match :

, ou \, 
	> ,

#SOMETHING 
	> Section of scenedef


$VAR remplacé par variable globale du même nom


Gestion d'erreur ?


*/


function scenedef(p)
{
	var c = p.replace(COMMENTS_regexp,"\n");
	var s = c.match(SCENE_regexp);
	
	// replace $VARIABLES
	var vars = VAR_regexp.exec(c);
	//TODO ...
	
	// split at EVENTS keyword
	var se = c.split(EVENTS_regexp);
	
	var r = se[0].split(CHAIN_regexp);
	

	
	var chainnames = c.match(CHAINNAME_regexp);
	
	//var r = s.match(SECTION_regexp);
	if (r && s) {
		// populate
		outlet(4,1); 	
		// CHAIN populate
		var scene_name = s[1];
		lastscene = scene_name;
		SCENES[scene_name] = [];

		var chain_count = r.length-1
		if (r.length>1) {
			var i;
			var substr = new Array(); 
			for (i=0;i<chain_count;i++) {
				SCENES[scene_name][i] = r[i+1];
				substr[i] = r[i+1];
			}
			//outlet(2,substr);
		}
		// output scene name
		outlet(2,scene_name)
		// outputs events on outlet 4
		if(se.length>1)
			outlet(3,se[1]); 
		
		// output chainnames
		for (cn in chainnames)
			outlet(1,chainnames[cn].replace('CHAIN ','').replace('\n',''));
		
		outlet(5,"maxlang : Scene loading Ok with chain numbers: " + String(chain_count));
	} else {
		outlet(4,0);
		outlet(5,"maxlang error : There was a problem laoding the scene. Check the syntax.");
	}
	
}

function getchain(s,n)
{
		
	var nn = SCENES[s].length;
	if( n<1 || n>nn )
	{
		outlet(5,"maxlang error : chain index out of bounds.");
	}
		
	outlet(0,SCENES[s][n-1])
}



function dump(scene)
{
	for ( c in SCENES[scene])
	{
		outlet(0,SCENES[scene][c])
	}
}

function dumplast()
{
	if( lastscene in SCENES)
		for ( c in SCENES[lastscene])
			outlet(0,SCENES[lastscene][c])
}

