
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

Math.clip = function(number, min, max) {
  return Math.max(min, Math.min(number, max));
}

var verbose = false;

var dictname = ""

if(jsarguments[1] != 0)
{
	dictname = jsarguments[1];
}

function setdict(name)
{
	dictname=name;
}

function verbose(v)
{
	verbose = (v>0);
}

function anything()
{

	var dict = new Dict(dictname);
	dict.clear();

	var inputs = arrayfromargs(messagename,arguments);
	// convert p as a joined string
	//post(inputs);
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
		
		var dictdef = entry.toSource().slice(1,-1);
		//entry_dict.slice(1,-1);

		if(verbose) post('maxlang Return :'+JSON.stringify(entry)+'\n');

		//dict.parse('{"module":"pitchshift", "param":[{"pname":"name", "value":"shishi"}, {"pname":"transp", "value":{"modtor":"randi", "param":[{"pname":"name", "value":"gigilfo"}, {"pname":"freq", "value":0.1}, {"pname":"min", "value":-2400}, {"pname":"max", "value":2400}]}}, {"pname":"outs", "value":[1, 2, 7, 8]}]}');

		dict.parse(JSON.stringify(entry));
		outlet(3,1);
		outlet(2,dict);
		outlet(1,dictdef);
		outlet(0,dict.stringify());

		
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
