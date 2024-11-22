/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textdocumentfilter.h"
#include <QString>
#include <QStringList>
#include "edbee/models/textrange.h"

#include "edbee/debug.h"

namespace edbee {

/// The default virtual constructor
TextDocumentFilter::~TextDocumentFilter()
{
}

/// A filter for single string replacements
void TextDocumentFilter::filterReplaceRangeSet(TextDocument *doc, TextRangeSet &rangeSet, QString& str)
{
    Q_UNUSED(doc);
    Q_UNUSED(rangeSet);
    Q_UNUSED(str);

}

/// A filter for stringlist replacements {}
void TextDocumentFilter::filterReplaceRangeSet(TextDocument* doc, TextRangeSet& rangeSet, QStringList& str)
{
    Q_UNUSED(doc);
    Q_UNUSED(rangeSet);
    Q_UNUSED(str);
}

}// edbee
