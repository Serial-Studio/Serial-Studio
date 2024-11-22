/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QTextStream>


/** @TODO: Replace these tests with the standard Qt testing framework */

//=============================================================================
// MACROS
//=============================================================================

#define TEST_CONCATENATE_DETAIL(x, y) x##y
#define TEST_CONCATENATE(x, y) TEST_CONCATENATE_DETAIL(x, y)
#define TEST_MAKE_UNIQUE(x) TEST_CONCATENATE(x, __COUNTER__)

#define DECLARE_TEST(className) static edbee::test::Test<className> TEST_MAKE_UNIQUE(t)(#className)
#define DECLARE_NAMED_TEST(name,className) static edbee::test::Test<className> TEST_MAKE_UNIQUE(name)(#className)


namespace edbee { namespace test {


class OutputHandler;
class TestEngine;
class TestCase;

//=============================================================================
// Test Results
//=============================================================================

template <class T>
QString toQString( const T& obj )
{
    return QStringLiteral("%1").arg(obj);
}


/// This method represents a test result
class EDBEE_EXPORT TestResult {
public:
    enum Status {
        Passed, Failed, Skipped
    };

    explicit TestResult( TestCase* testCase, const QString& methodName, const QString& description, const char* file, int lineNumber );
    virtual ~TestResult() {}

    virtual void setBooleanResult( bool result, const char *statement  );
    //virtual void setCompareResult( const QString& actualValue, const QString& expectedValue, const char* actualStatement, const char* expectedStatement );

    virtual void setCompareResult( bool result, const QString& actualValue, const QString& expectedValue, const char* actualStatement, const char* expectedStatement );

//    template <class T>
//    void setCompareResult( const T& actualValue, const T& expectedValue, const char* actualStatement, const char* expectedStatement )
//    {
//        setCompareResult( actualValue == expectedValue, toQString(actualValue), toQString(expectedValue), actualStatement, expectedStatement );
//    }

    virtual void setSkip();

    virtual TestCase* testCae() { return testCaseRef_; }
    virtual QString methodName() { return methodName_; }
    virtual QString description() { return description_; }
    virtual const char* fileName() { return fileNameRef_; }
    virtual int lineNumber() { return lineNumber_; }

    virtual bool compareStatement() { return compareStatement_; }
    virtual const char* actualStatement() { return actualStatementRef_; }
    virtual const char* statement() { return actualStatementRef_; }
    virtual const char* expectedStatement() { return expectedStatementRef_; }
    virtual QString actualValue() { return actualValue_; }
    virtual QString expectedValue() { return expectedValue_; }

    virtual Status status() { return status_; }


private:

    // method / line defionition
    TestCase* testCaseRef_;         ///< A reference to the test object
    QString  methodName_;           ///< The method name
    QString description_;           ///< The message
    const char* fileNameRef_;       ///< The filename
    int lineNumber_;                ///< The line number

    // code definition
    bool compareStatement_;            ///< A compare statement with an expected and actual
    const char* actualStatementRef_;   ///< The actual statement
    const char* expectedStatementRef_; ///< The expected statement
    QString actualValue_;              ///< The actual value
    QString expectedValue_;            ///< The expected value

    // the result
    Status status_;                    ///< The test status
};




//=============================================================================
// Test Unit Class
//=============================================================================

#define GET_3TH_ARG(arg1, arg2, arg3, ...) arg3
#define GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4


#define testTrue_1(statement)                testTrueImpl( ( statement ) ? true : false, #statement, "", __FILE__, __LINE__)
#define testTrue_2(statement, message)       testTrueImpl( ( statement ) ? true : false, #statement, (message), __FILE__, __LINE__ )
#define testTrue_MACRO_CHOOSER(...) \
    GET_3TH_ARG(__VA_ARGS__, testTrue_2, testTrue_1, )
#define testTrue(...) testTrue_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)


#define testFalse(statement)    testTrue( !statement )


#define testEqual_1(actual,expected) do { \
    QString actualStr = edbee::test::toQString(actual); \
    QString expectedStr = edbee::test::toQString(expected); \
    testEqualImpl( ( actualStr ) == ( expectedStr ), actualStr, expectedStr, #actual, #expected, "", __FILE__, __LINE__ ); \
} while( false )

#define testEqual_2(actual,expected, message) do \
    QString actualStr = edbee::test::toQString(actual); \
    QString expectedStr = edbee::test::toQString(expected); \
    testEqualImpl( ( actualStr ) == ( expectedStr ), actualStr, expectedStr, #actual, #expected, (message), __FILE__, __LINE__ ); \
} while( false )


#define testEqual_MACRO_CHOOSER(...) \
    GET_4TH_ARG(__VA_ARGS__, testEqual_2, testEqual_1, )
#define testEqual(...) testEqual_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)


#define testSkip(msg) testSkipImpl( msg, __FILE__, __LINE__)





/// I really really hate the QTestLib output on my Mac. It's a very ugly xwindows console result.
///
/// This extemely simple testing support class makes it possible for met to output the test results
/// to a WebKit window. Where I can added clickable divs etc. to show test result details
///
/// The basic conventions for these tests are the sames as those of QTest. The main difference is the usage
/// of exta output/compare info output messsages

/// A simple unit test. Every private slot is called for a test
///
class EDBEE_EXPORT TestCase : public QObject
{
    Q_OBJECT
public:
    explicit TestCase();

    /// This method returns the current engine
    virtual TestEngine* engine() { return engineRef_; }
    virtual void setEngine( TestEngine *engine ) { engineRef_ = engine; }

    virtual OutputHandler* out();


    virtual void testTrueImpl( bool condition, const char* statement, const QString& description, const char* file, int line );


    virtual void testEqualImpl( bool result, const QString& actual, const QString& expected,
                                    const char* actualStatement, const char* expectedStatement,
                                    const QString& description, const char* file, int line );

    virtual void testSkipImpl( const QString& description, const char* file, int line );


    virtual TestCase* currentTest(); // usually returns this :P
    virtual QString currentMethodName();
    virtual void giveTestResultToEngine( TestResult* result );


signals:

public slots:

private slots:

private:

    TestEngine *engineRef_;       ///< This method returns the reference to the engine
};


//=============================================================================
// Output Handling
//=============================================================================

/// This is the basic outputhandler.
/// The basic outputhandler simply executes a qDebug with the given information
class EDBEE_EXPORT OutputHandler {
public:

    explicit OutputHandler();
    virtual ~OutputHandler();

    // writes down the start of a multiple test run
    virtual void startTestRun( TestEngine* engine );
    virtual void endTestRun( TestEngine* engine );


    // writes down the 'category' of the test
    virtual void startTestCase( TestEngine* engine );
    virtual void endTestCase( TestEngine* engine );


    // method output of the test case
    virtual void startTestMethod( TestEngine* engine );
    virtual void endTestMethod( TestEngine* engine );

    virtual void testResultAdded( TestEngine* engine, TestResult* testResult );



private:

    QString buffer_;
    QString failBuffer_;
    QString outBuffer_;

    // this method notifies that a value is wrong
//    void writeSuccess( TestCase & testCase, const char *methodName );

    // fails with the given messages
    //void fail( const QString & message, const QString & details = ""  ) ;
};




//=============================================================================
// Test Engine
//=============================================================================

/// This is the main test engine
class EDBEE_EXPORT TestEngine : public QObject
{
    Q_OBJECT
public:

    explicit TestEngine();
    virtual ~TestEngine();

    virtual bool hasTest(TestCase* object);
    virtual void addTest(TestCase* object);
    virtual int runAll();
    virtual int run( TestCase* object );
    virtual int run( const QString& name );
    virtual void startRun();
    virtual void endRun();

    virtual OutputHandler* outputHandler() { return outputHandlerRef_; }
    virtual void setOutputHandler( OutputHandler* handler ) { outputHandlerRef_ = handler; }

    /// This method returns the current test
    virtual TestCase* currentTest() { return currentTestRef_; }

    /// This method returns the current class name
    virtual const char* currentClassName() { return currentTestRef_->metaObject()->className(); }

    /// This method returns the current method name
    virtual QString currentMethodName() { return currentMethodName_; }

    virtual void giveTestResult( TestResult* testResult );

    /// This method returns a list of test results
    virtual QList<TestResult*> testResultList() { return testResultList_; }

private:

    QList<TestCase*> testRefList_;     ///< All registered test libs
    OutputHandler* outputHandlerRef_;  ///< The output handler

    // the current state
    TestCase* currentTestRef_;           ///< the current test
    QString currentMethodName_;          ///< The current method name
//    const char* currentMethodNameRef_;  ///< A reference to the current method

    // test results
    QList<TestResult*> testResultList_;  ///< the test results
};


//=============================================================================
// Appending Tests via a Macro
//=============================================================================

// this method returns the test engine
inline TestEngine& engine()
{
    static TestEngine engine;
    return engine;
}


template <class T>
class EDBEE_EXPORT Test {
public:
    QSharedPointer<T> child;

    Test(const QString& name) : child(new T)
    {
        child->setObjectName(name);
        engine().addTest(child.data());
    }
};


}} // edbee::test
