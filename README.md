# phoxygen
A quick and dirty documentation tool for PHP code, written in Perl. Accepts doxygen-like tags in the PHP source code.

This can do less than Doxygen, but one thing it does better is that it parses PHP
function headers correctly and can document SQL CREATE TABLE statements.

## Usage

Run phoxygen in the root of the PHP document tree that you want to document. It will create a doc/ subdirectory therein
with two subdirectories:

 * doc/html/ with lots of HTML files, of which index.html contains the main overview;

 * doc/latex/ with index.tex, which you can feed into pdflatex to produce a beautiful PDF with the same information.

## Basic features in document blocks

phoxygen understands a lot of the same tags as Doxygen and PHPDoc. It is designed to work together with smart PHP editors 
such as PHPStorm, which also understand those tags.

It can document classes and functions like the other two if you precede the class or function declaration with a doc block.
A doc block is a standard C-style comment with two leading stars, like this: `/** ... docblock ... */` and can span multiple lines.

In addition to PHPDoc, phoxygen understands doxygen-like function argument documentation, which spares you some typing, like so:

```
/** 
 *  A function that does things.
 */
function myFunction($arg1, 		//!< description of arg1
                    $arg2)		//!< description of arg2
{
	...
}

```

This is equivalent to writing out a @param block for each of the arguments.

In addition to both PHPDoc and doxygen, you can also document SQL table definitions and REST APIs, which will be listed in 
separate blocks in the resulting documentation.

Any `/** ... */` in front of a line that contains the string `CREATE TABLE` is considered an SQL table documentation.

Any `/** ... */` in front of a line that matches the regular expression `^\s*\S+::(Get|Post|Put|Delete)\(["'"](.*)["'"],`
is considered a REST API. This is for frameworks like Slim that use those method calls to define REST API handlers.

