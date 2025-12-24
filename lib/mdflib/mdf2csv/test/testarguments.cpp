/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/
#include <gtest/gtest.h>

#include "programargument.h"


namespace mdf::test {

TEST(TestArguments, TestEmpty)  {
  char *argv[1] = {
    const_cast<char*>("c:\\Path1\\execute1.exe"),
  };
  constexpr int argc = 1;

  ProgramArgument& arguments = ProgramArgument::Instance();
  arguments.Parse(argc, argv);

  EXPECT_TRUE(arguments.Help());
}

TEST(TestArguments, TestHelp1)  {
  char *argv[] = {
      const_cast<char*>("c:\\Path1\\execute1.exe"),
      const_cast<char*>("--help"),
  };
  constexpr int argc = 2;

  ProgramArgument& arguments = ProgramArgument::Instance();
  arguments.Parse(argc, argv);

  EXPECT_TRUE(arguments.Help());
}

TEST(TestArguments, TestHelp2)  {
  char *argv[] = {
      const_cast<char*>("c:\\Path1\\execute1.exe"),
      const_cast<char*>("-h"),
  };
  constexpr int argc = 2;

  ProgramArgument& arguments = ProgramArgument::Instance();
  arguments.Parse(argc, argv);

  EXPECT_TRUE(arguments.Help());
}

TEST(TestArguments, TestVersionLong)  {
  char *argv[] = {
      const_cast<char*>("c:\\Path1\\execute1.exe"),
      const_cast<char*>("--version"),
  };
  constexpr int argc = 2;

  ProgramArgument& arguments = ProgramArgument::Instance();
  arguments.Parse(argc, argv);

  EXPECT_FALSE(arguments.Help());
  EXPECT_TRUE(arguments.Version());
}

TEST(TestArguments, TestVersionShort)  {
  char *argv[] = {
      const_cast<char*>("c:\\Path1\\execute1.exe"),
      const_cast<char*>("-V"),
  };
  constexpr int argc = 2;

  ProgramArgument& arguments = ProgramArgument::Instance();
  arguments.Parse(argc, argv);

  EXPECT_FALSE(arguments.Help());
  EXPECT_TRUE(arguments.Version());
}

TEST(TestArguments, TestArgDefault)  {
  char *argv[] = {
      const_cast<char*>("c:\\Path1\\execute1.exe"),
      const_cast<char*>("olle.mf4"),
      const_cast<char*>("pelle.mf4"),
  };
  constexpr int argc = 3;

  ProgramArgument& arguments = ProgramArgument::Instance();
  arguments.Parse(argc, argv);

  EXPECT_FALSE(arguments.Help());
  EXPECT_FALSE(arguments.Version());
  EXPECT_EQ(arguments.LogLevel(), -1);
  EXPECT_FALSE(arguments.NonInteractive());
  EXPECT_FALSE(arguments.DeleteConverted());
  EXPECT_FALSE(arguments.NoAppendRoot());
  EXPECT_EQ(arguments.BufferSize(), -1);
  EXPECT_TRUE(arguments.PasswordFile().empty());
  EXPECT_TRUE(arguments.OutputDirectory().empty());
  EXPECT_EQ(arguments.InputFiles().size(), 2);
}

TEST(TestArguments, TestArgLong)  {
  char *argv[] = {
      const_cast<char*>("c:\\Path1\\execute1.exe"),
      const_cast<char*>("--verbosity=4"),
      const_cast<char*>("--non-interactive"),
      const_cast<char*>("--delete-converted"),
      const_cast<char*>("--no-append-root"),
      const_cast<char*>("--buffer=100"),
      const_cast<char*>("--password-file=\"pass 1234.txt\""),
      const_cast<char*>("--output-directory=\"outdir\""),
      const_cast<char*>(R"(--input-files="file 1.mf4")"),
  };
  constexpr int argc = 9;

  ProgramArgument& arguments = ProgramArgument::Instance();
  arguments.Parse(argc, argv);

  EXPECT_FALSE(arguments.Help());
  EXPECT_FALSE(arguments.Version());
  EXPECT_EQ(arguments.LogLevel(), 4);
  EXPECT_TRUE(arguments.NonInteractive());
  EXPECT_TRUE(arguments.DeleteConverted());
  EXPECT_TRUE(arguments.NoAppendRoot());
  EXPECT_EQ(arguments.BufferSize(), 100);
  EXPECT_STREQ(arguments.PasswordFile().c_str(), "pass 1234.txt" );
  EXPECT_STREQ(arguments.OutputDirectory().c_str(), "outdir");
  EXPECT_EQ(arguments.InputFiles().size(), 1);
}


TEST(TestArguments, TestArgShort)  {
  char *argv[] = {
      const_cast<char*>("c:\\Path1\\execute1.exe"),
      const_cast<char*>("-v4"),
      const_cast<char*>("--quiet"),
      const_cast<char*>("--delete-converted"),
      const_cast<char*>("--no-append-root"),
      const_cast<char*>("--buffer=100"),
      const_cast<char*>("--password-file=\"pass 1234.txt\""),
      const_cast<char*>("--output-directory=\"outdir\""),
      const_cast<char*>(R"(--input-files="file 1.mf4")"),
  };
  constexpr int argc = 9;

  ProgramArgument& arguments = ProgramArgument::Instance();
  arguments.Parse(argc, argv);

  EXPECT_FALSE(arguments.Help());
  EXPECT_FALSE(arguments.Version());
  EXPECT_EQ(arguments.LogLevel(), 4);
  EXPECT_TRUE(arguments.NonInteractive());
  EXPECT_TRUE(arguments.DeleteConverted());
  EXPECT_TRUE(arguments.NoAppendRoot());
  EXPECT_EQ(arguments.BufferSize(), 100);
  EXPECT_STREQ(arguments.PasswordFile().c_str(), "pass 1234.txt" );
  EXPECT_STREQ(arguments.OutputDirectory().c_str(), "outdir");
  EXPECT_EQ(arguments.InputFiles().size(), 1);
}


}
