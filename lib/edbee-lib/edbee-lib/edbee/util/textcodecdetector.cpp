/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textcodecdetector.h"

#include <QTextCodec>
#include <QByteArray>

#include "textcodec.h"
#include "edbee/edbee.h"
#include "edbee/debug.h"

namespace edbee {

static TextCodec* globalPreferedCodecRef_ = 0; ///< The globally prefered codec


static inline TextCodecManager* codecManager() {
    return Edbee::instance()->codecManager();
}



/// return the static global prefered codec
TextCodec *TextCodecDetector::globalPreferedCodec()
{
    if( !globalPreferedCodecRef_ ) {
        globalPreferedCodecRef_ = codecManager()->codecForName("UTF-8");
        Q_ASSERT(globalPreferedCodecRef_);
    }
    return globalPreferedCodecRef_;
}

void TextCodecDetector::setGlobalPreferedCodec(TextCodec *codec)
{
    globalPreferedCodecRef_ = codec;
}



TextCodecDetector::TextCodecDetector(const QByteArray* buffer, TextCodec *preferedCodec)
    : bufferRef_(buffer->constData())
    , bufferLength_(buffer->size())
    , preferedCodecRef_(0)
    , fallbackCodecRef_(0)
{
    setPreferedCodec( preferedCodec );
    setFallbackCodec( 0 );
}

TextCodecDetector::TextCodecDetector(const char* buffer, int length, TextCodec *preferedCodec)
    : bufferRef_(buffer)
    , bufferLength_(length)
    , preferedCodecRef_(0)
    , fallbackCodecRef_(0)
{
    setPreferedCodec( preferedCodec );
    setFallbackCodec( 0 );
}

TextCodecDetector::~TextCodecDetector()
{

}

/// This method returns the prefered codec
void TextCodecDetector::setPreferedCodec(TextCodec* codec)
{
    if( codec ) {
        preferedCodecRef_ = codec;
    } else {
        /// prefer UTF-8
        preferedCodecRef_ = globalPreferedCodec();
    }

}


/// Sets the fallback text codec
/// @param codec the codec to use. When you use 0 the system codec is used
void TextCodecDetector::setFallbackCodec(TextCodec* codec)
{
    if( codec ) {
        fallbackCodecRef_ = codec;
    } else {
        /// prefer System
        fallbackCodecRef_ = Edbee::instance()->codecManager()->codecForName("ISO-8859-1"); //QTextCodec::codecForLocale();    // is identical to: QTextCodec::codecForName(“System”)
        Q_ASSERT(fallbackCodecRef_);
    }
}





/// Detects the encoding of the provided buffer.
/// If Byte Order Markers are encountered at the beginning of the buffer, we immidiately
/// return the charset implied by this BOM. Otherwise, the file would not be a human
///  readable text file.
///
/// If there is no BOM, this method tries to discern whether the file is UTF-8 or not.
/// If it is not UTF-8, we assume the encoding is the default system encoding
/// (of course, it might be any 8-bit charset, but usually, an 8-bit charset is the default one)
///
/// It is possible to discern UTF-8 thanks to the pattern of characters with a multi-byte sequence
///
/// @code
/// UCS-4 range (hex.)        UTF-8 octet sequence (binary)
/// 0000 0000-0000 007F       0xxxxxxx
/// 0000 0080-0000 07FF       110xxxxx 10xxxxxx
/// 0000 0800-0000 FFFF       1110xxxx 10xxxxxx 10xxxxxx
/// 0001 0000-001F FFFF       11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
/// 0020 0000-03FF FFFF       111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
/// 0400 0000-7FFF FFFF       1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
/// @endcode
///
/// With UTF-8, 0xFE and 0xFF never appear.
///
/// @return the QTextCodec that is 'detected'
TextCodec* TextCodecDetector::detectCodec()
{
    // if the file has a Byte Order Marker, we can assume the file is in UTF-xx
    // otherwise, the file would not be human readable
    if( hasUTF8Bom(bufferRef_,bufferLength_) ) return codecManager()->codecForName("UTF-8 with BOM");
    if( hasUTF16LEBom(bufferRef_,bufferLength_) ) return codecManager()->codecForName("UTF-16LE with BOM");
    if( hasUTF16BEBom(bufferRef_,bufferLength_) ) return codecManager()->codecForName("UTF-16BE with BOM");
    if( hasUTF32LEBom(bufferRef_,bufferLength_) ) return codecManager()->codecForName("UTF-32LE with BOM");
    if( hasUTF32BEBom(bufferRef_,bufferLength_) ) return codecManager()->codecForName("UTF-32BE with BOM");

    // if a byte has its most significant bit set, the file is in UTF-8 or in the default encoding
    // otherwise, the file is in US-ASCII
    bool highOrderBit = false;

    // if the file is in UTF-8, high order bytes must have a certain value, in order to be valid
    // if it's not the case, we can assume the encoding is the default encoding of the system
    bool validU8Char = true;

    // TODO the buffer is not read up to the end, but up to length - 6
    int length = bufferLength_;
    int i = 0;
    while( i < length - 6 )
    {
        char b0 = bufferRef_[i];
        char b1 = bufferRef_[i + 1];
        char b2 = bufferRef_[i + 2];
        char b3 = bufferRef_[i + 3];
        char b4 = bufferRef_[i + 4];
        char b5 = bufferRef_[i + 5];
        if (b0 < 0)
        {
            // a high order bit was encountered, thus the encoding is not US-ASCII
            // it may be either an 8-bit encoding or UTF-8
            highOrderBit = true;

            // a two-bytes sequence was encoutered
            if (isTwoBytesSequence(b0))
            {
                // there must be one continuation byte of the form 10xxxxxx,
                // otherwise the following characteris is not a valid UTF-8 construct
                if (!isContinuationChar(b1))
                    validU8Char = false;
                else
                    i++;
            }
            // a three-bytes sequence was encoutered
            else if (isThreeBytesSequence(b0))
            {
                // there must be two continuation bytes of the form 10xxxxxx,
                // otherwise the following characteris is not a valid UTF-8 construct
                if (!(isContinuationChar(b1) && isContinuationChar(b2)))
                    validU8Char = false;
                else
                    i += 2;
            }
            // a four-bytes sequence was encoutered
            else if (isFourBytesSequence(b0))
            {
                // there must be three continuation bytes of the form 10xxxxxx,
                // otherwise the following characteris is not a valid UTF-8 construct
                if (!(isContinuationChar(b1) && isContinuationChar(b2) && isContinuationChar(b3)))
                    validU8Char = false;
                else
                    i += 3;
            }
            // a five-bytes sequence was encoutered
            else if (isFiveBytesSequence(b0))
            {
                // there must be four continuation bytes of the form 10xxxxxx,
                // otherwise the following characteris is not a valid UTF-8 construct
                if (!(isContinuationChar(b1) && isContinuationChar(b2) && isContinuationChar(b3) && isContinuationChar(b4)))
                    validU8Char = false;
                else
                    i += 4;
            }
            // a six-bytes sequence was encoutered
            else if (isSixBytesSequence(b0))
            {
                // there must be five continuation bytes of the form 10xxxxxx,
                // otherwise the following characteris is not a valid UTF-8 construct
                if (!(isContinuationChar(b1) && isContinuationChar(b2) && isContinuationChar(b3) && isContinuationChar(b4) && isContinuationChar(b5)))
                    validU8Char = false;
                else
                    i += 5;
            }
            else
                validU8Char = false;
        }
        if (!validU8Char) break;
        i++;
    }

    // if no byte with an high order bit set, the encoding is US-ASCII
    // (it might have been UTF-7, but this encoding is usually internally used only by mail systems)
    if (!highOrderBit)
    {
        return preferedCodec();
        //return fallbackCodec(); /// This could be US-ASCII
    }

    // if no invalid UTF-8 were encountered, we can assume the encoding is UTF-8,
    // otherwise the file would not be human readable
    if( validU8Char ) {
//        return QTextCodec::codecForName("UTF-8");
        return preferedCodec(); // we sort of assume prefered codec is UTF-8 :P
    }

    // finally, if it's not UTF-8 nor US-ASCII, let's assume the encoding is the default encoding
    return fallbackCodec();
}


/// Has a Byte Order Marker for UTF-8
bool TextCodecDetector::hasUTF8Bom( const char* bom, int length )
{
    if( length < 3 ) return false;
    return (bom[0] == -17 && bom[1] == -69 && bom[2] == -65);
}

/// Has a Byte Order Marker for UTF-16 Low Endian
/// (ucs-2le, ucs-4le, and ucs-16le).
bool TextCodecDetector::hasUTF16LEBom( const char* bom, int length )
{
    if( length < 2 ) return false;
    return (bom[0] == -1 && bom[1] == -2);
}

/// Has a Byte Order Marker for UTF-16 Big Endian
/// (utf-16 and ucs-2).
bool TextCodecDetector::hasUTF16BEBom( const char* bom, int length )
{
    if( length < 2 ) return false;
    return (bom[0] == -2 && bom[1] == -1);
}



/// Has a Byte Order Marker for UTF-32 Low Endian
bool TextCodecDetector::hasUTF32LEBom( const char* bom, int length )
{
    if( length < 4 ) return false;
    return (bom[0] == -1 && bom[1] == -2 && bom[2] == 0 && bom[3] == 0 );
}

/// Has a Byte Order Marker for UTF-32 Big Endian
bool TextCodecDetector::hasUTF32BEBom( const char* bom, int length )
{
    if( length < 4 ) return false;
    return (bom[0] == 0 && bom[1] == 0 && bom[2] == -2 && bom[3] == -1);
}



} // edbee
