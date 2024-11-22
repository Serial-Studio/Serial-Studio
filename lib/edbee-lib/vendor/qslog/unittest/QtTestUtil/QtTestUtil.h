/*
 * Copyright (C) 2008  Remko Troncon
 * Licensed under the MIT license.
 * See COPYING for license details.
 */

#ifndef QtTestUtil_H
#define QtTestUtil_H

#include <QObject>
#include <QtTest/QtTest>
#include "QtTestUtil/TestRegistration.h"

/**
 * A macro to register a test class.
 *
 * This macro will create a static variable which registers the
 * testclass with the TestRegistry, and creates an instance of the 
 * test class.
 *
 * Execute this macro in the body of your unit test's .cpp file, e.g.
 *    class MyTest {
 *			...
 *		};
 *
 *		QTTESTUTIL_REGISTER_TEST(MyTest)
 */
#define QTTESTUTIL_REGISTER_TEST(TestClass) \
	static QtTestUtil::TestRegistration<TestClass> TestClass##Registration

#endif
