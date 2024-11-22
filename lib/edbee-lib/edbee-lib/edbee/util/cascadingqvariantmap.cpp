/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "cascadingqvariantmap.h"

#include "edbee/debug.h"

namespace edbee {


CascadingQVariantMap::CascadingQVariantMap(edbee::CascadingQVariantMap* parent)
    : parentRef_(parent)
{
}

/// This helper method deletes all containing parents recursively
/// Tough this doesnt' feel quite correct (it uses references to parent) it makes
/// managing the map a bit easier.
void CascadingQVariantMap::deleteParents()
{
    if( parentRef_ ) {
        parentRef_->deleteParents();
        delete parentRef_;
    }
}

/// Sets the variant mapt to the given map
void CascadingQVariantMap::setQVariantMap(const QVariantMap& map)
{
    map_ = map;
}

/// This method finds the 'root variant map
CascadingQVariantMap* CascadingQVariantMap::root()
{
    CascadingQVariantMap* result = this;
    while( result->parent() ) { result = result->parent(); }
    return result;
}


/// Returns the parent of this map
CascadingQVariantMap* CascadingQVariantMap::parent() const
{
    return parentRef_;
}


/// Insert the given value at the current cascading level
/// @param key the name of the key to set
/// @param value the value to set
void CascadingQVariantMap::insert(const QString& key, const QVariant& value)
{
    map_.insert( key, value );
}


/// Returns the value of the given key. If the map at this level doesn't have this key,
/// it cascades to a higher-level. If it doesn't find the key if will fallback to the defValue supplied
/// @param key the name of the item to retrieve
/// @param defValue the default value when the item isn't found
const QVariant CascadingQVariantMap::value(const QString& key, const QVariant& defValue) const
{
    if( map_.contains( key ) ) {
        return map_.value( key, defValue );
    }
    if( parentRef_ ) {
        return parentRef_->value( key, defValue );
    }
    return defValue;
}


/// Method to quickly fetch a string item
const QString CascadingQVariantMap::stringValue(const QString& key, const QString& defValue) const
{
    return value( key, defValue ).toString();
}

/// Method to quickly fetch int items
int CascadingQVariantMap::intValue(const QString &key, int defValue) const
{
    return value( key, defValue ).toInt();
}

/// Returns a double value
double CascadingQVariantMap::doubleValue(const QString &key, double defValue) const
{
    return value( key, defValue ).toDouble();
}

/// Returns the default boolean value
bool CascadingQVariantMap::boolValue(const QString& key, bool defValue) const
{
    return value( key, defValue ).toBool();
}



} // edbee
