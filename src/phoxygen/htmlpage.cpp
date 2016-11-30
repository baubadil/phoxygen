/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 *
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#include "phoxygen/htmlpage.h"

#include "xwp/stringhelp.h"

#include <iostream>
#include <fstream>
using namespace std;

/* static */
void HTMLWriter::Write(const string &dirHTMLOut,
                       const string &strFilename,
                       const string &strTitleWithoutHTML,       //!< in: title string for within HTML "title" element
                       const string &strBody)
{
    string strFullPath = makePath(dirHTMLOut, strFilename);
    ofstream myfile;
    myfile.open(strFullPath);
    myfile << "<html>\n";
    myfile << "<head>\n";
    myfile << "<meta charset=\"UTF-8\">\n";
    myfile << "<title>" << strTitleWithoutHTML << " &mdash; Doreen documentation</title>\n";
    myfile << "<style>\n";
    myfile << ".functable\n";
    myfile << "{\n";
    myfile << "    display: inline;\n";
    myfile << "    border-collapse: collapse;\n";
    myfile << "    vertical-align: top;\n";
    myfile << "}\n";
    myfile << ".functable td\n";
    myfile << "{\n";
    myfile << "    vertical-align: top;\n";
    myfile << "}\n";
    myfile << "  </style>\n";
    myfile << "</head>\n";
    myfile << "<body>\n";
    myfile << "<a href=\"index.html\">Home</a> &mdash;\n";
    myfile << "<a href=\"index_pages.html\">Topics</a> &mdash;\n";
    myfile << "<a href=\"index_restapis.html\">REST APIs</a> &mdash;\n";
    myfile << "<a href=\"index_classes.html\">Classes</a> &mdash;\n";
    myfile << "<a href=\"index_tables.html\">Tables</a>\n";
    myfile << "<hr>\n";
    myfile << strBody << "\n";
    myfile << "</body>\n";
    myfile << "</html>\n";
    myfile.close();
}

struct LatexWriter::Impl
{
    ofstream myfile;
};

LatexWriter::LatexWriter(const string &dirLatexOut)
    : _pImpl(new Impl())
{
    _pImpl->myfile.open(makePath(dirLatexOut, "doreen.tex"));
}

void LatexWriter::writeHeader(const string &strTitle)
{
    append(R"i____(
\documentclass[oneside,a4paper,10pt,DIV10]{scrbook}
\usepackage{ucs}
\usepackage[utf8x]{inputenc}
\usepackage[T1]{fontenc}
\usepackage[pdftex,
            a4paper,
            bookmarksnumbered,
            bookmarksopen=true,
            bookmarksopenlevel=0,
            colorlinks=true,
            hyperfootnotes=false,
            linkcolor=blue,
            plainpages=false,
            pdfpagelabels,
            urlcolor=green
  ]{hyperref}
\usepackage{underscore}

% Fonts
% \usepackage{gentium}
% \usepackage{libertine}
\usepackage{paratype}
\usepackage{helvet}
\usepackage{alltt}
\usepackage[scaled]{beramono}

% \renewcommand*\familydefault{\ttdefault}

\newcommand\mytitle{)i____" + strTitle + R"i____(}

\title{\mytitle}
\hypersetup{pdftitle=\mytitle}

\begin{document}
\maketitle
\tableofcontents

)i____");
}

LatexWriter::~LatexWriter()
{
    append("\n\\end{document}\n");
    _pImpl->myfile.close();
    delete _pImpl;
}

void LatexWriter::append(const string &str)
{
    _pImpl->myfile << str;
}
