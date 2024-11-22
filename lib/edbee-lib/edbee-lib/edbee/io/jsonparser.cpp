/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include <QFile>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "jsonparser.h"

#include "edbee/debug.h"

namespace edbee {


/// Constructs the jsonparser
JsonParser::JsonParser()
    : errorMessage_()
    , errorOffset_(0)
    , errorLine_(0)
    , errorColumn_(0)
{

}


/// The parser destructor
JsonParser::~JsonParser()
{
}


/// loads the given keymap file returns true on  success
/// @param filename the filename to parse
/// @return true if the file was parsed
bool JsonParser::parse(const QString& fileName )
{
    clearErrors();
    QFile file(fileName);
    if( file.open( QIODevice::ReadOnly ) ) {
        bool result = parse( &file );
        file.close();
        return result;
    } else {
        errorMessage_ = file.errorString();
        return false;
    }
}


/// Parses the given QIODevice
/// @param device the device to parse the json data
/// @return true if the file was successfully parsed
bool JsonParser::parse( QIODevice* device )
{
    clearErrors();
    bool opened = false;
    if( !device->isOpen() ) {
        if( !device->open( QIODevice::ReadOnly) ) {
            errorMessage_ = device->errorString();
            return false;
        }
        opened = true;
    }

    QByteArray bytesIn = device->readAll();
    bool result = parse( bytesIn );
    if( opened ) device->close();
    return result;
}


/// opens the given bytes as a json document
/// @param bytesIn the bytes of the Json document
/// @return true if the data was succeeful parsed
bool JsonParser::parse(const QByteArray& bytesIn)
{
    clearErrors();
    QByteArray bytes = stripCommentsFromJson( bytesIn );
    QJsonParseError parseResult;
    QJsonDocument document = QJsonDocument::fromJson( bytes, &parseResult );
    if( parseResult.error != QJsonParseError::NoError ) {
        errorMessage_ = parseResult.errorString();
        errorOffset_ = parseResult.offset;

        // convert the offset to a column and offset
        errorLine_ = errorColumn_ = 0;
        for( int i=0, cnt=qMin( errorOffset_,bytes.size()); i<cnt; ++i ) {
            ++errorColumn_;
            if( bytes.at(i) == '\n' ) {
                errorColumn_= 0;
                ++errorLine_;
            }
        }
        return false;
    }
    result_ = document.toVariant();
    return true;
}


/// Returns the result of the json parsing
QVariant JsonParser::result()
{
    return result_;
}


/// Clears the error flags/variables
void JsonParser::clearErrors()
{
    errorMessage_.clear();
    errorOffset_ = errorLine_ = errorColumn_ = 0;
}


/// Returns the errormessage
QString JsonParser::errorMessage() const
{
    return errorMessage_;
}


/// returns the line number of the error
int JsonParser::errorLine() const
{
    return errorLine_;
}


/// Returns the error column
int JsonParser::errorColumn() const
{
    return errorColumn_;
}


/// Returns the full eror message
QString JsonParser::fullErrorMessage() const
{
    if( !errorLine() ) return errorMessage();
    return QObject::tr("%1 @ line %2").arg(errorMessage()).arg(errorLine());
}


/// reads all bytes from the io device, while removing comments and keeping the number of lines
/// quick and dirty 'lexer' that replaces every comment by spaces (so the number of charactes keeps the same)
QByteArray JsonParser::stripCommentsFromJson( const QByteArray& bytesIn )
{
    QByteArray result;
    enum {
        StateNormal,
        StateString,
        StateLineComment,
        StateBlockComment
    } state = StateNormal;
    for( int i=0,cnt=bytesIn.size(); i<cnt; ++i ) {
        char c = bytesIn.at(i);
        switch( state ) {
            case StateNormal:
                // a '/'
                if( c == '/' ) {
                    char c2 = ((i+1)<cnt) ? bytesIn.at(i+1) : 0;
                    if( c2 == '/') {
                        i++;
                        state = StateLineComment;
                        result.append(' ');
                        result.append(' ');
                        break;
                    } else if ( c2 == '*') {
                        i++;
                        state = StateBlockComment;
                        result.append(' ');
                        result.append(' ');
                        break;
                    }
                // start of a string
                } else if( c == '\"') {
                    state = StateString;
                }
                result.append(c);
                break;

            case StateString:
                if( c == '\\' ) {
                    char c2 = ((i+1)<cnt) ? bytesIn.at(i+1) : 0;
                    if( c2 == '\"') {
                        i++;
                        result.append('\\');
                        result.append('\"');
                        break;
                    }
                } else if( c == '\"') {
                    state = StateNormal;
                }
                result.append(c);
                break;

            case StateLineComment:
                if( c == '\r') {
                    result.append('\r');
                }
                else if( c == '\n' ) {
                    result.append(c);
                    state = StateNormal;
                } else {
                    result.append(' ');
                }
                break;

            case StateBlockComment:
                if( c == '*') {
                    char c2 = ((i+1)<cnt) ? bytesIn.at(i+1) : 0;
                    if( c2 == '/') {
                        i++;
                        state = StateNormal;
                        result.append(' ');
                    }
                    result.append(' ');
                } else if( c == '\r' || c == '\n' ) {
                    result.append(c);
                } else {
                    result.append(' ');
                }
                break;
        }
    }
    return result;
}


} // edbee
