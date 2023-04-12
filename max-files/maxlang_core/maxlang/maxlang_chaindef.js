
/*
	 max minilang for CHAIN def
*/

// set up inlets/outlets/assist strings
outlets = 4;
setinletassist(0,"match/pattern/test/modifier");
setoutletassist(2,"is a match (int)");
setoutletassist(1,"chain name");
setoutletassist(0,"chain elements");

setoutletassist(3,"COMMENTS output ");


var CHAIN_NAME =[]
var CHAINS = {};
var SCENE = 'scene'

var LINES_regexp = new RegExp("\\n");


/*


*/

function filterEmptyLines(element) 
{
  return element != '';
}

function scene(s)
{
	SCENE = s;
}

function clearall()
{
	CHAIN_NAME =[]
	CHAINS = {};
	SCENE = 'scene'
}

function chaindef(p)
{
	var chains = p.split(LINES_regexp).filter(filterEmptyLines);
	cname = chains[0];
	CHAINS[cname]=chains;
	
}

function dump()
{	
	for( c in CHAINS )
	{
		outlet(1,SCENE+'.'+c);
		var elements = CHAINS[c];
		//elements.shift();
		for ( var i=1; i<elements.length; i++ ){
			if(elements[i] != '\n')
				outlet(0,elements[i]);
		}
		
	}
		
	
}




