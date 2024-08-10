#include "customprinter.h"
#include <QCoreApplication>
#include <QTimer>
#include <gtest/gtest.h>

void installCustomTestPrinter(testing::TestEventListener *listener)
{
  testing::TestEventListeners &listeners
      = testing::UnitTest::GetInstance()->listeners();
  delete listeners.Release(listeners.default_result_printer());
  listeners.Append(listener);
}

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);
  testing::InitGoogleTest(&argc, argv);
  installCustomTestPrinter(new CustomPrinter);
  int results = RUN_ALL_TESTS();
  QTimer::singleShot(0, &app, &QCoreApplication::quit);
  app.exec();
  return results;
}
