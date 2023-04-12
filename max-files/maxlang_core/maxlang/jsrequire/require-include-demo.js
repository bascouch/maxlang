 
// for require, the module must be defined by assigning anything
// the module wishes to export as properties of an "exports" property
// e.g. in the following module exports.foo = 37. This exports property
// is the return value from the require function. modules using 
// require are always scoped to prevent collisions in the top 
// level script (i.e. this one)

var reqmodule = require("require-example"); 
reqmodule.bar();
post("foo is "+reqmodule.foo+"\n");

post("-------------------------------------[1]\n");

// for include, we run in the scope of the current top level script object
// one can optionally can specify a second argument for scoping
// include is useful to evaluate any javascript, not just those 
// which conform to the Common JS module system conventions
// especially if one wishes all of the code to be added to the top 
// level script, or make use of top level scope for some reason

include("include-example"); 
var someval = "I'm dangerously top level";
included_function();

post("-------------------------------------[2]\n");

var scopeob = new Object();
scopeob.someval = "I'm a scopy scopebot";
include("include-example", scopeob); 
scopeob.included_function();

post("-------------------------------------[3]\n");


var req = require("parser"); 


post(req.PARSER.parse('3+7')+" -------------------------------------[2]\n");

