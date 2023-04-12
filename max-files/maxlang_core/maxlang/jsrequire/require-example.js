
// for require, the module must be defined by assigning anything
// the module wishes to export as properties of an "exports" property
// This exports property is the return value from the require function. 
// modules included using require are always scoped to prevent collisions 
// in the top level script. Currently we don't support any of the other
// typcial node.js module gobal properties like module.id, module.loaded, etc.

exports.foo = 37;
exports.bar = function () {
	post("I'm executing bar\n");
}




