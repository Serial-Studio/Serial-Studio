/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"


class QByteArray;

namespace edbee {

class TextCodec;

/// This class is used to detect the encoding of a given string.
/// The detector is based on the Java code of Guillaume LAFORGE
///
/// Utility class to guess the encoding of a given byte array.
/// The guess is unfortunately not 100% sure. Especially for 8-bit charsets.
/// It's not possible to know which 8-bit charset is used. Except through statistical analysis.
/// we will then infer that the charset encountered is the same as the default standard charset.
///
/// On the other hand, unicode files encoded in UTF-16 (low or big endian) or UTF-8 files
/// with a Byte Order Marker are easy to find. For UTF-8 files with no BOM, if the buffer
/// is wide enough, it's easy to guess.
///
/// A byte buffer of 4KB or 8KB is sufficient to be able to guess the encoding.
///
/// TextCodecDetector detector( QByteArray)  ;
/// TextCodec encoding = detector.guessEncoding( QByteArray arr, QTextCode fallback );
class EDBEE_EXPORT TextCodecDetector {

public:

    static TextCodec* globalPreferedCodec();
    static void setGlobalPreferedCodec( TextCodec* codec );


    explicit TextCodecDetector( const QByteArray* buffer=0,  TextCodec* preferedCodec=0 );
    explicit TextCodecDetector( const char* buffer, int length=0, TextCodec* preferedCodec=0 );
    virtual ~TextCodecDetector();


    virtual TextCodec* detectCodec();

    /// Sets the buffer reference
    virtual void setBuffer( const char* buf, int length )
    {
        bufferRef_ = buf;
        bufferLength_ = length;
    }

    /// Returns the buffer reference
    virtual const char*buffer() const { return bufferRef_; }

    /// Returns the buffer length
    virtual int bufferLength() { return bufferLength_; }

    virtual void setPreferedCodec( TextCodec* codec=0 );
    virtual TextCodec* preferedCodec() { return preferedCodecRef_; }


    virtual void setFallbackCodec( TextCodec* codec=0 );
    virtual TextCodec* fallbackCodec() const { return fallbackCodecRef_; }



protected:

    ///  If the byte has the form 10xxxxx, then it's a continuation byte of a multiple byte character;
    virtual bool isContinuationChar( char b) { return /*-128 <= b && */ b <= -65; }

    /// If the byte has the form 110xxxx, then it's the first byte of a two-bytes sequence character.
    virtual bool  isTwoBytesSequence(char b) { return -64 <= b && b <= -33; }

    /// If the byte has the form 1110xxx, then it's the first byte of a three-bytes sequence character.
    virtual bool isThreeBytesSequence(char b) { return -32 <= b && b <= -17; }

    /// If the byte has the form 11110xx, then it's the first byte of a four-bytes sequence character.
    virtual bool  isFourBytesSequence(char b) { return -16 <= b && b <= -9; }

    /// If the byte has the form 11110xx, then it's the first byte of a five-bytes sequence character.
    virtual bool  isFiveBytesSequence(char b) { return -8 <= b && b <= -5; }

    // If the byte has the form 1110xxx, then it's the first byte of a six-bytes sequence character.
    virtual bool isSixBytesSequence(char b){  return -4 <= b && b <= -3; }

public:
    static bool hasUTF8Bom( const char* buffer, int length );
    static bool hasUTF16LEBom( const char* buffer, int length );
    static bool hasUTF16BEBom( const char* buffer, int length );
    static bool hasUTF32LEBom( const char* buffer, int length );
    static bool hasUTF32BEBom( const char* buffer, int length );


private:

    //const QByteArray *bufferRef_;   ///< A reference to the current buffer of data
    const char* bufferRef_;         ///< A reference to the buffer
    int bufferLength_;              ///< The size of the buffer

    TextCodec* preferedCodecRef_;  ///< The prefered codec to use
    TextCodec* fallbackCodecRef_;   ///< The default codec to return. This is the codec to use if there's a problem detecting the codec or returning the prefered codec


};

} // edbee
