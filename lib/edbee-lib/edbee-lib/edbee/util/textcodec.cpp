/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include <QTextCodec>
#include <QApplication>

#include "textcodec.h"

#include "edbee/debug.h"

namespace edbee {

/// The codecmanager constructs
/// This method registeres all codecs available in Qt
TextCodecManager::TextCodecManager()
{
    // append all special encodings
    QList<QByteArray> encList;
    encList << "UTF-8" << "UTF-16" << "UTF-16BE" << "UTF-16LE" << "UTF-32" << "UTF-32BE" << "UTF-32LE";
    foreach( QByteArray enc, encList ) {
        QTextCodec* codec = QTextCodec::codecForName(enc);
        giveTextCodec( new TextCodec( QString(codec->name()), codec, QTextCodec::IgnoreHeader ) );
        giveTextCodec( new TextCodec( QStringLiteral("%1 with BOM").arg( QString(codec->name()) ), codec, QTextCodec::DefaultConversion ) );
    }

    // append the items
    QList<QByteArray> names = QTextCodec::availableCodecs();
    std::sort(names.begin(), names.end());
    foreach( QByteArray name, names) {
        QTextCodec* codec = QTextCodec::codecForName(name);
        if( !codecRefMap_.contains( codec->name() ) ) {
            TextCodec* textCodec = new TextCodec( name, codec, QTextCodec::DefaultConversion );
            giveTextCodec( textCodec );
        }
    }
}


/// Cleansup the codec list
TextCodecManager::~TextCodecManager()
{
    qDeleteAll(codecList_);
}


/// Registers the given codec. The ownership of this codec is transfered to the codec manager
/// @param codec the codec to register.
void TextCodecManager::giveTextCodec( TextCodec* codec )
{
    codecList_.append(codec);
    codecRefMap_.insert(codec->name(), codec);
}


/// Returns the list with all codecs
QList<TextCodec*> TextCodecManager::codecList()
{
    return codecList_;
}


/// Returns the given codec for the given name
/// @param name the name of the codec to search
/// @return the found codec or 0 if not found
TextCodec* TextCodecManager::codecForName(const QString& name)
{
    return codecRefMap_.value(name);
}


//----------------------------------------------------------


/// The default constructs for a textcodec
/// @param name the name of the codec
/// @param codec the Qt codec to reference
/// @param the QTextCodec conversion flags (is used for creating codecs with and without BOM)
TextCodec::TextCodec( const QString& name, const QTextCodec* codec, QTextCodec::ConversionFlags flags)
    : name_(name)
    , codecRef_( codec )
    , flags_( flags )
{
}


/// Returns the QTextCodec reference
const QTextCodec* TextCodec::codec()
{
    return codecRef_;
}

/// Creates a QTextEncoder
QTextEncoder* TextCodec::makeEncoder()
{
    return codec()->makeEncoder( flags_ );
}


/// Creates a QTextDecodec
QTextDecoder* TextCodec::makeDecoder()
{
    return codec()->makeDecoder( flags_ );
}


} // edbee
