/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

class QString;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    class QStringList;
#endif

namespace edbee {

class MultiTextChange;
class Change;
class ChangeGroup;
class TextDocument;
class TextRangeSet;


class EDBEE_EXPORT TextDocumentFilter {
public:
    virtual ~TextDocumentFilter();

/*
    /// this method is called when a group is added to the undo-stack
    virtual void filterBeginGroup( TextDocument* doc, TextChangeGroup* group ) = 0;

    /// This method is called when group is ended
    virtual void filterEndGroup( TextDocument* doc, TextChangeGroup* group, int coalesceId, bool flatten ) = 0;
*/

    /// makes it possible to modify the range-set that's passed to the document
    /// @param doc the textdocument
    /// @param rangeSet the rangeSet that's replaced
    /// @param str the string that's going to placed at the given ranges
    virtual void filterReplaceRangeSet( TextDocument* doc, TextRangeSet& rangeSet, QString& str  );



    /// makes it possible to modify the range-set string that's passed to the document
    /// @param doc the textdocument
    /// @param rangeSet the rangeSet that's replaced
    /// @param str the stringsets that's going to placed at the given ranges
    virtual void filterReplaceRangeSet( TextDocument* doc, TextRangeSet& rangeSet, QStringList& str  );


    /// This method is called for any other change
    /// Just before this change is executed the changes are passed to this method
    ///
    /// You can do what you want with this text change. Just remember you are the owner and if you do nothing. Nothing happens
    ///
    /// WARNING: When you do NOT use the changes object you should delete it!!!
    ///
    /// When done you can call doc->giveTextChangeWithoutFilter( changes ) for applying the changes
    ///
    /// @param doc a reference to the document
    /// @param changes the multi-text-change that's excuted
    /// @param coalesceId the coalesceId
    ///
    /// @return this method should returnt the effictive change applied
    virtual Change* filterChange( TextDocument* doc, Change* change, int coalesceId ) = 0;



};

} // edbee
