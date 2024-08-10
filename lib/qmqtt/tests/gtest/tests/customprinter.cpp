#include "customprinter.h"

using namespace testing;

CustomPrinter::CustomPrinter()
  : qout(stdout)
  , qerr(stderr)
{
}

void CustomPrinter::OnTestIterationStart(const UnitTest &unit_test,
                                         int iteration)
{
  if (GTEST_FLAG(repeat) != 1)
  {
    printf("\nRepeating all tests (iteration %d) . . .\n\n", iteration + 1);
  }

  if (GTEST_FLAG(shuffle))
  {
    qout << "Note: Randomizing tests' orders with a seed of "
         << unit_test.random_seed() << " ." << endl;
  }

  qout << "[==========] Running "
       << (unit_test.test_to_run_count() == 1 ? "test" : "tests") << " from "
       << unit_test.test_case_to_run_count() << "." << endl;
  qout.flush();
}

void CustomPrinter::OnEnvironmentsSetUpStart(const UnitTest & /*unit_test*/)
{
  qout << "[----------] Global test environment set-up." << endl;
  qout.flush();
}

void CustomPrinter::OnTestCaseStart(const TestCase &test_case)
{
  qout << "[----------] "
       << (test_case.test_to_run_count() == 1 ? "test" : "tests") << " from "
       << test_case.name();
  if (nullptr == test_case.type_param())
  {
    qout << endl;
  }
  else
  {
    qout << ", where TypeParam = " << test_case.type_param() << endl;
  }
  qout.flush();
}

void CustomPrinter::OnTestStart(const TestInfo &test_info)
{
  qout << "[ RUN      ] ";
  PrintTestName(test_info.test_case_name(), test_info.name());
  qout << endl;
  qout.flush();
}

void CustomPrinter::OnTestEnd(const TestInfo &test_info)
{
  const TestResult *result = test_info.result();
  if (!result->Passed())
  {
    for (int i = 0; i < result->total_part_count(); i++)
    {
      TestPartResult part = result->GetTestPartResult(i);
      qerr << part.file_name() << ":" << qMax(part.line_number(), 0) << ":"
           << "0: error: Test failed: " << test_info.test_case_name() << "."
           << test_info.name() << endl
           << part.summary() << endl;
    }
  }

  if (test_info.result()->Passed())
  {
    qout << "[       OK ] ";
  }
  else
  {
    qout << "[  FAILED  ] ";
  }
  PrintTestName(test_info.test_case_name(), test_info.name());
  if (test_info.result()->Failed())
  {
    if (nullptr != test_info.type_param() || nullptr != test_info.value_param())
    {
      qout << ", where ";
      if (nullptr != test_info.type_param())
      {
        qout << "TypeParam = " << test_info.type_param();
        if (nullptr != test_info.value_param())
        {
          qout << " and ";
        }
      }
      if (nullptr != test_info.value_param())
      {
        qout << "GetParam() = " << test_info.value_param();
      }
    }
  }

  if (GTEST_FLAG(print_time))
  {
    qout << " (" << test_info.result()->elapsed_time() << ")" << endl;
  }
  else
  {
    qout << endl;
  }
  qout.flush();
  qerr.flush();
}

void CustomPrinter::OnTestCaseEnd(const TestCase &test_case)
{
  if (GTEST_FLAG(print_time))
  {
    qout << "[----------] "
         << (test_case.test_to_run_count() == 1 ? "test" : "tests") << " from "
         << test_case.name() << " (" << test_case.elapsed_time() << " ms total)"
         << endl
         << endl;
    qout.flush();
  }
}

void CustomPrinter::OnEnvironmentsTearDownStart(const UnitTest & /*unit_test*/)
{
  qout << "[----------] Global test environment tear-down" << endl;
  qout.flush();
}

void CustomPrinter::OnTestIterationEnd(const UnitTest &unit_test,
                                       int /*iteration*/)
{
  qout << "[==========] "
       << (unit_test.test_to_run_count() == 1 ? "test" : "tests") << " from "
       << (unit_test.test_case_to_run_count() == 1 ? "test case" : "test cases")
       << (GTEST_FLAG(print_time)
               ? QString("(%1 ms total)").arg(unit_test.elapsed_time())
               : "")
       << endl;

  qout << "[  PASSED  ] "
       << (unit_test.successful_test_count() == 1 ? "test." : "tests.") << endl;

  int num_failures = unit_test.failed_test_count();
  if (!unit_test.Passed())
  {
    qout << "[  FAILED  ] " << num_failures << " FAILED"
         << (num_failures == 1 ? "TEST" : "TESTS") << endl;
  }

  int num_disabled = unit_test.reportable_disabled_test_count();
  if (num_disabled && !GTEST_FLAG(also_run_disabled_tests))
  {
    if (!num_failures)
    {
      qout << endl;
    }
    qout << "  YOU HAVE " << num_disabled << " DISABLED "
         << (num_disabled == 1 ? "TEST" : "TESTS") << endl
         << endl;
  }
  qout.flush();
}
