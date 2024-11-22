/*
 * Copyright (C) 2008  Remko Troncon
 * Licensed under the MIT license.
 * See COPYING for license details.
 */

#include <QCoreApplication>

#include "QtTestUtil/TestRegistry.h"

/**
 * Runs all tests registered with the QtTestUtil registry.
 */
int main(int argc, char* argv[]) {
	QCoreApplication application(argc, argv);
	return QtTestUtil::TestRegistry::getInstance()->runTests(argc, argv);
}
