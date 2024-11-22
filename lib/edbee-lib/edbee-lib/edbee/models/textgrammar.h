/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QHash>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

class QFile;

namespace edbee {

class RegExp;
class TextGrammar;
class Edbee;


/// defines a single grammar rule
class EDBEE_EXPORT TextGrammarRule {
public:

    /// the instructions
    enum Instruction {
        MainRule,                   ///< The main rule has no regexp matches
        RuleList,                   ///< A list of rules (no regexp)
        SingleLineRegExp,           ///< A single line regexp
        MultiLineRegExp,            ///< A multi-line regexp (begin end)
        IncludeCall,                ///< Includes another scope
        Parser                      ///< A full parser (not yet supported, by added as idea for the future). But could be marked by multiple regexps
    };


    TextGrammarRule( TextGrammar* grammar, Instruction instruction );
    ~TextGrammarRule();

    static TextGrammarRule* createMainRule( TextGrammar* grammar, const QString& scopeName );
    static TextGrammarRule* createRuleList( TextGrammar* grammar );
    static TextGrammarRule* createSingleLineRegExp( TextGrammar* grammar, const QString& scopeName, const QString& regExp );
    static TextGrammarRule* createMultiLineRegExp( TextGrammar* grammar, const QString& scopeName, const QString& contentScopeName, const QString& beginRegExp, const QString& endRegExp );
    static TextGrammarRule* createIncludeRule( TextGrammar* grammar, const QString& includeName );

    inline bool isMainRule() { return instruction_ == MainRule; }
    inline bool isRuleList() { return instruction_ == RuleList; }
    inline bool isMultiLineRegExp() { return instruction_ == MultiLineRegExp; }
    inline bool isSingleLineRegExp() { return instruction_ == SingleLineRegExp; }
    inline bool isIncludeCall() { return instruction_ == IncludeCall; }

    int ruleCount() const { return ruleList_.size(); }
    TextGrammarRule* rule( int idx ) const;
    void giveRule( TextGrammarRule* rule );

    void giveMatchRegExp( RegExp* regExp );
    void setEndRegExpString( const QString& str );

    Instruction instruction() const { return instruction_; }
    void setInstruction( Instruction ins ) { instruction_ = ins; }
    QString scopeName() const  { return scopeName_; }
    void setScopeName( const QString& scopeName ) { scopeName_ = scopeName; }
    RegExp* matchRegExp() const { return matchRegExp_; }
    QString endRegExpString() const { return endRegExpString_; }
    const QMap<int,QString>& matchCaptures() { return matchCaptures_; }
    const QMap<int,QString>& endCaptures() { return endCaptures_; }
    QString contentScopeName() const { return contentScopeName_; }
    void setContentScopeName( const QString& name) { contentScopeName_ = name; }

    /// the include name is stored in the content-scopename
    QString includeName() { return contentScopeName_;  }
    void setIncludeName( const QString& name ) { contentScopeName_ = name; }

    void setCapture( int idx, const QString& name ) { matchCaptures_.insert(idx,name); }
    void setEndCapture( int idx, const QString& name ) { endCaptures_.insert(idx,name); }

    QString toString(bool includePatterns=true);


    // An itetor class for iterating over the ruleset (todo template this)
    class Iterator
    {
    public:
        Iterator( const TextGrammarRule* rule ) : index_(0), ruleRef_(rule){}
        bool hasNext() { return index_ < ruleRef_->ruleCount(); }
        TextGrammarRule* next() { return ruleRef_->rule(index_++); }
    private:
        int index_;
        const TextGrammarRule* ruleRef_;
    };

    Iterator* createIterator() { return new Iterator(this); }

    TextGrammar* grammar() { return grammarRef_; }

private:

    static RegExp* createRegExp( const QString& regexp );


private:
    TextGrammar* grammarRef_;        ///< The grammar this rule belongs toe
    Instruction instruction_;            ///< THe instruction to execute
    QString scopeName_;                  ///< the scope name of this grammar

    RegExp* matchRegExp_;                ///< The begin-matcher (or simple matcher)
    //RegExp* endRegExp_;                  ///< The end regular expression matcher
    QString endRegExpString_;            ///< The end regexp is a string

    QMap<int,QString> matchCaptures_;    ///< all captures that need to be performed
    QMap<int,QString> endCaptures_;      ///< all end captures that need to be performed

    QString contentScopeName_;           ///< The content scopename

    QList<TextGrammarRule*> ruleList_;   ///< Sub-rules to execute
};


//================================


/// This class defines a single language grammar
class EDBEE_EXPORT TextGrammar {
public:

    TextGrammar( const QString& name, const QString& displayName );
    ~TextGrammar();

    void giveMainRule( TextGrammarRule* mainRule );

    QString name() const;
    QString displayName() const;
    TextGrammarRule* mainRule() const;
    QStringList fileExtensions() const;

    void giveToRepos( const QString& name, TextGrammarRule* rule);
    TextGrammarRule* findFromRepos( const QString& name, TextGrammarRule* defValue = 0  );
    void addFileExtension( const QString& ext );

private:
    QString name_;                               ///< the display name of this
    QString displayName_;                        ///< the name to display
    TextGrammarRule *mainRule_;                      ///< the 'main' rule of this grammar
    QMap<QString, TextGrammarRule*> repository_;     ///< A map with all named grammar rules
    QStringList fileExtensions_;                  ///< A list with all file-extensions
};



//================================


/// This class is used to manage all 'grammers' used by the lexers
class EDBEE_EXPORT TextGrammarManager {
protected:
    TextGrammarManager();
    virtual ~TextGrammarManager();

public:
    TextGrammar* readGrammarFile(const QString& file );
    void readAllGrammarFilesInPath(const QString& path );

    TextGrammar* get( const QString& name );
    void giveGrammar( TextGrammar* grammar );

    QList<QString> grammarNames();
    QList<TextGrammar*> grammars();
    QList<TextGrammar*> grammarsSortedByDisplayName();

    TextGrammar* defaultGrammar() { return defaultGrammarRef_; }
    TextGrammar* detectGrammarWithFilename( const QString& fileName );

    QString lastErrorMessage() const;

private:

    TextGrammar* defaultGrammarRef_;                   ///< A reference to the default grammar
    QMap<QString,TextGrammar*> grammarMap_;            ///< A map with all grammar definitions
    QString lastErrorMessage_;                             ///< Returns the error message

    friend class Edbee;
};

} // edbee
