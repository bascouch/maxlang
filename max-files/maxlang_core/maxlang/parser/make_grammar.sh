pegjs --format globals --trace --allowed-start-rules start,valuestart,eventdef maxlang-grammar.pegjs
# replace being and end
sed -i '' 's/(function(root)/exports\.PARSER \= (function()/g' maxlang-grammar.js
sed -i '' 's/root\.null \=/return/g' maxlang-grammar.js
sed -i '' 's/})(this);/})();/g' maxlang-grammar.js
