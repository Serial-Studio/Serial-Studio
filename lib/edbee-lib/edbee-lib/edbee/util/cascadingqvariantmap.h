/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QVariantMap>

namespace edbee {

/// A cascading map is a Hierarchical QVariantMap.
///
/// When retrieving an item, it fist tries to find the item at this level, if it does't find
/// the item it tries the parent item
class EDBEE_EXPORT CascadingQVariantMap {
public:
    explicit CascadingQVariantMap( CascadingQVariantMap* parent = 0);
    void deleteParents();

    void setQVariantMap( const QVariantMap& map );

    CascadingQVariantMap* root();
    CascadingQVariantMap* parent() const;
    void insert( const QString& key, const QVariant& value );


    const QVariant value( const QString& key, const QVariant& defValue = QVariant() ) const;
    const QString stringValue( const QString& key, const QString& defValue = QString() ) const;
    int intValue( const QString& key, int defValue = 0 ) const;
    double doubleValue( const QString& key, double defValue = 0.0 ) const;
    bool boolValue( const QString& key, bool defValue = false ) const;

private:

    CascadingQVariantMap* parentRef_;           ///< The parentmap
    QVariantMap map_;                   ///< The cascading map with options


};


} // edbee
