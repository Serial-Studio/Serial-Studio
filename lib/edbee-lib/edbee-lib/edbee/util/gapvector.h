/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QChar>
#include <QString>

//#define GAP_VECTOR_CLEAR_GAP

namespace edbee {


/// This class is used to define a gap vector. A Gapvector is split in 2 parts. where the gap
/// is moved to the insertation/changing point. So reducing the movement of fields
template <typename T>
class EDBEE_EXPORT GapVector {
public:
    GapVector( int capacity=16 ) : items_(nullptr), capacity_(0), gapBegin_(0), gapEnd_(0)  {
        items_    = new T[capacity];
        capacity_ = capacity;
        gapBegin_ = 0;
        gapEnd_   = capacity;
        growSize_ = 16;
    }

    ~GapVector() {
        delete[] items_;
    }

    /// returns the used length of the data
    inline int length() const { return capacity_ - gapEnd_ + gapBegin_; }
    inline int gapSize() const { return gapEnd_ - gapBegin_; }
    inline int gapBegin() const { return gapBegin_; }
    inline int gapEnd() const { return gapEnd_; }
    inline int capacity() const { return capacity_; }


    /// clears the data
    void clear()
    {
        delete[] items_;
        capacity_ = 16;
        items_    = new T[capacity_];
        gapBegin_ = 0;
        gapEnd_   = capacity_;
        growSize_ = 16;
    }


protected:
    /// this method replaces the given text with the given data.
    /// because the length of the source and target is the same in this method
    /// no gap-moving is required
    /// @param offset the target to move the data to
    /// @param length the number of items to replace
    /// @param data the data pointer with the source data
    void replace( int offset, int length, const T* data ) {
//qlog_info() << "** replace: " << offset << "," << length  << ": gapBegin:" << gapBegin();
        // copy the first part
        if( offset < gapBegin() ) {
            int len = qMin( gapBegin_-offset, length ); // issue 141, added -offset
//qlog_info() << "** A) len:"<<len;
            memcpy( items_ + offset, data, sizeof(T)*len );
            data      += len;   // increase the pointer
            offset    += len;
            length    -= len;
        }

        if( 0 < length ) {
//qlog_info() << "** B) offset:"<<offset+",gapSize:"<<gapSize()<<",length:"<<length;
            memcpy( items_ + offset + gapSize(), data, sizeof(T)*length );
        }
    }

    /// this method replaces the given text with the given data.
    /// because the length of the source and target is the same in this method
    /// no gap-moving is required
    /// @param offset the target to move the data to
    /// @param length the number of items to replace
    /// @param data the data pointer with the source data
    void fill( int offset, int length, const T& data ) {

        // copy the first part
        if( offset < gapBegin() ) {
            int len = qMin( gapBegin_-offset, length );
            for( int i=0; i<len; ++i ) { items_ [offset + i] = data; }
            offset    += len;
            length    -= len;
        }

        if( 0 < length ) {
            offset += gapSize();
            for( int i=0; i<length; ++i ) { items_ [offset + i] = data; }
        }
    }

public:


    /// this method replaces the given items
    /// @param offset the offset of the items to replace
    /// @param lenth the number of items to replace
    /// @param data an array with new items
    /// @param newLength the number of items in the new array
    void replace( int offset, int length, const T* data, int newLength ) {
        int currentLength=this->length();
        Q_ASSERT( 0 <= offset && ((offset+length) <= currentLength) );
        Q_UNUSED(currentLength);
//        Q_ASSERT(data && "You probably mean fill :)" );

//if( debug ) {
//qlog_info() << "REPLACE: " << offset << length << newLength;
//}
        int gapSize = this->gapSize();

        // Is it a 'delete' or 'insert' or 'replace' operation

        // a replace operation (do not perform gap moving)
        if( length == newLength ) {
            replace( offset, length, data );

        // insert operation
        } else if( length < newLength ) {
            int gapSizeRequired = newLength - length;
            ensureGapSize( gapSizeRequired );
            moveGapTo( offset + length );
            memcpy( items_ + offset, data, sizeof(T) * newLength );
            gapBegin_ = offset + newLength;

        // delete operation
        } else {
            moveGapTo( offset );
            memcpy( items_ + offset, data, sizeof(T) * newLength );
            gapBegin_ = offset + newLength;
            gapEnd_   = offset + gapSize + length;
        }

        Q_ASSERT( gapBegin_ <= gapEnd_ );
        Q_ASSERT( this->gapSize() <= capacity_ );
        Q_ASSERT( this->length() <= capacity_ );

    }

    /// this method replaces the given items with a single data item
    /// @param offset the offset of the items to replace
    /// @param lenth the number of items to replace
    /// @param newLength the number of times to repeat data
    void fill( int offset, int length, const T& data, int newLength ) {
        int currentLength=this->length();
        Q_ASSERT( 0 <= offset && ((offset+length) <= currentLength) );
        Q_UNUSED(currentLength);

        int gapSize = this->gapSize();

        // Is it a 'delete' or 'insert' or 'replace' operation

        // a replace operation (do not perform gap moving)
        if( length == newLength ) {
            fill( offset, length, data );

        // insert operation
        } else if( length < newLength ) {
            int gapSizeRequired = newLength - length;
            ensureGapSize( gapSizeRequired );
            moveGapTo( offset + length );
            for( int i=0; i<newLength; ++i ) { items_[offset+i] = data; }
            gapBegin_ = offset + newLength;

        // delete operation
        } else {
            moveGapTo( offset );
            for( int i=0; i<newLength; ++i ) { items_[offset+i] = data; }
            gapBegin_ = offset + newLength;
            gapEnd_   = offset + gapSize + length;
        }

        Q_ASSERT( gapBegin_ <= gapEnd_ );
        Q_ASSERT( this->gapSize() <= capacity_ );
        Q_ASSERT( this->length() <= capacity_ );

    }

    /// convenient append method
    void append( T t ) {
        replace( length(), 0, &t, 1 );
    }

    /// another append method
    void append( const T* t, int length ) {
        replace( this->length(), 0, t, length );
    }


    /// This method returns the item at the given index
    T at( int offset ) const {
        Q_ASSERT( 0 <= offset && offset < length() );
        if( offset < gapBegin_ ) {
            return items_[offset];
        } else {
            return items_[gapEnd_ + offset - gapBegin_];
        }
    }

    /// This method sets an item at the given index
    void set( int offset, const T& value ) {
        Q_ASSERT( 0 <= offset && offset < length() );
        if( offset < gapBegin_ ) {
            items_[offset] = value;
        } else {
            items_[gapEnd_ + offset - gapBegin_] = value;
        }
    }


    /// This method return an index
    T& operator[]( int offset ) {
        Q_ASSERT( 0 <= offset && offset < length() );
        if( offset < gapBegin_ ) {
            return items_[offset];
        } else {
            return items_[gapEnd_ + offset - gapBegin_];
        }
    }

    /// This method returns the 'raw' element at the given location
    /// This method does NOT take in account the gap
    T& rawAt( int index ) {
        Q_ASSERT(index < capacity_);
        return items_[index];
    }


    /// This method copies the given range to the data pointer
    void copyRange( QChar* data, int offset, int length ) const {

//AB__CD"
//qlog_info() << "copyRange" << offset << length;
        if( !length ) { return; }
        Q_ASSERT( 0 <= offset && offset < this->length() );
        Q_ASSERT( (offset+length) <= this->length() );

        // copy the first part
        if( offset < gapBegin() ) {
            int len = qMin( gapBegin_-offset, length );
//qlog_info() <<  " - 1: memcpy: offset=" << offset << ", len=" << len << items_[offset];
            memcpy( data, items_ + offset, sizeof(T)*len );
            data      += len;   // increase the pointer
            offset    += len;
            length    -= len;
        }

        if( length > 0 ) {
//qlog_info() <<  " - 2: memcpy:  offset="<<offset << "gapSize=" << gapSize()<< ", length=" << length << items_[offset + gapSize()];
            memcpy( data, items_ + offset + gapSize(), sizeof(T)*length );
        }
    }


    /// This method returns a direct pointer to the 0-terminated buffer
    /// This pointer is only valid as long as the buffer doesn't change
    /// WARNING, this method MOVES the gap! Which means this method should NOT be used for a lot of operations
    T* data() {
        ensureGapSize(1);
        moveGapTo( length() );
        items_[length()] = QChar(); // a \0 character
        return items_;
    }



    //// moves the gap to the given position
    //// Warning when the gap is moved after the length the gap shrinks
    void moveGapTo( int offset ) {
        Q_ASSERT( offset <= capacity_);
        Q_ASSERT( offset <= length() );
        if( offset != gapBegin_ ) {
//qlog_info() << "BEGIN moveGapTo: offset=" << offset << "/ gapBegin_"; // << gapBegin_ << getUnitTestString();
            int gapSize = this->gapSize();

            // move the the data right after the gap
            if (offset < gapBegin_ ) {
//qlog_info() << "- move"  << offset << "," << gapBegin_ << "(charcount:" << (gapBegin_ - offset)<<")";
                memmove( items_ + offset + gapSize, items_ + offset, sizeof(T) * (gapBegin_ - offset));   // memmove( target, source, size )

            } else {
//qlog_info() << "- move2: gapBegin_" << gapBegin_ << ", gapEnd_" << gapEnd_ << ", capacity_" << capacity_ << ", gapSize" << gapSize << "(charcount:"<<(offset - gapBegin_)<<")";
                memmove( items_ + gapBegin_, items_ + gapEnd_, sizeof(T) * (offset - gapBegin_ ));  // memmove( target, source, size )
            }
            gapBegin_ = offset;         
            gapEnd_   = gapBegin_ + gapSize; //qMin( gapBegin_ + gapSize, capacity_ );

        }
        Q_ASSERT( gapBegin_ <= gapEnd_ );

#ifdef GAP_VECTOR_CLEAR_GAP
        memset( items_+gapBegin_, 0, sizeof(T)*(gapEnd_-gapBegin_));
#endif
    }


    /// this method makes sure there's enough room for the insertation
    void ensureGapSize( int requiredSize ) {
        if( gapSize() < requiredSize ) {
            while( growSize_ < capacity_ / 6) { growSize_ *= 2; }
            resize(capacity_ + requiredSize + growSize_ - gapSize() );
        }
    }


    /// resizes the array of data
    void resize(int newSize)
    {
        if( capacity_ >= newSize) return;
        int lengte = length();
        Q_ASSERT( lengte <= capacity_);
/// TODO: optimize, so data is moved only once
/// in other words, gap movement is not required over here!!
/// this can be done with 2 memcopies
//qlog_info() << "BEGIN resize: capacity =" << capacity_<< " => " << newSize;


        moveGapTo( lengte );
        T *newChars = new T[ newSize ];

        if( capacity_ > 0 ) {
            memmove( newChars, items_, sizeof(T)*lengte );
            delete[] items_;
        }
        items_ = newChars;
        capacity_ = newSize;
        gapEnd_   = newSize;

        // DEBUG gapsize
#ifdef GAP_VECTOR_CLEAR_GAP
        memset( items_+gapBegin_, 0, sizeof(T)*(gapEnd_-gapBegin_));
#endif

//qlog_info() << "END resize";
    }


    /// sets the growsize. The growsize if the amount to reserve extra
    void setGrowSize( int size ) { growSize_=size; }

    /// returns the growsize
    int growSize() { return growSize_; }


    /// Converts the 'gap-buffer' to a unit-test debugging string
    QString getUnitTestString( QChar gapChar = '_' ) const {
        QString s;
        int gapBegin = this->gapBegin();
        int gapEnd   = this->gapEnd();
        int capacity = this->capacity();

        for( int i=0; i<gapBegin; ++i ) {
            if( items_[i].isNull() ) {
                s.append("@");
            } else {
                s.append( items_[i] );
            }
        }
        s.append( "[" );
        for( int i=gapBegin; i<gapEnd; ++i ) {
            s.append( gapChar );
        }
        s.append( ">" );
        for( int i=gapEnd; i<capacity; ++i ) {
            if( items_[i].isNull() ) {
                s.append("@");
            } else {
                s.append( items_[i] );
            }
        }

        return s;
    }

    /// Converts the 'gap-buffer' to a unit-test debugging string
    QString getUnitTestString2( ) const {
        QString s;
        int gapBegin = this->gapBegin();
        int gapEnd   = this->gapEnd();
        int capacity = this->capacity();

        for( int i=0; i<capacity;i++ ) {
            if( i ) { s.append(","); }
            if( gapEnd == i) s.append(">");
            s.append( QStringLiteral("%1").arg( "X" ));
            if( gapBegin==i ) s.append("[");
        }
        return s;
    }


protected:

    T *items_;              ///< The item data
    int capacity_;          ///< The number of reserved bytes
    int gapBegin_;          ///< The start of the gap
    int gapEnd_;            ///< The end of the gap
    int growSize_;          ///< The size to grow extra
};


/// The character vecor to use
class EDBEE_EXPORT QCharGapVector : public GapVector<QChar>
{
public:

    QCharGapVector( int size=16 ) : GapVector<QChar>(size){}

    /// initializes the vector with a given string
    QCharGapVector( const QString& data, int gapSize ) : GapVector<QChar>( data.length() + gapSize )
    {
        memcpy( items_, data.constData(), sizeof(QChar)*data.length() );
        gapBegin_ = data.length();
        gapEnd_ = capacity_;
    }


    /// Initializes the gapvector
    void init( const QString& data, int gapSize )
    {
        delete items_;
        capacity_ = data.length() + gapSize;
        items_ = new QChar[capacity_];
        memcpy( items_, data.constData(), sizeof(QChar)*data.length() );
        gapBegin_ = data.length();
        gapEnd_ = capacity_;
        growSize_ = 16;
    }

    /// a convenient string replace function
    void replaceString( int offset, int length, const QString& data ) {

//        qlog_info() << "replace(" << offset << length << data << ") : " << getUnitTestString().replace("\n","|");

        replace( offset, length, data.constData(), data.length() );

//        qlog_info() << " ==> " << getUnitTestString().replace("\n","|");
    }

    /// a convenient method to retrieve a QString part
    QString mid( int offset, int length ) const
    {
        Q_ASSERT( length >= 0 );

//        QString str( length, QChar() );
//        this->copyRange( str.data(), offset, length );

        QChar* data = new QChar[length];
        copyRange( data, offset, length );
        QString str( data, length );
        delete[] data;

//qlog_info() << "mid(" << offset << "," << length << ") => " << str.replace("\n","|")  << "  // " << getUnitTestString().replace("\n","|");
        return str;
    }



};


/// A special GapVector class that isn't a gapvector. It forwards it's request to a normal vector class
/// (for debugging purposes) that isn't a gapv
/// This class is only used for debugging issuess with the gapvector
template <typename T>
class EDBEE_EXPORT NoGapVector {
public:
    NoGapVector( int capacity=16 ) {
        Q_UNUSED(capacity);
    }

    ~NoGapVector() {
    }

    /// returns the used length of the data
    inline int length() const { return items_.size(); }
    inline int gapSize() const { return 0; }
    inline int gapBegin() const { return 0; }
    inline int gapEnd() const { return 0; }
    inline int capacity() const { return items_.capacity(); }


    /// clears the data
    void clear()
    {
        items_.clear();
    }



public:


    /// this method replaces the given items
    /// @param offset the offset of the items to replace
    /// @param lenth the number of items to replace
    /// @param data an array with new items
    /// @param newLength the number of items in the new array
    void replace( int offset, int length, const T* data, int newLength ) {
        items_.remove(offset,length);
        for( int i=0; i < newLength; i++ ) {
            items_.insert(offset+i,data[i]);
        }
    }

    /// this method replaces the given items with a single data item
    /// @param offset the offset of the items to replace
    /// @param lenth the number of items to replace
    /// @param newLength the number of times to repeat data
    void fill( int offset, int length, const T& data, int newLength ) {
        items_.remove(offset,length);
        for( int i=0; i < newLength; i++ ) {
            items_.insert(offset+i,data);
        }

    }

    /// convenient append method
    void append( T t ) {
        items_.append(t);
    }

    /// another append method
    void append( const T* t, int length ) {
        for( int i=0; i < length; i++ ) {
            items_.append(t[i]);
        }
    }


    /// This method returns the item at the given index
    T at( int offset ) const {
        return items_.at(offset);
    }

    /// This method sets an item at the given index
    void set( int offset, const T& value ) {
        items_.replace(offset,value);
    }


    /// This method return an index
    T& operator[]( int offset ) {
        return items_[offset];
    }

/*
    /// This method copies the given range to the data pointer
    void copyRange( QChar* data, int offset, int length ) const {

//AB__CD"
//qlog_info() << "copyRange" << offset << length;
        if( !length ) { return; }
        Q_ASSERT( 0 <= offset && offset < this->length() );
        Q_ASSERT( (offset+length) <= this->length() );

        // copy the first part
        if( offset < gapBegin() ) {
            int len = qMin( gapBegin_-offset, length );
//qlog_info() <<  " - 1: memcpy: offset=" << offset << ", len=" << len << items_[offset];
            memcpy( data, items_ + offset, sizeof(T)*len );
            data      += len;   // increase the pointer
            offset    += len;
            length    -= len;
        }

        if( length > 0 ) {
//qlog_info() <<  " - 2: memcpy:  offset="<<offset << "gapSize=" << gapSize()<< ", length=" << length << items_[offset + gapSize()];
            memcpy( data, items_ + offset + gapSize(), sizeof(T)*length );
        }
    }
*/

    /// This method returns a direct pointer to the 0-terminated buffer
    /// This pointer is only valid as long as the buffer doesn't change
    /// WARNING, this method MOVES the gap! Which means this method should NOT be used for a lot of operations
/*
    T* data() {
        ensureGapSize(1);
        moveGapTo( length() );
        items_[length()] = QChar(); // a \0 character
        return items_;
    }
*/


    //// moves the gap to the given position
    //// Warning when the gap is moved after the length the gap shrinks
/*
    void moveGapTo( int offset ) {
        Q_ASSERT( offset <= capacity_);
        Q_ASSERT( offset <= length() );
        if( offset != gapBegin_ ) {
//qlog_info() << "BEGIN moveGapTo: offset=" << offset << "/ gapBegin_"; // << gapBegin_ << getUnitTestString();
            int gapSize = this->gapSize();

            // move the the data right after the gap
            if (offset < gapBegin_ ) {
//qlog_info() << "- move"  << offset << "," << gapBegin_ << "(charcount:" << (gapBegin_ - offset)<<")";
                memmove( items_ + offset + gapSize, items_ + offset, sizeof(T) * (gapBegin_ - offset));   // memmove( target, source, size )

            } else {
//qlog_info() << "- move2: gapBegin_" << gapBegin_ << ", gapEnd_" << gapEnd_ << ", capacity_" << capacity_ << ", gapSize" << gapSize << "(charcount:"<<(offset - gapBegin_)<<")";
                memmove( items_ + gapBegin_, items_ + gapEnd_, sizeof(T) * (offset - gapBegin_ ));  // memmove( target, source, size )
            }
            gapBegin_ = offset;
            gapEnd_   = gapBegin_ + gapSize; //qMin( gapBegin_ + gapSize, capacity_ );
        }
        Q_ASSERT( gapBegin_ <= gapEnd_ );

    }


    /// this method makes sure there's enough room for the insertation
    void ensureGapSize( int requiredSize ) {
        if( gapSize() < requiredSize ) {
            while( growSize_ < capacity_ / 6) { growSize_ *= 2; }
            resize(capacity_ + requiredSize + growSize_ - gapSize() );
        }
    }


    /// resizes the array of data
    void resize(int newSize)
    {
        if( capacity_ >= newSize) return;
        int lengte = length();
        Q_ASSERT( lengte <= capacity_);
/// TODO: optimize, so data is moved only once
/// in other words, gap movement is not required over here!!
/// this can be done with 2 memcopies
//qlog_info() << "BEGIN resize: capacity =" << capacity_<< " => " << newSize;


        moveGapTo( lengte );
        T *newChars = new T[ newSize ];

        if( capacity_ > 0 ) {
            memmove( newChars, items_, sizeof(T)*lengte );
            delete[] items_;
        }
        items_ = newChars;
        capacity_ = newSize;
        gapEnd_   = newSize;
//qlog_info() << "END resize";
    }


    /// sets the growsize. The growsize if the amount to reserve extra
    void setGrowSize( int size ) { growSize_=size; }

    /// returns the growsize
    int growSize() { return growSize_; }


    /// Converts the 'gap-buffer' to a unit-test debugging string
    QString getUnitTestString( QChar gapChar = '_' ) const {
        QString s;
        int gapBegin = this->gapBegin();
        int gapEnd   = this->gapEnd();
        int capacity = this->capacity();

        for( int i=0; i<gapBegin; ++i ) {
            if( items_[i].isNull() ) {
                s.append("@");
            } else {
                s.append( items_[i] );
            }
        }
        s.append( "[" );
        for( int i=gapBegin; i<gapEnd; ++i ) {
            s.append( gapChar );
        }
        s.append( ">" );
        for( int i=gapEnd; i<capacity; ++i ) {
            if( items_[i].isNull() ) {
                s.append("@");
            } else {
                s.append( items_[i] );
            }
        }

        return s;
    }
*/


protected:

    QVector<T> items_;
};



} // edbee
