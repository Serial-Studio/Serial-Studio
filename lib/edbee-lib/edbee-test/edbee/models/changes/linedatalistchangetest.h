/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include <QList>

#include "edbee/util/test.h"

namespace edbee {

class LineDataListChange;
class TextDocument;
class TextLineDataManager;

// For testing the line data list text changes
class LineDataListChangeTest: public edbee::test::TestCase
{
    Q_OBJECT
public:
    LineDataListChangeTest();

private slots:
    void init();
    void clean();

    void testExecute();
    void testMerge_growBelow();
    void testMerge_growAbove();
    void testMerge_shrinkBelow();
    void testMerge_shrinkAbove();

private:
    TextLineDataManager* manager();
    LineDataListChange* createChange( int line, int length, int newLength );
    LineDataListChange* takeChange(LineDataListChange* change);
    QString data2str( LineDataListChange* change );
    QString data2ptr( LineDataListChange* change );


    TextDocument* doc_;                             ///< The document used for testing
    QList<LineDataListChange*> changeList_;     ///< The change list (For auto deletion of changes)
};

} // edbee

DECLARE_TEST(edbee::LineDataListChangeTest);
