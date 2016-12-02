/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 *
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#ifndef PHOXYGEN_H
#define PHOXYGEN_H

#include "xwp/basetypes.h"
#include "xwp/stringhelp.h"
#include "xwp/debug.h"
#include "xwp/regex.h"

#include "phoxygen/formatter.h"

#include <memory>
#include <map>

using namespace XWP;

const DebugFlag MAIN        = (1 <<  1);
const DebugFlag FORMAT      = (1 <<  2);

enum class State
{
    INIT,
    IN_DOCCOMMENT,
    IN_DOCCOMMENT_PAGE,
    IN_DOCCOMMENT_MAINPAGE,
    EXAMINE_NEXT_AFTER_DOCCOMMENT,
    IN_FUNCTION_HEADER,
    IN_CREATE_TABLE,
};

class CommentBase;
typedef shared_ptr<CommentBase> PCommentBase;

class ClassComment;
typedef shared_ptr<ClassComment> PClassComment;
typedef vector<PClassComment> ClassesVector;
typedef map<string, PClassComment> ClassesMap;

class FunctionComment;
typedef shared_ptr<FunctionComment> PFunctionComment;
typedef vector<PFunctionComment> FunctionsVector;
typedef map<string, PFunctionComment> FunctionsMap;

class TableComment;
typedef shared_ptr<TableComment> PTableComment;
typedef map<string, PTableComment> TablesMap;

class PageComment;
typedef shared_ptr<PageComment> PPageComment;
typedef map<string, PPageComment> PagesMap;

class MainPageComment;
typedef shared_ptr<MainPageComment> PMainPageComment;


/***************************************************************************
 *
 *  DocComment
 *
 **************************************************************************/

class CommentBase
{
    static const string s_strUnknown;

protected:
    enum class Type
    {
        PAGE,
        MAINPAGE,
        CLASS,
        FUNCTION,
        TABLE,
        REST
    };

    Type    _type;
    string  _keyword;
    string  _identifier;
    string  _comment;
    string  _file;
    int     _linenoFirst;
    int     _linenoLast;

    string  _strTargetHTML,
            _strTargetLaTeX;

    CommentBase(Type theType,
                const string &strKeyword,
                const string &strIdentifier,
                const string &strComment,
                const string &strFile,
                int linenoFirst,
                int linenoLast,
                const string &strTargetBase);

    void makeTargets(const string &strTargetBase);

public:
    Type getType() const
    {
        return _type;
    }

    const string& getIdentifier() const
    {
        return _identifier;
    }

    const string getTarget(FormatterBase &fmt)
    {
        return (fmt.getMode() == OutputMode::HTML) ? _strTargetHTML : _strTargetLaTeX;
    }

    virtual string getTitle(OutputMode mode)
    {
        return s_strUnknown;
    }

    /**
     *  Returns the class this comment is a part of. This returns NULL but is overridden
     *  in the ClassComment and FunctionComment subclasses.
     */
    virtual ClassComment* getClassForFunctionRef()
    {
        return NULL;
    }

    void append(string strLine)
    {
        _comment += strLine;
    }

    string formatContext();

    virtual string formatComment(OutputMode mode);

    string resolveExplicitRef(FormatterBase &fmt,
                              const string &strMatch);
};


/***************************************************************************
 *
 *  PageComment
 *
 **************************************************************************/

class PageComment : public CommentBase
{
    string      _title;
    string      _strTitleHTML;

protected:
    PageComment(const string &strPageID,
                const string &strTitle,
                const string &strInputFile,
                int linenoFirst,
                int linenoLast);

public:
    static PPageComment Make(const string &strPageID,
                             const string &strTitle,
                             const string &strInputFile,
                             int linenoFirst,
                             int linenoLast);

    virtual string getTitle(OutputMode mode) override;

    string makeLink(FormatterBase &fmt)
    {
        return fmt.makeLink(getTarget(fmt),
                            NULL,
                            getTitle(fmt.getMode()));
    }

    static const PagesMap& GetAll();

    static PPageComment Find(const string &strPage);
};


/***************************************************************************
 *
 *  MainPageComment
 *
 **************************************************************************/

class MainPageComment : public PageComment
{
public:
    MainPageComment(const string &strTitle,
                    const string &strInputFile,
                    int linenoFirst,
                    int linenoLast)
        : PageComment("",
                      strTitle,
                      strInputFile,
                      linenoFirst,
                      linenoLast)
    {
        _type = Type::MAINPAGE;
    }
};


/***************************************************************************
 *
 *  RESTComment
 *
 **************************************************************************/

class RESTComment;
typedef shared_ptr<RESTComment> PRESTComment;
typedef map<string, PRESTComment> RESTMap;

class RESTComment : public CommentBase
{
    string      _strMethod;
    string      _strName;
    string      _strArgs;

    RESTComment(const string &strMethod,
                const string &strName,
                const string &strArgs,
                const string &strComment,
                const string &strInputFile,
                int linenoFirst,
                int linenoLast);
public:
    static PRESTComment Make(const string &strMethod,
                             const string &strName,
                             const string &strArgs,
                             const string &strComment,
                             const string &strInputFile,
                             int linenoFirst,
                             int linenoLast);

    virtual string getTitle(OutputMode mode) override;

    string makeLink(FormatterBase &fmt)
    {
        return fmt.makeLink(getTarget(fmt),
                            NULL,
                            _strMethod + " /api/" + _strName);;
    }

    bool operator<(const RESTComment &p) const
    {
        if (_strName < p._strName)
            return true;
        if (_strName == p._strName)
            return _strMethod < p._strMethod;
        return false;
    }

    static string MakeIdentifier(const string &name, const string method)
    {
        return name + "_" + strToLower(method);
    }

    static const RESTMap& GetAll();

    static PRESTComment Find(const string &strIdentifier);
};


/***************************************************************************
 *
 *  TableComment
 *
 **************************************************************************/

class TableComment : public CommentBase
{
    StringVector _vDefinitionLines;

    TableComment(const string &strIdentifier,
                 const string &strComment,
                 const string &strInputFile,
                 int linenoFirst,
                 int linenoLast)
        : CommentBase(Type::TABLE,
                      "",
                      strIdentifier,
                      strComment,
                      strInputFile,
                      linenoFirst,
                      linenoLast,
                      "table_" + strIdentifier)
    { }

public:
    static PTableComment Make(const string &strIdentifier,
                              const string &strComment,
                              const string &strInputFile,
                              int linenoFirst,
                              int linenoLast);

    void processInputLine(const string &strLine, State &state);

    virtual string getTitle(OutputMode mode) override;

    virtual string formatComment(OutputMode mode) override;

    string makeLink(FormatterBase &fmt)
    {
        return fmt.makeLink(getTarget(fmt), NULL, _identifier);
    }

    static const TablesMap& GetAll();

    static PTableComment Find(const string &strTable);
};


/***************************************************************************
 *
 *  ClassComment
 *
 **************************************************************************/

class ClassComment : public CommentBase
{
    FunctionsVector     _vMembers;
    FunctionsMap        _mapMembers;

    StringVector        _vParents;          // Classes and interfaces this class extends / implements
    ClassesVector       _vChildren;

    StringVector        _vImplements;
    StringVector        _vExtends;

    // Regex for class linkification. Instantiate them in the class data so we don't have to recompile them.
    Regex               _reClassHTML;
    Regex               _reClassLaTeX;

    string              _strReplSelfHTML;
    string              _strReplOtherHTML;
    string              _strReplOtherLaTeX;

    ClassComment(const string &strKeyword,
                 const string &strIdentifier,
                 const string &strComment,
                 const string &strInputFile,
                 int linenoFirst,
                 int linenoLast);

public:
    static PClassComment Make(const string &strKeyword,
                              const string &strIdentifier,
                              const string &strComment,
                              const string &strInputFile,
                              int linenoFirst,
                              int linenoLast);

    void addImplements(const string &str)
    {
        _vImplements.push_back(str);
    }

    void addExtends(const string &str)
    {
        _vExtends.push_back(str);
    }

    void addParent(const string &str)
    {
        _vParents.push_back(str);
    }

    void addChild(PClassComment pChild)
    {
        _vChildren.push_back(pChild);
    }

    void addMember(PFunctionComment pMember);

    /**
     *  Override of the ClassComment function. This returns "this", meaning that
     *  if a \ref in a class comment refers to a function without a class name,
     *  the current class is assumed.
     */
    virtual ClassComment* getClassForFunctionRef() override
    {
        return this;
    }

    const StringVector& getParents()
    {
        return _vParents;
    }

    const ClassesVector& getChildren()
    {
        return _vChildren;
    }

    const FunctionsVector& getMembers()
    {
        return _vMembers;
    }

    virtual string getTitle(OutputMode mode) override;

    string formatChildrenList(FormatterBase &fmt);

    string formatHierarchy(FormatterBase &fmt);

    string formatMembers(FormatterBase &fmt);

    string makeLink(FormatterBase &fmt,
                    const string &strDisplay,
                    const string *pstrAnchor);

    static void LinkifyClasses(FormatterBase &fmt,
                               string &str,
                               const string *pstrSelf);

    void linkify(FormatterBase &fmt,
                 string &htmlComment,
                 bool fSelf);

    static const ClassesMap& GetAll();

    static PClassComment Find(const string &strClass);

};


/***************************************************************************
 *
 *  FunctionComment
 *
 **************************************************************************/

class FunctionComment : public CommentBase
{
    PClassComment   _pClass;
    ParamsVector    _vParams;

public:
    FunctionComment(const string &strKeyword,
                    const string &strIdentifier,
                    const string &strComment,
                    const string &strInputFile,
                    int linenoFirst,
                    int linenoLast)
        : CommentBase(Type::FUNCTION,
                      strKeyword,
                      strIdentifier,
                      strComment,
                      strInputFile,
                      linenoFirst,
                      linenoLast,
                      "")
    { }

    /**
     *  Override of the ClassComment function.
     */
    virtual ClassComment* getClassForFunctionRef() override
    {
        return _pClass.get();
    }

    void setClass(PClassComment pClass);

    void parseParam(string &oneParam, const string &descr);

    void parseArguments(const string &strLine, State &state);

    string formatFunction(FormatterBase &fmt, bool fLong);
};

#endif // PHOXYGEN_H
