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

public:
    static FormatterBase& Get(OutputMode mode);

    virtual const string& openPara() { return Empty; }
    virtual const string& closePara() { return TwoNewlines; }

    virtual const string& openPRE() { return Empty; }
    virtual const string& closePRE() { return Empty; }

    virtual const string& openUL() { return Empty; }
    virtual const string& closeUL() { return Empty; }

    virtual const string& openOL() { return Empty; }
    virtual const string& closeOL() { return Empty; }

    virtual const string& openLI() { return Empty; }
    virtual const string& closeLI() { return Empty; }

    virtual const string& openCODE() { return Empty; }
    virtual const string& closeCODE() { return Empty; }

    virtual string format(const string &str) { return str; }

    virtual void convertFormatting(string &str) { };

    virtual string makeLink(const string &strIdentifier,
                            const string *pstrAnchor,
                            const string &strTitle)
    {
        return strTitle;
    }
};

class FormatterPlain : public FormatterBase
{
public:
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

public:
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

    virtual string format(const string &str) override;

    virtual void convertFormatting(string &str) override;

    virtual string makeLink(const string &strIdentifier,
                            const string *pstrAnchor,
                            const string &strTitle) override;
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
    virtual const string& openPRE() override { return BeginVerbatim; }
    virtual const string& closePRE() override { return EndVerbatim; }

    virtual const string& openUL() override { return BeginItemize; }
    virtual const string& closeUL() override { return EndItemize; }

    virtual const string& openOL() override { return BeginEnumerate; }
    virtual const string& closeOL() override { return EndEnumerate; }

    virtual const string& openLI() override { return Item; }

    virtual const string& openCODE() override { return OpenTextTT; }
    virtual const string& closeCODE() override { return CloseCurly; }

    virtual string format(const string &str) override;

    virtual void convertFormatting(string &str) override;
};

#endif // FORMATTER_H
