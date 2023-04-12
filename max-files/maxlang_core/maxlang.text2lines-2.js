var command = jsarguments[1];

outlets=2;

function text()
{
	var inputs = arrayfromargs(messagename,arguments);
	inputs.shift();
	var input = inputs.join(' ');
	
	var inputlines = input.match(/[^\r\n]+/g);
	for(line in inputlines)
	{
		var linesplit = inputlines[line].split(" ");
		var prefix = linesplit.shift()
		var param = linesplit.shift()
		linesplit.unshift(command)
		linesplit.unshift(param)
		outlet(1,prefix);
		outlet(0,linesplit);
		
	}
}
