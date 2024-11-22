#include "textautocompleteprovider.h"

#include "edbee/models/texteditorconfig.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/textdocumentscopes.h"
#include "edbee/models/textgrammar.h"

#include "edbee/debug.h"

namespace edbee {

/// @param kind the kind of autocomplete items (Use a Langserver constant)
TextAutoCompleteItem::TextAutoCompleteItem(const QString &label, const int kind, const QString &detail, const QString &documentation)
    : label_(label),
      kind_(kind),
      detail_(detail),
      documentation_(documentation)
{
}


/// returns the autocmomplete label
QString TextAutoCompleteItem::label() const
{
    return label_;
}

int TextAutoCompleteItem::kind() const
{
    return kind_;
}

QString TextAutoCompleteItem::detail() const
{
    return detail_;
}

QString TextAutoCompleteItem::documentation() const
{
    return documentation_;
}


/// Compares the given text-autocomplete item with the label
/// This method should return a match score, where a score of 0 means NO match
/*! \fn int TextAutoCompleteItem::matchLabelScore(TextDocument *document, const TextRange &range, const QString &word)
    Returns a copy of the \a str string. The given string is converted
    to Unicode using the fromUtf8() function.
    \sa fromLatin1(), fromLocal8Bit(), fromUtf8(), QByteArray::fromStdString()
*/
int TextAutoCompleteItem::matchLabelScore(TextDocument* doc, const TextRange&, const QString &word)
{
    /// For now a simple prefix-prefix search. Later fuzzy search.
    /// Inspiration:
    /// - https://www.quora.com/How-is-the-fuzzy-search-algorithm-in-Sublime-Text-designed-How-would-you-design-something-similar)
    /// - https://github.com/renstrom/fuzzysearch
    /// We probably need to calculate a score
    if( word.length() < doc->config()->autocompleteMinimalCharacters())
        return 0;
    if( label_ == word ) {
        return 1;
    } else if ( label_.toLower().startsWith(word.toLower()) ) {
        return 2;
    } else if ( label_.toLower().contains(word.toLower()) ) {
        return 3;
    } else {
        return 0;
    }
}



// -----------------------------


/// Adds the given string to the list
StringTextAutoCompleteProvider::~StringTextAutoCompleteProvider()
{
    qDeleteAll(itemList_);
}

/// Search auto-complete items in the list
QList<TextAutoCompleteItem *> StringTextAutoCompleteProvider::findAutoCompleteItemsForRange(TextDocument *document, const TextRange &range, const QString &word)
{
    QMultiMap<int, TextAutoCompleteItem *> items;

    foreach( TextAutoCompleteItem* item, itemList_ ) {
        int match = item->matchLabelScore(document,range,word);
        if( match && match == 1 && item->kind() == edbee::TextAutoCompleteKind::Keyword ) {
            items.clear();
            return items.values();
        } else if( match && item->kind() != edbee::TextAutoCompleteKind::Keyword ) {
            items.insert(match, item);
        }
    }

    if( items.size() == 1 && items.contains(1) ) {
        items.clear();
        return items.values();
    }
    return items.values();
}


/// directly add a label
void StringTextAutoCompleteProvider::add(const QString &label, const int kind, const QString &detail, const QString &documentation)
{
    itemList_.push_back(new TextAutoCompleteItem(label, kind, detail=="" && kind == 3 ? label + "()" : detail, documentation));
}


/// gives the given autocomplete item
void StringTextAutoCompleteProvider::give(TextAutoCompleteItem *item)
{
    itemList_.push_back(item);
}

// ------------------------------

TextAutoCompleteProviderList::TextAutoCompleteProviderList(TextAutoCompleteProvider *parentProvider)
    : parentProviderRef_(parentProvider)
{
}

TextAutoCompleteProviderList::~TextAutoCompleteProviderList()
{
    qDeleteAll(providerList_);
}


/// builds an auto-complete list by querying all providers
QList<TextAutoCompleteItem *> TextAutoCompleteProviderList::findAutoCompleteItemsForRange(TextDocument *document, const TextRange &range, const QString &word)
{
    QList<TextAutoCompleteItem*> result;
    foreach( TextAutoCompleteProvider* provider, providerList_) {
        result.append(provider->findAutoCompleteItemsForRange(document,range,word));
    }
    if( parentProviderRef_ ) {
        result.append(parentProviderRef_->findAutoCompleteItemsForRange(document,range,word));
    }
    return result;
}


/// adds a provider
void TextAutoCompleteProviderList::giveProvider(TextAutoCompleteProvider *provider)
{
    providerList_.push_back(provider);
}


/// set the parent provider
void TextAutoCompleteProviderList::setParentProvider(TextAutoCompleteProvider *provider)
{
    parentProviderRef_ = provider;
}


// -----------------------------
/*


/// retuns all auto complete items for the given list
QList<TextAutoCompleteItem *> ScopeBasedTextAutoCompleteProvider::findAutoCompleteItemsForRange(TextDocument *document, TextRange &range)
{
//    TextScopeList scopes = document->scopes()->scopesAtOffset(range.caret());
//    if( scopes.cont)
    /// TODO: Not yet implemented

    return QList<TextAutoCompleteItem*>();
}

*/

// -----------------------------
/*

TextAutoCompleteManager::TextAutoCompleteManager()
{
}

TextAutoCompleteManager::~TextAutoCompleteProvider()
{
    qDeleteAll(providerList_);
}

void TextAutoCompleteManager::giveProvider(TextAutoCompleteManager::~TextAutoCompleteProvider *provider)
{
    providerList_.append(provider);
}
*/

}
