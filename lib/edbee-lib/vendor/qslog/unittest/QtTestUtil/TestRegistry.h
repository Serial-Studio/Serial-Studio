/*
 * Copyright (C) 2008  Remko Troncon
 * Licensed under the MIT license.
 * See COPYING for license details.
 */

#ifndef QtTestUtil_TestRegistry_H
#define QtTestUtil_TestRegistry_H

#include <QList>

class QObject;

namespace QtTestUtil {
	
	/**
	 * A registry of QtTest test classes.
	 * All test classes registered with QTTESTUTIL_REGISTER_TEST add 
	 * themselves to this registry. All registered tests can then be run at 
	 * once using runTests().
	 */
	class TestRegistry {
		public:
			/**
			 * Retrieve the single instance of the registry.
			 */
			static TestRegistry* getInstance();

			/**
			 * Register a QtTest test. 
			 * This method is called  by QTTESTUTIL_REGISTER_TEST, and you should 
			 * not use this method directly.
			 */
			void registerTest(QObject*);

			/**
			 * Run all registered tests using QTest::qExec()
			 */
			int runTests(int argc, char* argv[]);

		private:
			TestRegistry() {}
		
		private:
			QList<QObject*> tests_;
	};
}

#endif
