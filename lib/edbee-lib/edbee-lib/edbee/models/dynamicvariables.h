/**
 * Copyright 2011-2014 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QMultiMap>
#include <QSet>
#include <QString>
#include <QVariant>

namespace edbee {

class TextScopeList;
class TextScopeSelector;

/// The abstract base class for a dynamic variable
class EDBEE_EXPORT DynamicVariable {
public:
    virtual ~DynamicVariable();
    virtual QVariant value() const = 0;
};


//--------------------------------


/// A static dynamic variable :P
class EDBEE_EXPORT BasicDynamicVariable : public DynamicVariable
{
public:
    BasicDynamicVariable( const QVariant& value );
    virtual QVariant value() const;

private:
    QVariant value_;                    ///< The value of this variable (when this scope is selected)
};


//--------------------------------


/// A class that specifies a dynamic variabele with a scope selector
class EDBEE_EXPORT ScopedDynamicVariable : public BasicDynamicVariable
{
public:
    ScopedDynamicVariable( const QVariant& value, TextScopeSelector* selector = 0 );
    virtual ~ScopedDynamicVariable();

    virtual double score(TextScopeList* scopes ) const;
    TextScopeSelector* selector() const;

private:
    TextScopeSelector* selector_;       ///< The selector that matches this dynamic variable
};


//--------------------------------


/// This class is used for remembering/managing dynamic variables
/// This are a kind of environment variables that are dependent on the given context
class EDBEE_EXPORT DynamicVariables {
public:
    DynamicVariables();
    virtual ~DynamicVariables();

    void setAndGiveScopedSelector(const QString& name, const QVariant& value, const QString& selector);
    void set( const QString& name, const QVariant& value );

    int size() const;
    int valueCount( const QString& name ) const;

    DynamicVariable* find( const QString& name, TextScopeList* scopelist );
    QVariant value( const QString& name, TextScopeList* scopeList=0 );

private:
    QSet<QString> variableNames_;                                             ///< A set with all unique variable names
    QMap<QString, BasicDynamicVariable*> variableMap_;                        ///< The static variable map
    QMultiMap<QString, ScopedDynamicVariable *> scopedVariableMap_;           ///< A map with all scoped variables.
};


} // edbee
