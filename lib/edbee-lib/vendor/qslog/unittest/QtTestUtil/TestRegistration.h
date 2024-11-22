/*
 * Copyright (C) 2008  Remko Troncon
 * Licensed under the MIT license.
 * See COPYING for license details.
 */

#ifndef QtTestUtil_TestRegistration_H
#define QtTestUtil_TestRegistration_H

#include "QtTestUtil/TestRegistry.h"

namespace QtTestUtil {

	/**
	 * A wrapper class around a test to manage registration and static
	 * creation of an instance of the test class.
	 * This class is used by QTTESTUTIL_REGISTER_TEST(), and you should not 
	 * use this class directly.
	 */
	template<typename TestClass>
	class TestRegistration {
		public:
			TestRegistration() {
				test_ = new TestClass();
				TestRegistry::getInstance()->registerTest(test_);
			}

			~TestRegistration() {
				delete test_;
			}
		
		private:
			TestClass* test_;
	};

}

#endif
