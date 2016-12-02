/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 *
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#include "phoxygen/phoxygen.h"

RESTMap g_mapRESTComments;

RESTComment::RESTComment(const string &strMethod,
                         const string &strName,
                         const string &strArgs,
                         const string &strComment,
                         const string &strInputFile,
                         int linenoFirst,
                         int linenoLast)
    : CommentBase(Type::REST,
                  "",
                  "",       // identifier, see below
                  strComment,
                  strInputFile,
                  linenoFirst,
                  linenoLast,
                  "")
{
    _strMethod = strToUpper(strMethod);
    _identifier = RESTComment::MakeIdentifier(_strMethod, strName);
    makeTargets(_identifier);
    _strName = strName;
    _strArgs = strArgs;

}

/* static */
PRESTComment RESTComment::Make(const string &strMethod,
                               const string &strName,
                               const string &strArgs,
                               const string &strComment,
                               const string &strInputFile,
                               int linenoFirst,
                               int linenoLast)
{
    class Derived : public RESTComment { public: Derived(const string &strMethod,
                               const string &strName,
                               const string &strArgs,
                               const string &strComment,
                               const string &strInputFile,
                               int linenoFirst,
                               int linenoLast) : RESTComment(strMethod, strName, strArgs, strComment, strInputFile, linenoFirst, linenoLast) {}
    };
    auto p = make_shared<Derived>(strMethod,
                                  strName,
                                  strArgs,
                                  strComment,
                                  strInputFile,
                                  linenoFirst,
                                  linenoLast);
    g_mapRESTComments[p->getIdentifier()] = p;
    return p;
}

/* virtual */
string RESTComment::getTitle(OutputMode mode) /* override */
{
    switch (mode)
    {
        case OutputMode::PLAINTEXT:
            return _strMethod + " /api/" + _strName + " REST API";
        break;

        case OutputMode::HTML:
            return "<code>" + _strMethod + " /api/" + _strName + "</code> REST API";

        case OutputMode::LATEX:
        break;
    }

    // Latex
    return "\\texttt{" + _strMethod + " /api/" + _strName + "} REST API";
}

/* static */
const RESTMap& RESTComment::GetAll()
{
    return g_mapRESTComments;
}

/* static */
PRESTComment RESTComment::Find(const string &strIdentifier)
{
    auto it = g_mapRESTComments.find(strIdentifier);
    if (it != g_mapRESTComments.end())
        return it->second;

    return NULL;
}

