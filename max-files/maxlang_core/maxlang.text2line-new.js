
var command = jsarguments[1];

outlets=2;

var sendname = 'pink.semitone.in'

function postln(a)
{
	post(a)
	post()
}

function text()
{
	var inputs = arrayfromargs(messagename,arguments);
	inputs.shift();
	var input = inputs.join(' ');
	
	
	var inputlines = input.match(/[^\r\n]+/g);
	
	for(line in inputlines)
	{
		var outmess = [command];
		var linesplit = inputlines[line].split(" ");
		outlet(0,linesplit);
		
	}
}
