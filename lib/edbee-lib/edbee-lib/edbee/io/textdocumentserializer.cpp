/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textdocumentserializer.h"

#include <QBuffer>
#include <QIODevice>
#include <QTextCodec>

#include "edbee/models/textdocument.h"
#include "edbee/util/lineending.h"
#include "edbee/util/textcodecdetector.h"
#include "edbee/util/textcodec.h"

#include "edbee/debug.h"

namespace edbee {

TextDocumentSerializer::TextDocumentSerializer( TextDocument* textDocument )
    : textDocumentRef_(textDocument)
    , blockSize_( 8192 )
    , filterRef_(0)
{
}


/// loads the file data for the given (opened) ioDevice
/// @return true on success,
bool TextDocumentSerializer::loadWithoutOpening( QIODevice* ioDevice )
{
    errorString_.clear();

    // start raw appending
    textDocumentRef_->rawAppendBegin();

    // read the file in blocks of 4096 bytes
    TextCodec* detectedCodec = 0;
    const LineEnding* detectedLineEnding = 0;

    QTextDecoder* textDecoder=0;

    // read the buffer
    QByteArray bytes(blockSize_ + 1, 0);
    QString remainingBuffer;

    /// TODO: atEnd doesn't seem to work !?!
/// TODO: implement isStopRequested to stop loading if required
    while( /*ioDeviceRef_->atEnd() &&*/ true /*!isStopRequested()*/ ) {

        int bytesRead = ioDevice->read( bytes.data(), blockSize_ - 1);
        if( bytesRead > 0 ) {
            bytes[bytesRead + 1] = 0; // 0 terminate the read bytes

            // In the first block we're need to detect the correct encoding
            if( !detectedCodec ) {
                TextCodecDetector codecDetector(bytes.constData(), bytesRead);
                detectedCodec = codecDetector.detectCodec();
                Q_ASSERT(detectedCodec);
                textDecoder = detectedCodec->makeDecoder();
            }

            // convert the bytes to a string
            QString newBuffer = textDecoder->toUnicode( bytes.constData(), bytesRead );

            // next detect the line ending. When no line ending is detected 0 is returned!
            if( !detectedLineEnding ) {
                detectedLineEnding = LineEnding::detect( newBuffer );
            }

            // when we detected a line ending
            if( detectedLineEnding ) {
                remainingBuffer.append( newBuffer );
                remainingBuffer = appendBufferToDocument( remainingBuffer );
            // else we need to append it to the rest
            } else {
                remainingBuffer.append( newBuffer );
            }
        }

        // we're done
        if( bytesRead <= 0 ) break;
    }

    // When no line ending could be detected, take the unix line ending
    if( !detectedLineEnding ) detectedLineEnding = LineEnding::get(LineEnding::UnixType); // fallback to unix
    if( !detectedCodec ) {  detectedCodec = TextCodecDetector::globalPreferedCodec();  }

    // set the detected items
    delete textDecoder;
    textDocumentRef_->setEncoding( detectedCodec );
    textDocumentRef_->setLineEnding( detectedLineEnding );

    // append the remaing line ending
    remainingBuffer = appendBufferToDocument( remainingBuffer );
    //fileDocument_->appendLine( remainingBuffer, FsFileDocument::Line::StateUnkown ) ;  // append the last line

    // next detect the file type
//    FileType *fileType = app()->fileTypeManager()->detectFileType( virtualFile()->fileName() );
//    fileDocument_->setFileType( fileType );

    // start raw appending
    textDocumentRef_->rawAppendEnd();
    return true;
}



/// executes the file loading for the given (unopened) ioDevice
/// @return true on success,
bool TextDocumentSerializer::load( QIODevice* ioDevice )
{
    errorString_.clear();

    // open the device
    if( !ioDevice->open( QIODevice::ReadOnly ) ) {
        errorString_ = ioDevice->errorString();
        return false;

    }
    bool result = loadWithoutOpening( ioDevice );
    ioDevice->close();

    return result;
}


/// loads the file data for the given (opened) ioDevice
/// @return true on success
bool TextDocumentSerializer::saveWithoutOpening(QIODevice *ioDevice)
{
    errorString_.clear();

    // get the codec en encoder
    TextCodec* codec = textDocumentRef_->encoding();
    QTextEncoder* encoder = codec->makeEncoder();
    QString lineEnding( textDocumentRef_->lineEnding()->chars() );

    // work via a buffer
    QByteArray buffer;
    for( int lineIdx=0,cnt=textDocumentRef_->lineCount(); lineIdx<cnt; ++lineIdx ) {
        QString line = textDocumentRef_->lineWithoutNewline(lineIdx);
        if( filter() ) {
            // if this line is not selected move to the next
            if( !filter()->saveLineSelector( this, lineIdx, line ) ) { continue; }
        }
        buffer.append( encoder->fromUnicode( line) );

        // no newline after the last line
        if( lineIdx+1<cnt ) {
            buffer.append( encoder->fromUnicode( lineEnding ) );
        }

        // flush the bufer
        if( buffer.size() >= blockSize_ ) {
            if( ioDevice->write(buffer) < 0 ) {
                errorString_ = ioDevice->errorString();
                buffer.clear();
                break;
            }
            buffer.clear();
        }
    }

    // flush the last part of the buffer
    if( buffer.size() ) {
        if( ioDevice->write(buffer) < 0 ) {
            errorString_ = ioDevice->errorString();
        }
        buffer.clear();
    }
    delete encoder;
    return errorString_.isEmpty();
}


/// Saves the given document to the iodevice
/// This method opens and closes the ioDevice..
bool TextDocumentSerializer::save(QIODevice* ioDevice)
{
    errorString_.clear();

    // open the device
    if( !ioDevice->open( QIODevice::WriteOnly ) ) {
        errorString_ = ioDevice->errorString();
        return false;
    }

    bool result = saveWithoutOpening(ioDevice);
    ioDevice->close();

    return result;
}


/// This method appends the given lines to the document
/// @param strIn the string to append
/// @retirn the remaining last line
QString TextDocumentSerializer::appendBufferToDocument(const QString& strIn)
{
    // The code blow is to translate all newlines to the correct format
    int lastPos = 0;
    for( int pos=0, len=strIn.length(); pos<len; ++pos ) {

        // we found an 'wrong' endline
        QChar c = strIn.at(pos);
        if( c  == '\r' ) {
            // append the buffer
            if( pos-lastPos-1 >= 0 ) {
                textDocumentRef_->rawAppend( strIn.data() + lastPos, pos-lastPos);
                lastPos = pos;
            }

            // lookahead to check if it's a '\n'
            if( pos+1 < strIn.length() ) {
                pos += 1;
                QChar c2 = strIn.at(pos);
                if( c2 != '\n' ) {
                    pos -= 1;
                }
                lastPos = pos;
                //textDocumentRef_->rawAppend('\n');
            // we need to process the last line-end (later)
            } else {
                return "\r";
            }
        }
    }

    if( strIn.length()-lastPos > 0 ) {
        textDocumentRef_->rawAppend( strIn.data() + lastPos, strIn.length()-lastPos);
    }
    return "";
}


} // edbee
