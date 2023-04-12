// for include, we run in the scope of the current top level script object
// one can optionally can specify a second argument for scoping
// include is useful to evaluate any javascript, not just those 
// which conform to the Common JS module system conventions
// especially if one wishes all of the code to be added to the top 
// level script, or make use of top level scope for some reason

function included_function()
{
	post("I'm an included function\n");
	post("someval tells me \""+someval+"\"\n");
}