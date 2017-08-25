# phoxygen
A documentation tool for PHP code. Accepts doxygen-like tags in the PHP source code.

This can do less than Doxygen, but one thing it does better is that it parses PHP
function headers correctly and can document SQL `CREATE TABLE` statements and REST APIs.

An early version of this was written in Perl, and that's still in the [perl-old/](perl-old) subdirectory,
but it was rewritten in C++11 in Nov 2016. Execution time went down from 2.5 seconds to
one second, and the code is a lot more readable now.

## Building

### Requirements for building

 * kBuild (the VirtualBox build system; http://trac.netlabs.org/kbuild). On Gentoo, it's `dev-util/kbuild`
   and should be installed already if you have VirtualBox installed as it's required for building it.
   On Debian it's `kbuild`.

 * libpcre for fast regular expressions. pcre.h must be in INCLUDE somewhere and libpcre must be
   somewhere where the linker can find it. On Gentoo it seems to be installed pretty much by default,
   on Debian you need `libpcre3-dev`.

### Build process

Run `kmk` in the root directory to build. `kmk` is the make utility of kBuild. `kmk BUILD_TYPE=debug` will
create a debug build instead of a release build.

There is no configuration presently, nor is there any install. After building, you will find the
executable under out/linux.amd64/{release|debug}/stage/bin/phoxygen.

## Usage

Run phoxygen in the root of the PHP document tree that you want to document. It will create a doc/html/ subdirectory
with lots of HTML files, of which index.html contains the main overview.

An earlier version also generated LaTeX sources for PDF generation but that's currently broken.

## Basic features in document blocks

phoxygen understands a lot of the same tags as Doxygen and PHPDoc. It is designed to work together with smart PHP editors
such as PHPStorm, which also understand those tags.

It can document classes and functions like the other two if you precede the class or function declaration with a doc block.
A doc block is a standard C-style comment with two leading stars, like this: `/** ... docblock ... */` and can span multiple lines.

In addition to PHPDoc, phoxygen understands doxygen-like **function argument** documentation, which spares you some typing, like so:

```php
/**
 *  A function that does things.
 */
function myFunction($arg1,      //!< description of arg1
                    $arg2)      //!< description of arg2
{
    ...
}
```

This is equivalent to writing out a `@param` block for each of the arguments.

In addition to both PHPDoc and doxygen, you can also document SQL table definitions and REST APIs, which will be listed in
separate blocks in the resulting documentation.

Any `/** ... */` in front of a line that contains the string `CREATE TABLE` is considered an **SQL table documentation.**

Any `/** ... */` in front of a line that matches the regular expression `^\s*\S+::(Get|Post|Put|Delete)\(["'"](.*)["'"],`
is considered a **REST API.** This is for frameworks like Slim that use those method calls to define REST API handlers.


## Formatting

Some limited HTML and Markdown formatting is supported: phoxygen allows the `<B>`, `<I>`, `<OL>`, `<UL>`, `<LI>` and `<CODE>`
HTML tags as well as backticks (`` ` ``) for code and three backticks for larger code blocks.

## Linking from other docblocks

Any mention of a documented class name in a doc block will automatically be linkified in other docblocks. For example,
if you have documented a class named `MyClass`, just writing `MyClass` in other doc blocks will turn such mentions into
a link to the `MyClass` documentation page. This is handy but you might not want to document a class name like `out`
because it would linkify that word everywhere.

Any occurrence of the form "GET|POST|PUT|DELETE /foo REST API" will be turned into a link to the corresponding REST API,
if it is known.

Table links are not automatically converted. To create a link to a documented table, use `\ref tablename` (with `tablename`
being the same that was used in the `CREATE TABLE` statement that you documented).

You can also link to functions with `\ref` provided that the link ends with `(`. It must contain a class name, otherwise
the current class (if any) is assumed. For example: `\ref MyClass::function()` or just `\ref function()` if you are in a block
of a class or method.


## Special pages

As with doxygen, you can create special pages as docblock comments that have `@page identifier heading...` in their first
line. This will store the docblock separately as a page, which can be referred with `\ref` from elsewhere. This allows you
to write longer explanations that do not pertain to an individual class or function, for example to explain how your
authentication works, and then reference that explanation from related classes, methods, tables and REST APIs. These
pages are listed under "Topics" in the HTML output.
