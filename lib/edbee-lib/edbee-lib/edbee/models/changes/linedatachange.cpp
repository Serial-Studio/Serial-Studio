/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "linedatachange.h"

#include "edbee/models/textdocument.h"
#include "edbee/models/textlinedata.h"

#include "edbee/debug.h"

namespace edbee {

LineDataChange::LineDataChange( int line, int field )
    : line_(line)
    , field_(field)
    , lineData_( 0 )
{
}

LineDataChange::~LineDataChange()
{
    delete lineData_;
}

void LineDataChange::giveLineData(TextLineData *lineData)
{
    lineData_ = lineData;
}

void LineDataChange::execute(TextDocument* doc)
{
    changeLineData( doc );
}

void LineDataChange::revert(TextDocument* doc)
{
    changeLineData( doc );
}

/// merge is never a problem, simply
bool LineDataChange::giveAndMerge(TextDocument* document, Change* textChange)
{
    Q_UNUSED( document );
    Q_UNUSED( textChange );
    LineDataChange* change =  dynamic_cast<LineDataChange*>(lineData_);
    if( change ) {
        if( line_ == change->line_ && field_ == change->field_ ) {
            delete textChange;
            return true;
        }
    }
    return false;
}

/// line is moved with the given delta
void LineDataChange::applyLineDelta(int line, int length, int newLength)
{
    if( line <= line_ ) {
        line_ += newLength - length;
    }
}


/// Returns the debug text
QString LineDataChange::toString()
{
    return QStringLiteral("LineDataChange(%1,%2)").arg(line_).arg(field_);
}


/// The change line data
void LineDataChange::changeLineData(TextDocument* doc)
{
    TextLineData* oldLineData = doc->lineDataManager()->take( line_, field_ );
    doc->lineDataManager()->give( line_, field_, lineData_ );
    lineData_ = oldLineData;
}


/// Returns the line index
int LineDataChange::line() const
{
    return line_;
}


/// Sets the line of this change
void LineDataChange::setLine(int line)
{
    line_ = line;
}


/// retursn the field index of this line-data item
int LineDataChange::field() const
{
    return field_;
}


/// sets the field position
void LineDataChange::setField(int field)
{
    field_ = field;
}


} // edbee
