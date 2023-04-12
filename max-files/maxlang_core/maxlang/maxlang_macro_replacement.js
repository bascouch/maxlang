
/*
	 maxlang macro evaluation
	@rand()
	@choice()
	@nrand()
	@nchoice()
*/

// set up inlets/outlets/assist strings
outlets = 1;
setinletassist(0,"input message");

setoutletassist(0,"output message");


function choose(choices) {
  var index = Math.floor(Math.random() * choices.length);
  return choices[index];
}


function macro_replace(match, macro, macro_args, shift, whole)
{
  	// rand choice or nrand nchoice

	n_args = arguments.length;

	arg_splitted = macro_args.split(" ")
	arg_splitted_len = arg_splitted.length;

	if(macro == "rand")
	{
		// arg1: min
		// arg2: max
		// arg3: curve
		var min = (arg_splitted_len>0)? Number(arg_splitted[0]) : 0
		var max = (arg_splitted_len>1)? Number(arg_splitted[1]) : 1
		var range = max - min
		var curve_e = (arg_splitted_len>2)? Math.exp(Number(arg_splitted[2])) : 1

		var val = min + range * Math.pow(Math.random(),curve_e)

		return val.toFixed(10)
	}

  if(macro == "nrand")
	{
    // arg1: n
		// arg2: min
		// arg3: max
		// arg4: curve
    var n = (arg_splitted_len>0)? Number(arg_splitted[0]) : 1
		var min = (arg_splitted_len>1)? Number(arg_splitted[1]) : 0
		var max = (arg_splitted_len>2)? Number(arg_splitted[2]) : 1
		var range = max - min
		var curve_e = (arg_splitted_len>3)? Math.exp(Number(arg_splitted[3])) : 1

    var val = min + range * Math.pow(Math.random(),curve_e)
 	var out = val
		for(var i=1; i<n; i++)
    {
      val = min + range * Math.pow(Math.random(),curve_e)
      out += " "+val.toFixed(10)
    }

		return out
	}

	if(macro == "choice")
	{
    // argn: vals
		if(arg_splitted_len == 0)
			return 0

		return choose(arg_splitted)
	}

  if(macro == "nchoice")
  {
    // arg1: n
    // argn: vals
    if(arg_splitted_len == 0)
			return 0
    var n = (arg_splitted_len>0)? Number(arg_splitted[0]) : 1
    arg_splitted.shift()

    var out = choose(arg_splitted)
    for(var i=1; i<n; i++)
    {
      out += " "+choose(arg_splitted)
    }

    return out
  }

  	return 0;
}

/* just a test */
if(false)
{
	var nouvelleChaine = 'toto titi @maxlang() @choice(caca kiki cucu concon 4 65 672 636) @rand(30 1)'.replace(/@([^\\(\s]*)\(([^\)]*)\)/g, macro_replace);

	post(nouvelleChaine); // abc - 12345 - #$*%
	post()
}


function anything(a)
{
	var a = arrayfromargs(messagename,arguments);
	var p = a.join(" ")
	var l = p.replace(/@([^\\(\s]*)\(([^\)]*)\)/g, macro_replace)
	outlet(0, l )
}
