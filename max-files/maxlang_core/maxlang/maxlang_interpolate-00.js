
/*
	  maxlang value parser
*/

// set up inlets/outlets/assist strings
inlets = 2;
outlets = 4;
setinletassist(0,"setA, setB, merge");

setoutletassist(0,"merged dict");
setoutletassist(1,"dict A");
setoutletassist(2,"dict B");
setoutletassist(3,"dump");


// load the PEGJS grammar
var req = require("maxlang-grammar");

Math.clip = function(number, min, max) {
  return Math.max(min, Math.min(number, max));
}

function post_stringify(input)
{
	post(JSON.stringify(input));
}

function postln(input)
{
	post(input)
	post()
}

var verbose = false;

var dict;
var dictA;
var dictB;

var interpol_modtor = {modtor:"input", param:{in:"interpolate00", min:0, max:1}}


var dictname = ""
var dictnameA = ""
var dictnameB = ""

if(jsarguments[1] != 0)
{
	dictname = jsarguments[1];
	dictnameA = dictname + 'A'
	dictnameB = dictname + 'B'
}

function setdict(name)
{
	dictname=name;
}

function verbose(v)
{
	verbose = (v>0);
}


function setA()
{
	dictA = new Dict(dictnameA);
	var inputs = arrayfromargs(messagename,arguments);
		// convert p as a joined string
	//post(inputs);
	inputs.shift();
	input = inputs.join(' ');
	//post('INPUT: '+input.toSource());
	try {
    var entry = req.PARSER.parse(input,{startRule : 'valuestart'});
 	} catch (err) {
		// js: maxlang error: message,expected,found,location,name

    	if (!err.hasOwnProperty('location')) throw(err);
		//post('maxlang error: '+err.message+'\n');
    	// Slice `text` with a little context before and after the error offset
		var start = Math.clip(err.location.start.offset-10,0,input.length)
		var end = Math.clip(err.location.end.offset+10,0,input.length)
    	error('maxlang '+ err.name + ' : at ...'+ input.slice(start,
       end).replace(/\r/g, '\\r')+'...  : '+ ' ' +err.message+'\n');
 	}

	//var r = s.match(SECTION_regexp);
	if (entry) {
		// populate
		
		var dictdef = entry.value.toSource().slice(1,-1);
		//entry_dict.slice(1,-1);

		if(verbose) post('maxlang Return :'+JSON.stringify(entry.value)+'\n');

		//dict.parse('{"module":"pitchshift", "param":[{"pname":"name", "value":"shishi"}, {"pname":"transp", "value":{"modtor":"randi", "param":[{"pname":"name", "value":"gigilfo"}, {"pname":"freq", "value":0.1}, {"pname":"min", "value":-2400}, {"pname":"max", "value":2400}]}}, {"pname":"outs", "value":[1, 2, 7, 8]}]}');

		dictA.parse(JSON.stringify(entry.value));
		/*outlet(3,1);
		outlet(2,dictA);
		outlet(1,dictdef);
		outlet(0,dictA.stringify());*/

		
		/*
		var chain_count = entry.length
		if (entry.length>0) {
			var i;
			var substr = new Array();
			for (i=0;i<chain_count;i++) {
				substr[i] = entry[i];
			}
			outlet(2,substr);
		}
		*/
		// output scene name


	} else {
		outlet(4,0);
	}
	
}


function setB()
{
	dictB = new Dict(dictnameB);
	var inputs = arrayfromargs(messagename,arguments);
		// convert p as a joined string
	//post(inputs);
	inputs.shift();
	input = inputs.join(' ');
	//post('INPUT: '+input.toSource());
	try {
    var entry = req.PARSER.parse(input,{startRule : 'valuestart'});
 	} catch (err) {
		// js: maxlang error: message,expected,found,location,name

    	if (!err.hasOwnProperty('location')) throw(err);
		//post('maxlang error: '+err.message+'\n');
    	// Slice `text` with a little context before and after the error offset
		var start = Math.clip(err.location.start.offset-10,0,input.length)
		var end = Math.clip(err.location.end.offset+10,0,input.length)
    	error('maxlang '+ err.name + ' : at ...'+ input.slice(start,
       end).replace(/\r/g, '\\r')+'...  : '+ ' ' +err.message+'\n');
 	}

	//var r = s.match(SECTION_regexp);
	if (entry) {
		// populate
		
		var dictdef = entry.value.toSource().slice(1,-1);
		//entry_dict.slice(1,-1);

		if(verbose) post('maxlang Return :'+JSON.stringify(entry.value)+'\n');

		//dict.parse('{"module":"pitchshift", "param":[{"pname":"name", "value":"shishi"}, {"pname":"transp", "value":{"modtor":"randi", "param":[{"pname":"name", "value":"gigilfo"}, {"pname":"freq", "value":0.1}, {"pname":"min", "value":-2400}, {"pname":"max", "value":2400}]}}, {"pname":"outs", "value":[1, 2, 7, 8]}]}');

		dictB.parse(JSON.stringify(entry.value));
		outlet(3,1);
		outlet(2,dictB);
		outlet(1,dictdef);
		outlet(0,dictB.stringify());

		
		/*
		var chain_count = entry.length
		if (entry.length>0) {
			var i;
			var substr = new Array();
			for (i=0;i<chain_count;i++) {
				substr[i] = entry[i];
			}
			outlet(2,substr);
		}
		*/
		// output scene name


	} else {
		outlet(3,0);
	}
	
}



function bang()
{
	post()
}

function mergefun_(_dictA,_dictB)
{
	
	// getkeys() will return an array of strings, each string being a key for our dict
	var keys = _dictA.getkeys();
	
	// access the name of a dict object as a property of the dict object
	var name = _dictA.name;

	post_info(name, keys);
	
	for (key in keys) {
		var key_A = keys[key];

		var val_A = _dictA.get(key_A);
		var val_B = _dictB.get(key_A);
		if( val_A == val_B )
		{
			post("EQUAL " + key_A + " " + val_A );
			post();
			//_dictA.parse(key_a,)
			//_dictA[key_a] = val_a;
		}
		else
		{
			post("øøøø " + key_A + " " + val_A );
			post();
			//_dictA[key_a] =  {"____gotyou":val_a};
			//_dictA.parse(key_a,{"____gotyou":val_a})
			
		}
		
	  //keyconsole.log(`${key}: ${value}`);
	}
	
	return _dictA;
}
 

function mergefun__(_dictA,_dictB)
{
	
	// post("entering mergefun with :")
	// post_stringify({"DICTA":_dictA})
	// post_stringify({"DICTB":_dictB})
	
	for (key in _dictA) {
		var val_A = _dictA[key];
		var val_B = _dictB[key];
		
		post("		...TESTING key:" + key + " val: " + val_A)
		post()
		var ret = {};
		
		if( val_A == val_B )
		{
			//post("EQ.     " + key + " " + val_A );
			post( typeof val_A );
			//_dictA.parse(key_a,)
			//_dictA[key_a] = val_a;
			if(  typeof val_A === 'object' && val_A !== null )
			{
				post("		recursing ...")
				post();
				ret = {'COUCOU' : "&&&&&&&&&&"}
				//ret = mergefun(val_A,val_B)
				// post("return ing")
				// post();
				// post(ret);
				// post();
				
			}
			else
			{
				//var ret = mergefun(val_A,val_B)
				postln("EQ.     " + key + " " + val_A );
				ret[key] = val_A;
				
			}
			
		}
		else
		{
			postln("NOTEQ.  " + key + " " + val_A );
			
			if(typeof val_A === 'object')
			{
				postln(" BOUUUUUH " +JSON.stringify(val_A));
				
			}
			
			ret = {'modtor' : 'xfade', 'params' : {'a':val_A, 'b':val_B }};
			//_dictA[key_a] =  {"____gotyou":val_a};
			//_dictA.parse(key_a,{"____gotyou":val_a})
		}
		
		_dictA[key] = ret;
		
	}
	
	return _dictA;
}



function mergemodtorfun(_dictA,_dictB)
{
	
	// post("entering mergefun with :")
	// post_stringify({"DICTA":_dictA})
	// post_stringify({"DICTB":_dictB})
	
	var ret = {};
	var modtor_ckecked = false;
	for(k in _dictA)
	{
		if( k ==='modtor')
			modtor_ckecked= true;
	}
	
	if(! modtor_ckecked)
	{
		// atomic value
		
		//check equality
		postln("			...atomic : " + _dictA)
		if(_dictA === _dictB || Array.isArray(_dictA))
		{
			ret = _dictA;
		}
		else
		{
			ret = {'modtor' : 'xfade', 'param' : {'a': _dictA, 'b': _dictB, 'fade': interpol_modtor }};
		}
		
	}
	else if( _dictA['modtor'] === _dictB['modtor'] )
	{
		postln("		... same modtor : " + _dictA['modtor'])
		var param_A = _dictA['param'];
		var param_B = _dictB['param'];
		
		var retparam = param_A;
		
		for (pkA in param_A) {
			var value_A = param_A[pkA]
			var value_B = (param_B[pkA] == undefined)? param_A[pkA] : param_B[pkA];
			retparam[pkA] = mergemodtorfun(value_A,value_B)
		}
		
		ret = ret = {'modtor' : _dictA['modtor'], 'param' : retparam};
		
		
	}else
	{
		postln("		... diff modtor A : " + _dictA['modtor'] )
		postln("						B : " + _dictB['modtor'])
		ret = {'modtor' : 'xfade', 'params' : {'a': _dictA, 'b': _dictB, 'fade': interpol_modtor }};
	}
	
	
	
	
	return ret;
}


function merge()
{
	var success_flag = false;
	
	var dict = new Dict(dictname);
	var dictjsonA = JSON.parse(dictA.stringify());
	var dictjsonB = JSON.parse(dictB.stringify());
	//var dictB = new Dict(dictnameB);
	dict.clear();
	
	//dict = dictA;
	
	
	
	// traverse and compare each trees 
	/*post("traversing with dictA :")
	post();
	post(dictjsonA);
	post();
*/
	var dictjson = mergemodtorfun(dictjsonA,dictjsonB)
	postln("DIRECT OUTPUT of mergefun");
	postln(JSON.stringify(dictjson))
	
	
	dict.setparse("merged",JSON.stringify(dictjson))
	//dict.parse(mergefun(dictjsonA,dictjsonB));
	post("dict"+ dictname+ "  set !! ")
	post();
	
	
	
	outlet(3,1);
	outlet(2,dict);
	//outlet(1,dictdef);
	outlet(0,dict.stringify());


}
