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

HTMLPage::HTMLPage(const string &dirHTMLOut,
                   const string &strFilename,
                   const string &strTitle,
                   const string &strBody)
{
    string strFullPath = makePath(dirHTMLOut, strFilename);
    ofstream myfile;
    myfile.open(strFullPath);
    myfile << "<html>\n";
    myfile << "<head>\n";
    myfile << "<meta charset=\"UTF-8\">\n";
    myfile << "<title>$title &mdash; Doreen documentation</title>\n";
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

