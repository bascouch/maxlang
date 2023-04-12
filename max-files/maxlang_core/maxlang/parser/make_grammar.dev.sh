pegjs --format globals --trace --allowed-start-rules valuestart maxlang-grammar.dev.pegjs
# replace being and end
sed -i '' 's/(function(root)/exports\.PARSER \= (function()/g' maxlang-grammar.dev.js
sed -i '' 's/root\.null \=/return/g' maxlang-grammar.dev.js
sed -i '' 's/})(this);/})();/g' maxlang-grammar.dev.js
