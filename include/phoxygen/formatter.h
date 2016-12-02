/*
 * phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
 *
 * phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, in version 2 as it comes
 * in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
 */

#ifndef FORMATTER_H
#define FORMATTER_H

struct Param
{
    string _type;            // optiponal
    string _argname;         // including leading & and $
    string _defaultArg;      // optional, everything after =\s*
    string _description;

    string _strTypeFormattedHTML;
    string _strTypeFormattedLaTeX;

    Param(const string &type,
          const string &argname,
          const string &strDefaultArg,
          const string &descr,
          const string &strTypeFormattedHTML,
          const string &strTypeFormattedLaTeX);
};
typedef vector<Param> ParamsVector;

enum class OutputMode
{
    PLAINTEXT,
    HTML,
    LATEX
};

class FormatterBase
{
protected:
    static const string Empty;
    static const string TwoNewlines;
    static const string TwoDashes;

    OutputMode _mode;

    FormatterBase(OutputMode mode)
        : _mode(mode)
    { }

public:
    static FormatterBase& Get(OutputMode mode);

    OutputMode getMode()
    {
        return _mode;
    }

    virtual const string& openPara() { return Empty; }
    virtual const string& closePara() { return TwoNewlines; }

    virtual const string& openPRE() { return Empty; }
    virtual const string& closePRE() { return Empty; }

    virtual const string& openUL() { return Empty; }
    virtual const string& closeUL() { return Empty; }

    virtual const string& openOL() { return Empty; }
    virtual const string& closeOL() { return Empty; }

    virtual const string& openLI() { return Empty; }
    virtual const string& closeLI() { return TwoNewlines; }

    virtual const string& openCODE() { return Empty; }
    virtual const string& closeCODE() { return Empty; }

    virtual const string& mdash() { return TwoDashes; }

    virtual string format(const string &str,
                          bool fInPRE NO_WARN_UNUSED)
    {
        return str;
    }

    virtual void convertFormatting(string &str) { };

    virtual string makeLink(const string &strIdentifier,
                            const string *pstrAnchor,
                            const string &strTitle)
    {
        return strTitle;
    }

    virtual string makeBold(const string &str)
    {
        return str;
    }

    virtual string makeCODE(const string &str)
    {
        return str;
    }

    virtual string makeHeading(uint level, const string &str)
    {
        return "\n\n" + str + "\n\n";
    }

    virtual string makeFunctionHeader(const string &strKeyword,
                                      const string &strIdentifier,
                                      ParamsVector &vParams NO_WARN_UNUSED,
                                      bool fLong NO_WARN_UNUSED)
    {
        return strKeyword + " " + strIdentifier;
    }
};

class FormatterPlain : public FormatterBase
{
public:
    FormatterPlain()
        : FormatterBase(OutputMode::PLAINTEXT)
    { }
};

class FormatterHTML : public FormatterBase
{
    static const string OpenP;
    static const string CloseP;
    static const string OpenPRE;
    static const string ClosePRE;
    static const string OpenUL;
    static const string CloseUL;
    static const string OpenOL;
    static const string CloseOL;
    static const string OpenLI;
    static const string EndLI;
    static const string OpenCODE;
    static const string CloseCODE;
    static const string MDash;

public:
    FormatterHTML()
        : FormatterBase(OutputMode::HTML)
    { }

    virtual const string& openPara() override { return OpenP; }
    virtual const string& closePara() override { return CloseP; }

    virtual const string& openPRE() override { return OpenPRE; }
    virtual const string& closePRE() override { return ClosePRE; }

    virtual const string& openUL() override { return OpenUL; }
    virtual const string& closeUL() override { return CloseUL; }

    virtual const string& openOL() override { return OpenOL; }
    virtual const string& closeOL() override { return CloseOL; }

    virtual const string& openLI() override { return OpenLI; }
    virtual const string& closeLI() override { return EndLI; }

    virtual const string& openCODE() override { return OpenCODE; }
    virtual const string& closeCODE() override { return CloseCODE; }

    virtual const string& mdash() override { return MDash; }

    virtual string format(const string &str, bool fInPRE) override;

    virtual void convertFormatting(string &str) override;

    virtual string makeLink(const string &strIdentifier,
                            const string *pstrAnchor,
                            const string &strTitle) override;

    virtual string makeBold(const string &str) override
    {
        return "<b>" + str + "</b>";
    }

    virtual string makeCODE(const string &str) override
    {
        return openCODE() + str + closeCODE();
    }

    virtual string makeHeading(uint level, const string &str) override;

    virtual string makeFunctionHeader(const string &strKeyword,
                                      const string &strIdentifier,
                                      ParamsVector &vParams,
                                      bool fLong) override;
};

class FormatterLatex : public FormatterBase
{
    static const string BeginVerbatim;
    static const string EndVerbatim;
    static const string BeginItemize;
    static const string EndItemize;
    static const string BeginEnumerate;
    static const string EndEnumerate;
    static const string Item;
    static const string OpenTextTT;
    static const string CloseCurly;

public:
    FormatterLatex()
        : FormatterBase(OutputMode::LATEX)
    { }

    virtual const string& openPRE() override { return BeginVerbatim; }
    virtual const string& closePRE() override { return EndVerbatim; }

    virtual const string& openUL() override { return BeginItemize; }
    virtual const string& closeUL() override { return EndItemize; }

    virtual const string& openOL() override { return BeginEnumerate; }
    virtual const string& closeOL() override { return EndEnumerate; }

    virtual const string& openLI() override { return Item; }

    virtual const string& openCODE() override { return OpenTextTT; }
    virtual const string& closeCODE() override { return CloseCurly; }

    virtual string format(const string &str, bool fInPRE) override;

    virtual void convertFormatting(string &str) override;

    virtual string makeLink(const string &strIdentifier,
                            const string *pstrAnchor,
                            const string &strTitle) override;

    static string MakeLink(const string &strIdentifier,
                           const string &strTitle);

    virtual string makeBold(const string &str) override
    {
        return "\\textbf{" + str + CloseCurly;
    }

    virtual string makeCODE(const string &str) override
    {
        return OpenTextTT + str + CloseCurly;
    }

    virtual string makeHeading(uint level, const string &str) override;

    virtual string makeFunctionHeader(const string &strKeyword,
                                      const string &strIdentifier,
                                      ParamsVector &vParams,
                                      bool fLong) override;
};

#endif // FORMATTER_H
