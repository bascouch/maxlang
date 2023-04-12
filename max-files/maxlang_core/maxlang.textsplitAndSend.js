
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
		var catch_param = false;
		var param = "unset"
		for(var k=0;k<linesplit.length; k++)
		{
			if( linesplit[k] != 'pink.msgs' )
			{
				if(linesplit[k] != ' ' )
				{
					if(catch_param == true)
					{
						param = linesplit[k];
						catch_param = false;
					}
						
					outmess.push(linesplit[k]);
					/*if(catch_param == true)
					{
						param = linesplit[k];
						catch_param == false;
						
					}
					else
					{
						
					}*/
					
						
				}
			}	
			else
				catch_param = true;
				
		}
		
		postln(outmess);
		sendname = 'pink.' + param + '.in'
		postln("Sending to "+ sendname);
		outlet(1,sendname);
		outlet(0,linesplit);
		
	}
}
