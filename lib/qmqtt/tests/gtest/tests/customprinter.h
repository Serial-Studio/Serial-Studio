#ifndef CUSTOM_PRINTER_H
#define CUSTOM_PRINTER_H

#include <QTextStream>
#include <gtest/gtest.h>

class CustomPrinter : public testing::EmptyTestEventListener
{
public:
  CustomPrinter();

  QTextStream qout;
  QTextStream qerr;

  void PrintTestName(const char *test_case, const char *test)
  {
    qout << test_case << "." << test;
  }

  virtual void OnTestProgramStart(const testing::UnitTest & /*unit_test*/) {}
  virtual void OnTestIterationStart(const testing::UnitTest &unit_test,
                                    int iteration);
  virtual void OnEnvironmentsSetUpStart(const testing::UnitTest &unit_test);
  virtual void OnEnvironmentsSetUpEnd(const testing::UnitTest & /*unit_test*/)
  {
  }
  virtual void OnTestCaseStart(const testing::TestCase &test_case);
  virtual void OnTestStart(const testing::TestInfo &test_info);
  // virtual void OnTestPartResult(const testing::TestPartResult& result);
  virtual void OnTestEnd(const testing::TestInfo &test_info);
  virtual void OnTestCaseEnd(const testing::TestCase &test_case);
  virtual void OnEnvironmentsTearDownStart(const testing::UnitTest &unit_test);
  virtual void
  OnEnvironmentsTearDownEnd(const testing::UnitTest & /*unit_test*/)
  {
  }
  virtual void OnTestIterationEnd(const testing::UnitTest &unit_test,
                                  int iteration);
  virtual void OnTestProgramEnd(const testing::UnitTest & /*unit_test*/) {}

private:
  static void PrintFailedTests(const testing::UnitTest &unit_test);
};

#endif // CUSTOM_PRINTER_H
