
/*
	 SCENE poly
	get a valid chainid for each chaindef of each scene
*/

// set up inlets/outlets/assist strings
outlets = 6;
setinletassist(0,"match/pattern/test/modifier");


setoutletassist(0,"chain id");
setoutletassist(1,"chain name");
setoutletassist(2,"to poly dict name");
setoutletassist(3,"to poly dict num");
setoutletassist(4,"to dict scene chains");
setoutletassist(5,"COMMENTS output ");


// dict { chainname : chainid } 
var SCENES = {};

var chain_num = 64;
var chainpoly_array = [];
chainpoly_array[chain_num-1] = 0;
// busy array
fillArray(chainpoly_array,0);

/*


*/

function fillSceneChainDict(scene)
{
	
	if(scene in SCENES)
	{
		var l = [];
		for (chainname in SCENES[scene])
			l.push(SCENES[scene][chainname]);
		outlet(4,'remove',scene);
		outlet(4,'set',scene,l);
	}
	
	
}

function fillArray(array,val)
{
	for ( var i=0; i< array.length; i++ )
		array[i]=val;
}

function findFirstIndex(array,val)
{
	for ( var i=0; i< array.length; i++ )
	{
		if(array[i] == val )
			return i;
	}
	return -1;
}

function assign(scene,chainname)
{
	
	if(scene in SCENES)
	{
		
	}else
		SCENES[scene] = {};
	
	if (chainname in SCENES[scene]){
		// RELEASE CHAIN in SCENE
		// chainname el
		// chainid SCENES[scene][chainname]
		var chainid = SCENES[scene][chainname];	
		chainpoly_array[chainid-1] = 1;
		fillSceneChainDict(scene);
		outlet(2,'set',scene+'.'+chainname,chainid);
		outlet(3,'set',chainid,scene+'.'+chainname);
		outlet(1,chainname);
		outlet(0,chainid);	
	}else
	{
		// find first free voice
		var index = findFirstIndex(chainpoly_array,0);
		//post('CHAIN NEW',scene,chainname,index,'\n');
		if(index == -1 )
		{
			post('maxlang : ERROR no more free polychain voice (limit is 16)','\n')
			outlet(4,"maxlang : ERROR no more free polychain voice ");
		}else
		{
			
			var chainid = index+1;
			chainpoly_array[index] = 1;
			SCENES[scene][chainname] = chainid;
			fillSceneChainDict(scene);
			outlet(2,'set',scene+'.'+chainname,chainid);
			outlet(3,'set',chainid,scene+'.'+chainname);
			outlet(1,chainname);
			outlet(0,chainid);
		}
	}
}

function clear(scene)
{
	if (scene in SCENES)
	{
		for ( chainname in SCENES[scene])
		{
			var chainid = SCENES[scene][chainname];
			chainpoly_array[chainid-1] = 0;
			outlet(2,'remove',scene+'.'+chainname);
			outlet(3,'remove',chainid.toString());
		}
		outlet(4,'remove',scene);
		
	}
	SCENES[scene] = {};
	
}

function clearall()
{
	fillArray(chainpoly_array,0);
	SCENES = {};
	outlet(2,'clear');
	outlet(3,'clear');
	outlet(4,'clear');
}

function run(scene)
{
	
}

function release(scene)
{
	
}





