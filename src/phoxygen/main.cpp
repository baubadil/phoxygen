/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 *
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#define DEF_STRING_IMPLEMENTATION

#include "xwp/exec.h"
#include "xwp/debug.h"
#include "xwp/except.h"
#include "xwp/regex.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "phoxygen/phoxygen.h"
#include "phoxygen/htmlpage.h"

#include <sys/stat.h>


/***************************************************************************
 *
 *  Globals
 *
 **************************************************************************/

const string dirHTMLOut = "doc/html";
const string dirLatexOut = "doc/latex";

PMainPageComment g_pMainPage;

/***************************************************************************
 *
 *  Top-level functions called from main()
 *
 **************************************************************************/

void parseSources(StringVector &vFilenames)
{
    int c = 0;
    int cFiles = vFilenames.size();
    for (const auto &strInputFile : vFilenames)
    {
        ++c;

        cout << "Processing " + to_string(c) + "/" + to_string(cFiles) + ": " + strInputFile + "...\n";

        int lineno = 0;
        State state = State::INIT;                  // STATE_* constant
        int cLinesExamined = 0;

        string strCurrentComment = "";
        int linenoWhereCommentBegan;
        int linenoWhereCommentEnded = 0;
        PFunctionComment pLastFunction;
        PTableComment pLastTable;
        PClassComment pCurrentClass;
        PCommentBase pCurrent;                         // Current object. In the event of a \page, this gets set BEFORE the doccomment closes.
                                            // In the event of a class or function or the like, this gets set AFTER the doccomment closes.
        ifstream infile(strInputFile);
        string strCurrentLine;
        while (getline(infile, strCurrentLine))
        {
            ++lineno;

            static const Regex s_reCommentOpen(R"i____(^\s*\/\*\*)i____");
            static const Regex s_reEmptyLine(R"i____(^\s*$)i____");

            if (s_reCommentOpen.matches(strCurrentLine))
            {
                /*
                 * OPENING DOCCOMMENT
                 */
                // Ignore them if they have many asterisks.
                static const Regex s_reCommentIgnore(R"i____(^\s*\/\*\*\*\*\*\*\*)i____");
                if (s_reCommentIgnore.matches(strCurrentLine))
                    continue;

                // Ignore one-line PHPDoc variable declarations.
                if (strCurrentLine.find("@var") != string::npos)
                    continue;

                // Begin doxygen comment:
                state = State::IN_DOCCOMMENT;
                strCurrentComment = "";
                linenoWhereCommentBegan = lineno;
                Debug::Log(MAIN, "Found opening comment on line $lineno, state=$state");
            }
            else if (    (state == State::IN_DOCCOMMENT)
                      || (state == State::IN_DOCCOMMENT_PAGE)
                      || (state == State::IN_DOCCOMMENT_MAINPAGE)
                    )
            {
                static const Regex s_reCommentClose(R"i____(\*\/)i____");
                if (s_reCommentClose.matches(strCurrentLine))
                {
                    /*
                     * CLOSING DOCCOMMENT
                     */
//                     State oldstate = state;
                    if (    (state == State::IN_DOCCOMMENT_PAGE)
                         || (state == State::IN_DOCCOMMENT_MAINPAGE)
                       )
                    {
                        state = State::INIT;
                        pCurrent->append(strCurrentComment);
                    }
                    else
                    {
                        state = State::EXAMINE_NEXT_AFTER_DOCCOMMENT;
                        cLinesExamined = 0;
                    }
                    Debug::Log(MAIN, "Found closing comment on line $lineno, state=$oldstate->$state");
                    linenoWhereCommentEnded = lineno;
                }
                else
                {
                    static const Regex s_reCommentShrinkleft(R"i____(^\s+\* ?)i____");
                    string lineTemp = strCurrentLine;
                    s_reCommentShrinkleft.findReplace(lineTemp, "", false);

                    static const Regex s_reMainPage(R"i____(^\s*[\\@]mainpage\s+(.*))i____");
                    static const Regex s_rePage(R"i____(^\s*[\\@]page\s+([a-zA-Z0-9_-]+)\s+(.*))i____");

                    RegexMatches aMatches;
                    if (s_reMainPage.matches(lineTemp, aMatches))
                    {
                        const string &strPageTitle = aMatches.get(1);

                        state = State::IN_DOCCOMMENT_MAINPAGE;
                        Debug::Log(MAIN, "line $linenoWhereCommentBegan: found \\mainpage (state=$state)");
                        g_pMainPage = make_shared<MainPageComment>(strPageTitle,
                                                                   strInputFile,
                                                                   linenoWhereCommentBegan,
                                                                   linenoWhereCommentEnded);
                        pCurrent = g_pMainPage;
                    }
                    else if (s_rePage.matches(lineTemp, aMatches))
                    {
                        const string &pageID = aMatches.get(1);
                        const string &pageTitle = aMatches.get(2);

                        if (pageTitle.empty())
                            throw FSException("Incomplete \\page in comment beginning at line $lineno");

                        state = State::IN_DOCCOMMENT_PAGE;
                        Debug::Log(MAIN, "line $linenoWhereCommentBegan: found \\page $pageID with title \"$pageTitle\" (state=$state)");

                        auto p = PageComment::Make(pageID,
                                                   pageTitle,
                                                   strInputFile,
                                                   linenoWhereCommentBegan,
                                                   linenoWhereCommentEnded);
                        pCurrent = p;
                    }
                    else
                    {
                        strCurrentComment += lineTemp + "\n";
                        Debug::Log(MAIN, "Added line: " + lineTemp);
                    }
                }
            }
            else if (s_reEmptyLine.matches(strCurrentLine))     // Outside comments, ignore empty lines.
                continue;
            else if (state == State::EXAMINE_NEXT_AFTER_DOCCOMMENT)
            {
                // First non-empty line after non-page comment:
                static const Regex s_reClass(R"i____(^\s*((?:abstract\s+)?class)\s+(\S+).*)i____");
                static const Regex s_reInterface(R"i____(^\s*(interface)\s+(\S+))i____");
                static const Regex s_reFunction(R"i____((?:^|\s+)(?:public\s+|private\s+|protected\s+|static\s+)*function\s+([A-Za-z0-9_]+)\s*\((.*))i____");
                static const Regex s_reCreateTable(R"i____(^\s*CREATE\s+TABLE\s+(?:IF\s+NOT\s+EXISTS\s+)?([A-Za-z_]+).*)i____");
                static const Regex s_reRESTAPIDef(R"i____(^\s*\S+::(Get|Post|Put|Delete)\(["'"](.*)["'"],(.*))i____");

                RegexMatches aMatches;
                if (    (s_reClass.matches(strCurrentLine, aMatches))
                     || (s_reInterface.matches(strCurrentLine, aMatches))
                   )
                {
                    const string &keyword = aMatches.get(1);
                    const string &identifier = aMatches.get(2);

                    /*
                     * STORE CLASS
                     */
                    Debug::Enter(MAIN, "line " + to_string(linenoWhereCommentBegan) + ": found " + keyword  + " " + identifier);

                    auto p = ClassComment::Make(keyword,
                                                identifier,
                                                strCurrentComment,
                                                strInputFile,
                                                linenoWhereCommentBegan,
                                                linenoWhereCommentEnded);
                    pCurrent = p;
                    StringVector llParents;

                    static const Regex s_reImplements(R"i____(^\s*class\s+\S+\s+implements\s+(\S+))i____");
                    static const Regex s_reSplitComma(R"i____(\s*,\s*)i____");

                    if (s_reImplements.matches(strCurrentLine, aMatches))
                    {
                        const string &strImplements = aMatches.get(1);
                        StringVector sv;
                        s_reSplitComma.split(strImplements, sv);
                        for (const auto &s : sv)
                        {
                            p->addImplements(s);
                            llParents.push_back(s);
                        }
                    }

                    static const Regex s_reExtends(R"i____(^\s*(?:abstract\s+)?(?:class|interface)\s+\S+\s+extends\s+(\S+))i____");
                    if (s_reExtends.matches(strCurrentLine, aMatches))
                    {
                        const string &strExtends = aMatches.get(1);
                        StringVector sv;
                        s_reSplitComma.split(strExtends, sv);
                        for (const auto &s : sv)
                        {
                            p->addExtends(s);
                            llParents.push_back(s);
                        }
                    }

                    for (const auto &s : llParents)
                        p->addParent(s);

                    pCurrentClass = p;
                    state = State::INIT;

                    // my $exts = ($pCurrent->extends) ? ', extends "'.$pCurrent->extends.'"' : '';
                    // my $impl = ($pCurrent->implements) ? ', implements "'.$pCurrent->implements.'"' : '';
                    Debug::Leave();
                }
                else if (s_reFunction.matches(strCurrentLine, aMatches))
                {
                    string identifier = aMatches.get(1);
                    string afterOpeningBracket = aMatches.get(2);

                    /*
                     * STORE FUNCTION
                     */
                    Debug::Log(MAIN, "line " + to_string(linenoWhereCommentBegan) + ": found function " + identifier + ", afterOpeningBracket=" + afterOpeningBracket);

                    static const Regex s_reFunctionKeyword(R"i____(^\s*(.*function).*)i____");
                    s_reFunctionKeyword.matches(strCurrentLine, aMatches);
                    const string &keyword = aMatches.get(1);

                    auto p = make_shared<FunctionComment>(keyword,
                                                          identifier,
                                                          strCurrentComment,
                                                          strInputFile,
                                                          linenoWhereCommentBegan,
                                                          linenoWhereCommentEnded);
                    pLastFunction = p;
                    pCurrent = p;

                    if (pCurrentClass)
                    {
                        // Member function: two-way linking.
                        pLastFunction->setClass(pCurrentClass);
                        pCurrentClass->addMember(pLastFunction);
                    }

                    pLastFunction->processInputLine(afterOpeningBracket,
                                                    state);
                }
                else if (s_reCreateTable.matches(strCurrentLine, aMatches))
                {
                    const string &identifier = aMatches.get(1);
                    /*
                     * STORE TABLE
                     */
                    Debug::Log(MAIN, "line $linenoWhereCommentBegan: found table $identifier");

                    auto p = TableComment::Make(identifier,
                                                strCurrentComment,
                                                strInputFile,
                                                linenoWhereCommentBegan,
                                                linenoWhereCommentEnded);
                    pLastTable = p;
                    pCurrent = p;
                    state = State::IN_CREATE_TABLE;

                    pLastTable->processInputLine(strCurrentLine, state);
                }
                else if (s_reRESTAPIDef.matches(strCurrentLine, aMatches))
                {
                    /*
                     *  STORE REST API
                     */
                    const string &method = aMatches.get(1);     // mixed case!
                    const string &nameAndArgs = aMatches.get(2); // ) = /^\s*\S+::(Get|Post|Put|Delete)\(["'"](.*)["'"],/ )

                    static const Regex s_reRESTAPIArgs(R"i____(\/([^\/]+)(\/.*)?)i____");
                    RegexMatches aMatches2;
                    if (!s_reRESTAPIArgs.matches(nameAndArgs, aMatches2))
                        Debug::Warning("wonky REST API args \"" + nameAndArgs + "\"");
                    else
                    {
                        const string &name = aMatches2.get(1);
                        const string &args = (aMatches2.size() > 1) ? aMatches2.get(2) : "";
                        auto p = RESTComment::Make(method,
                                                   name,
                                                   args,
                                                   strCurrentComment,
                                                   strInputFile,
                                                   linenoWhereCommentBegan,
                                                   linenoWhereCommentEnded);
                        pCurrent = p;

                        Debug::Log(MAIN, "line " + to_string(linenoWhereCommentBegan) + ": found REST API " + p->getIdentifier());
                    }

                    state = State::INIT;

                    // $pLastTable->processInputLine($_, \$state);
                }
                else if (cLinesExamined < 2)
                {
                    ++cLinesExamined;
                }
                else
                {
                    Debug::Warning("don't know what to do with comment in " + strInputFile + " (lines " + to_string(linenoWhereCommentBegan) + "--" + to_string(linenoWhereCommentEnded) + ")");
                    state = State::INIT;
                }
            }
            else if (state == State::IN_FUNCTION_HEADER)
            {
                // Continuing function header:
                pLastFunction->processInputLine(strCurrentLine, state);
            }
            else if (state == State::IN_CREATE_TABLE)
            {
                pLastTable->processInputLine(strCurrentLine, state);
            }
        }
    }

    cout << c << " files processed.\n";
}

void writePages()
{
    /*
     *  MAIN PAGE
     */
    Debug::Enter(MAIN, "Writing main page");
    if (!g_pMainPage)
        g_pMainPage = make_shared<MainPageComment>("Missing \\mainpage (not yet written)", "", 0, 0);

    auto p = make_shared<HTMLPage>(dirHTMLOut,
                                   "index.html",
                                   g_pMainPage->getTitle(false),
                                   g_pMainPage->formatComment());
    Debug::Leave();

    /*
     *  PAGES
     */
    Debug::Enter(MAIN, "Writing pages");

    string strTitle = "Pages list";
    string htmlBody = "<h1>" + strTitle + "</h1>\n\n<ul>";

    // The pages are sorted by page ID, not page title, which is not very helpful to the user. So sort them by title first.
    vector<PPageComment> v;
    for (auto it : PageComment::GetAll())
        v.push_back(it.second);
    sort(v.begin(),
         v.end(),
         [](PPageComment p1, PPageComment p2)
         {
             return p1->getTitle(false) < p2->getTitle(false);
         });

    for (auto pPage : v)
        htmlBody += "<li>" + pPage->makeLink();

    htmlBody += "</ul>\n";

    p = make_shared<HTMLPage>(dirHTMLOut,
                              "index_pages.html",
                              strTitle,
                              htmlBody);

    for (auto pPage : v)
    {
        htmlBody = "<h1>" + pPage->getTitle(true) + "</h1>\n";

        string htmlThis = pPage->formatComment();
        htmlBody += htmlThis;

        p = make_shared<HTMLPage>(dirHTMLOut,
                                  "page_" + pPage->getIdentifier() + ".html",
                                  pPage->getTitle(false),
                                  htmlBody);
    }

    Debug::Leave();
}

void writeRESTAPIs()
{
    Debug::Enter(MAIN, "writing REST APIs");

    string strTitle = "REST APIs list";
    string htmlBody = "<h1>" + strTitle + "</h1>\n\n<ul>";

    // The pages are sorted by page ID, but we want to sort them by API name first and method second.
    vector<PRESTComment> v;
    for (auto it : RESTComment::GetAll())
        v.push_back(it.second);
    sort(v.begin(),
         v.end(),
         [](PRESTComment p1, PRESTComment p2)
         {
             // RESTComment has operator<.
             return *p1 < *p2;
         });

    for (auto pREST : v)
        htmlBody += "<li>" + pREST->makeLink();

    htmlBody += "</ul>\n";

    auto p = make_shared<HTMLPage>(dirHTMLOut,
                                   "index_restapis.html",
                                   strTitle,
                                   htmlBody);

    for (auto pREST : v)
    {
        htmlBody = "<h1>" + pREST->getTitle(true) + "</h1>\n";

        string htmlThis = pREST->formatComment();
        htmlBody += htmlThis;

        p = make_shared<HTMLPage>(dirHTMLOut,
                                  pREST->makeHREF(),
                                  pREST->getTitle(false),
                                  htmlBody);
    }

    Debug::Leave();
}

void writeTables()
{
    Debug::Enter(MAIN, "writing tables");

    string strTitle = "SQL tables list";
    string htmlBody = "<h1>" + strTitle + "</h1>\n\n<ul>";

    for (auto it : TableComment::GetAll())
    {
        // const string &strTable = it.first;
        auto pTable = it.second;
        htmlBody += "<li>" + pTable->makeLink();;
    }

    htmlBody += "</ul>\n";

    auto p = make_shared<HTMLPage>(dirHTMLOut,
                                   "index_tables.html",
                                   strTitle,
                                   htmlBody);

    for (auto it : TableComment::GetAll())
    {
        const string &strTable = it.first;
        auto pTable = it.second;

        htmlBody = "<h1>" + pTable->getTitle(true) + "</h1>\n";

        string htmlThis = pTable->formatComment();
        htmlBody += htmlThis;

        auto p = make_shared<HTMLPage>(dirHTMLOut,
                                       "table_" + strTable + ".html",
                                       pTable->getTitle(false),
                                       htmlBody);
    }

    Debug::Leave();
}

void writeClasses()
{
    Debug::Enter(MAIN, "Writing class list");

    string strTitle = "Class list";
    string htmlBody = "<h1>" + strTitle + "</h1>\n\n";
    string htmlThis = "<ul>";

    // Resolve children.
    StringSet stClassesWithBrokenParents;
    for (auto it : ClassComment::GetAll())
    {
        const string &strClass = it.first;
        auto pClass = it.second;

        for (const auto &strParent : pClass->getParents())
        {
            auto pParent = ClassComment::Find(strParent);
            if (pParent)
            {
                pParent->addChild(pClass);
                Debug::Log(MAIN, "adding child $class to parent $parent");
            }
            else
            {
                Debug::Warning("Ignoring unknown parent class \"" + strParent + "\" of class \"" + strClass + "\"");
                stClassesWithBrokenParents.insert(strClass);
            }
        }
    }

    // Loop through all classes which have NO PARENT and list children thereunder.
    for (auto it : ClassComment::GetAll())
    {
        const string &strClass = it.first;
        Debug::Enter(MAIN, "testing class " + strClass);
        auto pClass = it.second;
        size_t cParents = pClass->getParents().size();
        Debug::Log(MAIN, to_string(cParents) + " parents");
        if (!cParents || STL_EXISTS(stClassesWithBrokenParents, strClass))
            htmlThis += "<li>" + pClass->makeLink(strClass, NULL) + pClass->formatChildrenList() + "</li>\n";
        Debug::Leave();
    }

    htmlThis += "</ul>";

    htmlBody += htmlThis;
    auto p = make_shared<HTMLPage>(dirHTMLOut,
                                   "index_classes.html",
                                   strTitle,
                                   htmlBody);

    Debug::Leave();

    /*
     *  Now, one HTML file per class.
     */

    Debug::Enter(MAIN, "Writing class files");

    for (auto it : ClassComment::GetAll())
    {
        const string &strClass = it.first;
        auto pClass = it.second;

        htmlBody = "<h1>" + pClass->getTitle(true) + "</h1>\n";

        htmlThis = pClass->formatComment();
        htmlBody += htmlThis;

        htmlBody += "<h2>Hierarchy</h2>\n\n";
        htmlThis = "";

        auto pllParents = pClass->getParents();
        auto pllChildren = pClass->getChildren();
        if (pllParents.size())
        {
            for (const string &strParent : pllParents)
                htmlThis += "<a href=\"class_" + strParent + ".html\">" + strParent + "</a> &mdash; ";
        }

        if (    (pllParents.size())
             || (pllChildren.size())
           )
        {
            htmlThis += "<b>" + strClass + "</b>";
        }
        else
        {
            htmlThis += "<p>This class stands alone and has neither parents nor children.</p>";
        }

        if (pllChildren.size())
        {
            htmlThis += " &mdash; ";
            size_t c = 0;
            for (auto pChild : pllChildren)
            {
                if (c)
                    htmlThis += ", ";
                const string &strChild = pChild->getIdentifier();
                htmlThis += "<a href=\"class_" + strChild + ".html\">" + strChild + "</a>";
                ++c;
            }
        }

        htmlBody += htmlThis;

        auto pllMembers = pClass->getMembers();
        size_t cMembers = pllMembers.size();
        Debug::Log(MAIN, "Class " + strClass + " has " + to_string(cMembers) + " members");
        if (cMembers)
        {
            htmlBody += "<h2>" + to_string(cMembers) + " members</h2>\n\n";

            htmlThis = "<ul>\n";
            for (auto pMemberFunction : pllMembers)
            {
                const string &strIdentifier = pMemberFunction->getIdentifier();
                htmlThis += "<li><a href=\"#" + strIdentifier + "\">";
                htmlThis += pMemberFunction->formatFunction(false); // short
                htmlThis += "</a></li>\n";
            }
            htmlThis += "</ul>\n";

            htmlBody += htmlThis;

            htmlBody += "\n<h2>Details</h2>\n";
            htmlThis = "";

            for (auto pMemberFunction : pllMembers)
            {
                const string &strIdentifier = pMemberFunction->getIdentifier();
                htmlThis += "<dl><dt id=\"" + strIdentifier + "\">";
                htmlThis += pMemberFunction->formatFunction(true) + "</dt>\n";
                string htmlDesc = pMemberFunction->formatComment();
                htmlThis += "<dd>" + htmlDesc + "</dd></dl>";
                htmlThis += "\n";
            }

            htmlBody += htmlThis;
        }

        auto p = make_shared<HTMLPage>(dirHTMLOut,
                                       "class_" + strClass + ".html",
                                       pClass->getTitle(false),
                                       htmlBody);
    }

    Debug::Leave();
}


/***************************************************************************
 *
 *  Entry point
 *
 **************************************************************************/

int main(int argc, char **argv)
{
//     Regex reTest("---");
//     string strText("a---b---c");
//     StringVector v;
//     reTest.split(strText, v);
//     for (auto s : v)
//         cout << s << "\n";
//

//     Regex reTest("(.)cces(.)");
//     string strTest("The Access Access object");
//     reTest.findReplace(strTest, "$2cces$1", true);
//     cout << strTest << "\n";
//
//     Regex reTest2("FO*");
//     strTest = "FOOOOOO.FOOOOOOOOOOOOOOOO.TESTFOOOO.REST";
//     reTest2.findReplace(strTest, "BAR", true);
//     cout << strTest << "\n";

//     Regex reTest3("^\\s+\\* ?");
//     string strTest = " * \\page access_control Access control in Doreen";
//     reTest3.findReplace(strTest, "", false);
//     cout << strTest << "\n";

//     static const Regex re("&lt;(\\/?[bi])&gt;");
//     string strCopy = "Test <b>boldface</b> string";
//     stringReplace(strCopy, "&", "&amp;");
//     stringReplace(strCopy, "<", "&lt;");
//     stringReplace(strCopy, ">", "&gt;");
//     cout << strCopy << "\n";
//     re.findReplace(strCopy, "<$1>", true);
//     cout << strCopy << "\n";
//     exit(2);

    exec("mkdir -p " + dirHTMLOut);
    exec("mkdir -p " + dirLatexOut);

    StringVector vFilenames;

    struct stat s;
    for (int i = 1;
         i < argc;
         ++i)
    {
        string strArg(argv[i]);
        if (strArg[0] == '-')
        {
            if (strArg == "-v")
                g_flDebugSet = 0xFFFF;
        }
        else if (0 == ::stat(strArg.c_str(), &s))
            vFilenames.push_back(strArg);
        else
            throw FSException("don't know what to do with argument " + strArg);
    }

    if (vFilenames.empty())
    {
        string strFiles = exec("find -L . -name \"*.php\" -not -path \"./3rdparty/*\" -not -path \"./htdocs/3rdparty/*\"");
        for (const auto &strLine : explodeVector(strFiles, "\n"))
        {
            if (    (strLine.find("/3rdparty/") != string::npos)
                 || (strLine.find("/vendor/") != string::npos)
                 || (strLine.find("/node_modules/") != string::npos)
               )
                continue;
            vFilenames.push_back(strLine);
        }
    }

    parseSources(vFilenames);

    writePages();
    writeRESTAPIs();
    writeTables();
    writeClasses();
}

