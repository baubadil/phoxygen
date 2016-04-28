# phoxygen
A quick and dirty documentation tool for PHP code, written in Perl. Accepts doxygen-like tags in the PHP source code.

This can do less than Doxygen, but one thing it does better is that it parses PHP
function headers correctly and can document SQL CREATE TABLE statements.

# Usage:

Run phoxygen in the root of the PHP document tree that you want to document. It will create a doc/ subdirectory therein
with two subdirectories:

 * doc/html/ with lots of HTML files, of which index.html contains the main overview;

 * doc/latex/ with index.tex, which you can feed into pdflatex to produce a beautiful PDF with the same information.

 



