/**
 * Copyright 2011-2014 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "dynamicvariables.h"

#include "edbee/models/textdocumentscopes.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/textdocumentscopes.h"
#include "edbee/texteditorcontroller.h"

#include "edbee/debug.h"

namespace edbee {


/// empty virtual destructor
DynamicVariable::~DynamicVariable()
{
}


//--------------------------------


/// Constructs a basic dynamic (read static :P ) variable
BasicDynamicVariable::BasicDynamicVariable( const QVariant& value )
    : value_(value)
{
}


/// returns value of the variable
QVariant BasicDynamicVariable::value() const
{
    return value_;
}


//--------------------------------


/// constructs a dynamic variable
/// @param value the value of the variable
/// @param selector (default=0) the selector to use. This class takes ownership of the selector!!
ScopedDynamicVariable::ScopedDynamicVariable( const QVariant& value, TextScopeSelector* selector  )
    : BasicDynamicVariable(value)
    , selector_(selector)
{
}


/// destructs the variable
ScopedDynamicVariable::~ScopedDynamicVariable()
{
    delete selector_;
}


/// returns the score for the location
double ScopedDynamicVariable::score( TextScopeList* scopes ) const
{
    return selector()->calculateMatchScore( scopes );
}


/// returns the active selector
TextScopeSelector* ScopedDynamicVariable::selector() const
{
    return selector_;
}


//--------------------------------

/// default constructor
DynamicVariables::DynamicVariables()
{
}


/// Destructs all variables
DynamicVariables::~DynamicVariables()
{
    qDeleteAll(variableMap_);
    qDeleteAll(scopedVariableMap_);
}


/// defines a dynamic variable and gives to the dynamic variables environment
/// @param name the name of the variable to set
/// @param value the value of the variable
/// @param scopes the scope selector
void DynamicVariables::setAndGiveScopedSelector(const QString& name, const QVariant& value, const QString& selector)
{
    /// Todo, perhaps we should detect identical scope selectors and replace the original
    variableNames_.insert(name);
    scopedVariableMap_.insert( name, new ScopedDynamicVariable(value, new TextScopeSelector(selector) ) );
}


/// Sets the given static variable to the given value
/// @param name the name of the
void DynamicVariables::set(const QString& name, const QVariant& value)
{
    delete variableMap_.value(name);     // delete the old one
    variableNames_.insert(name);
    variableMap_.insert( name, new BasicDynamicVariable(value) );
}


/// Returns the number of variables available
/// @return the number of variable names. (there can be more values as variables)
int DynamicVariables::size() const
{
    return variableNames_.size();
}


/// returns the number of variable-rules/entries available with the given name
/// @param name the name of the variable to retrieve the size for
/// @return the number of variables entires with the given name
int DynamicVariables::valueCount(const QString& name) const
{
    return scopedVariableMap_.count(name) + variableMap_.count(name);
}


/// Finds the variable for the given condition
/// @param name the name of the variable to search
/// @param scopeList the scope list to find the variable for
DynamicVariable* DynamicVariables::find(const QString& name, TextScopeList* scopelist)
{
//qlog_info() << "name: " << name << "," << scopelist->toString();
    // the initial result is no variable found
    DynamicVariable* result = variableMap_.value(name);
    double resultScore = -0.01;
    if( scopelist ) {
        foreach( ScopedDynamicVariable* var, scopedVariableMap_.values(name) ) {
            double score = var->score( scopelist );
//qlog_info() << "-  " << var->value() << ": score: " << score << " > " << resultScore;

            // the variable is only found if the score is better
            if( score > resultScore ) {
                resultScore = score;
                result = var;
            }
        }
    }
    return result;
}


/// Returns the value at the given position
/// @param name the name of the variable
/// @param scopeList the scope list to find the variable for
QVariant DynamicVariables::value(const QString& name, TextScopeList* scopelist )
{
    DynamicVariable* var = find( name, scopelist );
    if( var ) {
        return var->value();
    }
    return QVariant();
}



} // edbee
