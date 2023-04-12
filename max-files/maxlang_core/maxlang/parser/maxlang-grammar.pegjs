{
  function extractOptional(optional, index) {
    return optional ? optional[index] : null;
  }

  function extractList(list, index) {
    return list.map(function(element) { return element[index]; });
  }


  function extractListFlatObj(list, index) {
    var ll = list.map(function(element) { return element[index]; });
    //post(ll.toSource()+'\n');
    var obj = {};
    for (var el in ll)
      for (var attrname in ll[el]) { obj[attrname] = ll[el][attrname]; }
    return obj;
  }

  function buildList(head, tail, index) {
    return [head].concat(extractList(tail, index))
      .filter(function(element) { return element !== null; });
  }

  function buildExpression(head, tail) {
    return tail.reduce(function(result, element) {
      return {
        type: "Expression",
        operator: element[0],
        left: result,
        right: element[1]
      };
    }, head);
  }
}



start 'start rule'
  = moddef

moddef 'module definition'
  = modname:symbol modparams:(_ parameter)* _ { return {module: modname, param: extractListFlatObj(modparams,1)} }

parameter 'parameter'
  = pname:symbol _ [=] _ value:value { var o = Object(); o[pname]=value; return o}

value 'value'
  =   float
  / integer
  / modtor
  / symbol
  / list

valuestart 'valuestart'
  = value:value {return {value: value}}

list 'list'
  =  "[" _ list:((float / integer / modtor/ symbol  ) _ ","* _ )* _ "]"
  { return extractList(list,0) }

modtor 'modulator'
  = name:symbol _ "(" _ parameters:( _ parameter _ ','?)* _ ")" {return {modtor: name, param: extractListFlatObj(parameters,1)}}

symbol 'symbol'
  = literal:[0-9a-zA-Z\-\.\_#&]+  {return text();}//literal.join('')}

integer 'integer'
  = sign:[+-]? digits:[0-9]+ !symbol { return parseInt(text(), 10); }

float 'float'
  = sign:[+-]? digits:[0-9\\.]+ !symbol { return parseFloat(text(), 10); }

_ 'whitespace'
  = [ \t\n\r]*

eventdef 'eventdef'
    = _ 'ONSTART' whole:.* 'ONRELEASE' rest:.* {return whole;}
  //= _ 'ONSTART' onstart:([.]*) 'ONRELEASE' onrelease:([.]*) cues:('CUE' ([.]*))+ {return {onstart: onstart, onrelease: onrelease, cues: cues}}
