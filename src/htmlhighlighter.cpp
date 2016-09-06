/*
    Copyright (C) 2016 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "htmlhighlighter.h"
#include "format.h"
#include "syntaxdefinition.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

using namespace SyntaxHighlighting;

HtmlHighlighter::HtmlHighlighter()
{
}

HtmlHighlighter::~HtmlHighlighter()
{
}

void HtmlHighlighter::setOutputFile(const QString& fileName)
{
    m_file.reset(new QFile(fileName));
    if (!m_file->open(QFile::WriteOnly | QFile::Truncate)) {
        qWarning() << "Failed to open output file" << fileName << ":" << m_file->errorString();
        return;
    }
    m_out.reset(new QTextStream(m_file.get()));
}

void HtmlHighlighter::setOutputFile(FILE *fileHandle)
{
    m_out.reset(new QTextStream(fileHandle, QIODevice::WriteOnly));

}

void HtmlHighlighter::highlightFile(const QString& fileName)
{
    if (!m_out) {
        qWarning() << "No output stream defined!";
        return;
    }

    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open input file" << fileName << ":" << f.errorString();
        return;
    }

    reset();
    *m_out << "<!DOCTYPE html>\n";
    *m_out << "<html><head>\n";
    *m_out << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>\n";
    QFileInfo fi(fileName);
    *m_out << "<title>" << fi.fileName() << "</title>\n";
    *m_out << "<meta name=\"generator\" content=\"KF5::SyntaxHighlighting (" << definition().name() << ")\"/>\n";
    *m_out << "</head><body><pre>\n";

    QTextStream in(&f);
    while (!in.atEnd()) {
        m_currentLine = in.readLine();
        highlightLine(m_currentLine);
        *m_out << "\n";
    }

    *m_out << "</pre></body></html>\n";
    m_out->flush();

    m_out.reset();
    m_file.reset();
}

void HtmlHighlighter::setFormat(int offset, int length, const Format& format)
{
    if (!format.isNormal(theme())) {
        *m_out << "<span style=\"";
        if (format.hasColor(theme()))
            *m_out << "color:" << format.color(theme()).name() << ";";
        if (format.hasBackgroundColor(theme()))
            *m_out << "background-color:" << format.backgroundColor(theme()).name() << ";";
        if (format.isBold(theme()))
            *m_out << "font-weight:bold;";
        if (format.isItalic(theme()))
            *m_out << "font-style:italic;";
        if (format.isUnderline(theme()))
            *m_out << "text-decoration:underline;";
        if (format.isStrikeThrough(theme()))
            *m_out << "text-decoration:line-through;";
        *m_out << "\">";
    }

    *m_out << m_currentLine.mid(offset, length).toHtmlEscaped();

    if (!format.isNormal(theme()))
        *m_out << "</span>";
}
