
/*
	  maxlang value parser
*/

// set up inlets/outlets/assist strings
outlets = 5;
setinletassist(0,"match/pattern/test/modifier");

setoutletassist(0,"chaindef out");
setoutletassist(1,"chain names");
setoutletassist(2,"scene name");
setoutletassist(3,"is a match (int)");
setoutletassist(4,"COMMENTS output ");

// load the PEGJS grammar
var req = require("maxlang-grammar");
var ONSTART_regexp = new RegExp("ONSTART");
var ONRELEASE_regexp = new RegExp("ONRELEASE");
var CUE_regexp = new RegExp("CUE");
var LINES_regexp = new RegExp("\\n");



Math.clip = function(number, min, max) {
  return Math.max(min, Math.min(number, max));
}

function filterEmptyLines(element) 
{
  return element != '';
}

var verbose = false;

var dictname = ""
var scenename = "";
SCENE_EVENTS = {};

if(jsarguments[1] != 0)
{
	dictname = jsarguments[1];
	//post(dictname);
}

function setdict(name)
{
	dictname=name;
}

function verbose(v)
{
	verbose = (v>0);
}

function scene(s)
{
	scenename= s;
}

function clear(s)
{
	if(s in SCENE_EVENTS)
		delete SCENE_EVENTS[s];
	var dict = new Dict(dictname);
	dict.remove(s);
}

function clearall()
{
	var dict = new Dict(dictname);
	dict.clear();
	SCENE_EVENTS = {};
}

function anything()
{
	var outdict = new Dict(dictname);
	var dict = new Dict();
	dict.clear();

	var inputs = arrayfromargs(messagename,arguments);
	// convert p as a joined string
	//post(inputs);
	input = inputs.join(' ');
	var cue = input.split(CUE_regexp);
	var cues = cue.slice(1);
	var onrelease = cue[0].split(ONRELEASE_regexp);
	var onrelease_ = onrelease.slice(1);
	
	onrelease_ = onrelease_[0].split('\n').filter(filterEmptyLines);
	
	var onstart = onrelease[0].split(ONSTART_regexp);
	var onstart_ = onstart.slice(1);
	onstart_ = onstart_[0].split('\n').filter(filterEmptyLines);
	
	if(verbose){
	post('INPUT: '+input.toSource());
	post();
	post('CUE: '+cues.toSource());
	post();
	post('ONRELEASE: '+onrelease_.toSource());
	post();
	post('ONSTART: '+onstart_.toSource());
	post();
	}

	
	var cuedict = {};
	var index = 1;
	for(cc in cues)
	{
		cuedict[index]=cues[cc].split('\n').filter(filterEmptyLines);
		index = index+1;
	}
	if(verbose){
	post('CUEDICT: '+cuedict.toSource());
	post();
	}
		//if(verbose) post('maxlang Return :'+JSON.stringify(entry)+'\n');

		//dict.parse('{"module":"pitchshift", "param":[{"pname":"name", "value":"shishi"}, {"pname":"transp", "value":{"modtor":"randi", "param":[{"pname":"name", "value":"gigilfo"}, {"pname":"freq", "value":0.1}, {"pname":"min", "value":-2400}, {"pname":"max", "value":2400}]}}, {"pname":"outs", "value":[1, 2, 7, 8]}]}');
		
	dict.set('onstart',onstart_);
	dict.set('onrelease',onrelease_);
	dict.setparse('cues',JSON.stringify(cuedict));
	
	
	SCENE_EVENTS[scenename] = dict.stringify();
	outdict.setparse(scenename,SCENE_EVENTS[scenename]);
	
	outlet(3,1);
	outlet(2,dict);
	//outlet(1,dictdef);
	outlet(0,outdict.stringify());

		



}
