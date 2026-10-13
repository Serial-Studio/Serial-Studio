/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
// clang-format on

#include <QJSEngine>
#include <QJSValue>
#include <QString>
#include <QStringList>
#include <QTest>

#include "Core/Checksum.h"
#include "DataModel/Scripting/ChecksumApi.h"

//--------------------------------------------------------------------------------------------------
// Fixtures: the same "123456789" check input tst_checksums pins the registry with
//--------------------------------------------------------------------------------------------------

[[nodiscard]] static lua_State* newLuaState()
{
  lua_State* L = luaL_newstate();
  if (!L)
    return nullptr;

  luaL_openlibs(L);
  DataModel::ChecksumApi::installLua(L);
  return L;
}

[[nodiscard]] static int runChunk(lua_State* L, const char* code)
{
  if (luaL_loadstring(L, code) != LUA_OK)
    return LUA_ERRSYNTAX;

  return lua_pcall(L, 0, LUA_MULTRET, 0);
}

[[nodiscard]] static QString luaTopString(lua_State* L)
{
  size_t length    = 0;
  const char* data = lua_tolstring(L, -1, &length);
  return QString::fromLatin1(data ? data : "", static_cast<qsizetype>(length));
}

[[nodiscard]] static int registryAlgorithmCount()
{
  int count = 0;
  for (const auto& name : IO::availableChecksums())
    if (!name.isEmpty())
      ++count;

  return count;
}

//--------------------------------------------------------------------------------------------------
// Test class
//--------------------------------------------------------------------------------------------------

class ChecksumApiTests : public QObject {
  Q_OBJECT

private slots:
  void luaHexOfAString();
  void luaBytesInWireOrder();
  void luaTableOfBytes();
  void luaUnknownAlgorithmIsAnError();
  void luaBadDataIsAnError();
  void luaAlgorithmListMatchesTheRegistry();

  void jsHexOfAString();
  void jsBytesAsBinaryString();
  void jsArrayOfBytes();
  void jsUnknownAlgorithmThrows();
  void jsBadDataThrows();
  void jsAlgorithmListMatchesTheRegistry();
  void jsInstallIsIdempotent();
};

//--------------------------------------------------------------------------------------------------
// Lua
//--------------------------------------------------------------------------------------------------

void ChecksumApiTests::luaHexOfAString()
{
  lua_State* L = newLuaState();
  QVERIFY(L != nullptr);

  QCOMPARE(runChunk(L, "return checksumHex('CRC-16', '123456789')"), int(LUA_OK));
  QCOMPARE(luaTopString(L), QStringLiteral("29b1"));
  lua_settop(L, 0);

  QCOMPARE(runChunk(L, "return checksumHex('CRC-8', '123456789')"), int(LUA_OK));
  QCOMPARE(luaTopString(L), QStringLiteral("f7"));

  lua_close(L);
}

void ChecksumApiTests::luaBytesInWireOrder()
{
  lua_State* L = newLuaState();
  QVERIFY(L != nullptr);

  QCOMPARE(runChunk(L, "local s = checksum('CRC-16', '123456789') return #s, s:byte(1), s:byte(2)"),
           int(LUA_OK));
  QCOMPARE(static_cast<int>(lua_tointeger(L, -3)), 2);
  QCOMPARE(static_cast<int>(lua_tointeger(L, -2)), 0x29);
  QCOMPARE(static_cast<int>(lua_tointeger(L, -1)), 0xb1);
  lua_settop(L, 0);

  QCOMPARE(runChunk(L, "return #checksum('CRC-32', '123456789')"), int(LUA_OK));
  QCOMPARE(static_cast<int>(lua_tointeger(L, -1)), 4);

  lua_close(L);
}

void ChecksumApiTests::luaTableOfBytes()
{
  lua_State* L = newLuaState();
  QVERIFY(L != nullptr);

  QCOMPARE(runChunk(L, "return checksumHex('XOR-8', {0x31, 0x32, 0x33})"), int(LUA_OK));
  QCOMPARE(luaTopString(L), QStringLiteral("30"));
  lua_settop(L, 0);

  QCOMPARE(
    runChunk(L, "return checksumHex('XOR-8', {0x31, 0x32, 0x33}) == checksumHex('XOR-8', '123')"),
    int(LUA_OK));
  QVERIFY(lua_toboolean(L, -1));

  lua_close(L);
}

void ChecksumApiTests::luaUnknownAlgorithmIsAnError()
{
  lua_State* L = newLuaState();
  QVERIFY(L != nullptr);

  QCOMPARE(runChunk(L, "return checksum('CRC-99', 'abc')"), int(LUA_ERRRUN));
  QVERIFY(luaTopString(L).contains(QStringLiteral("unknown checksum algorithm")));
  lua_settop(L, 0);

  QCOMPARE(runChunk(L, "return checksum('', 'abc')"), int(LUA_ERRRUN));

  lua_close(L);
}

void ChecksumApiTests::luaBadDataIsAnError()
{
  lua_State* L = newLuaState();
  QVERIFY(L != nullptr);

  QCOMPARE(runChunk(L, "return checksum('CRC-16', 42)"), int(LUA_ERRRUN));
  QVERIFY(luaTopString(L).contains(QStringLiteral("byte values")));

  lua_close(L);
}

void ChecksumApiTests::luaAlgorithmListMatchesTheRegistry()
{
  lua_State* L = newLuaState();
  QVERIFY(L != nullptr);

  QCOMPARE(runChunk(L, "return #checksumAlgorithms()"), int(LUA_OK));
  QCOMPARE(static_cast<int>(lua_tointeger(L, -1)), registryAlgorithmCount());
  lua_settop(L, 0);

  QCOMPARE(
    runChunk(L, "for _, n in ipairs(checksumAlgorithms()) do assert(#n > 0) end return true"),
    int(LUA_OK));

  lua_close(L);
}

//--------------------------------------------------------------------------------------------------
// JavaScript
//--------------------------------------------------------------------------------------------------

void ChecksumApiTests::jsHexOfAString()
{
  QJSEngine engine;
  DataModel::ChecksumApi::installJS(&engine);

  const auto hex = engine.evaluate(QStringLiteral("__ss_ck.checksumHex('CRC-16', '123456789')"));
  QVERIFY2(!hex.isError(), qPrintable(hex.toString()));
  QCOMPARE(hex.toString(), QStringLiteral("29b1"));

  const auto crc8 = engine.evaluate(QStringLiteral("__ss_ck.checksumHex('CRC-8', '123456789')"));
  QCOMPARE(crc8.toString(), QStringLiteral("f7"));
}

void ChecksumApiTests::jsBytesAsBinaryString()
{
  QJSEngine engine;
  DataModel::ChecksumApi::installJS(&engine);

  const auto result =
    engine.evaluate(QStringLiteral("var s = __ss_ck.checksum('CRC-16', '123456789');"
                                   "[s.length, s.charCodeAt(0), s.charCodeAt(1)]"));
  QVERIFY2(!result.isError(), qPrintable(result.toString()));
  QCOMPARE(result.property(0).toInt(), 2);
  QCOMPARE(result.property(1).toInt(), 0x29);
  QCOMPARE(result.property(2).toInt(), 0xb1);

  const auto modbus =
    engine.evaluate(QStringLiteral("var m = __ss_ck.checksum('CRC-16-MODBUS', '123456789');"
                                   "var h = __ss_ck.checksumHex('CRC-16-MODBUS', '123456789');"
                                   "(m.charCodeAt(0) << 8 | m.charCodeAt(1)).toString(16) === h"));
  QVERIFY(modbus.toBool());
}

void ChecksumApiTests::jsArrayOfBytes()
{
  QJSEngine engine;
  DataModel::ChecksumApi::installJS(&engine);

  const auto fromArray =
    engine.evaluate(QStringLiteral("__ss_ck.checksumHex('XOR-8', [0x31, 0x32, 0x33])"));
  QCOMPARE(fromArray.toString(), QStringLiteral("30"));

  const auto fromTyped = engine.evaluate(
    QStringLiteral("__ss_ck.checksumHex('XOR-8', new Uint8Array([0x31, 0x32, 0x33]))"));
  QVERIFY2(!fromTyped.isError(), qPrintable(fromTyped.toString()));
  QCOMPARE(fromTyped.toString(), QStringLiteral("30"));

  const auto fromString = engine.evaluate(QStringLiteral("__ss_ck.checksumHex('XOR-8', '123')"));
  QCOMPARE(fromString.toString(), QStringLiteral("30"));
}

void ChecksumApiTests::jsUnknownAlgorithmThrows()
{
  QJSEngine engine;
  DataModel::ChecksumApi::installJS(&engine);

  const auto result = engine.evaluate(QStringLiteral("__ss_ck.checksum('CRC-99', 'abc')"));
  QVERIFY(result.isError());
  QVERIFY(result.toString().contains(QStringLiteral("unknown checksum algorithm")));

  const auto blank = engine.evaluate(QStringLiteral("__ss_ck.checksum('', 'abc')"));
  QVERIFY(blank.isError());
}

void ChecksumApiTests::jsBadDataThrows()
{
  QJSEngine engine;
  DataModel::ChecksumApi::installJS(&engine);

  const auto result = engine.evaluate(QStringLiteral("__ss_ck.checksum('CRC-16', 42)"));
  QVERIFY(result.isError());
  QVERIFY(result.toString().contains(QStringLiteral("byte values")));
}

void ChecksumApiTests::jsAlgorithmListMatchesTheRegistry()
{
  QJSEngine engine;
  DataModel::ChecksumApi::installJS(&engine);

  const auto names = engine.evaluate(QStringLiteral("__ss_ck.checksumAlgorithms()"));
  QCOMPARE(names.property(QStringLiteral("length")).toInt(), registryAlgorithmCount());
  QVERIFY(engine.evaluate(QStringLiteral("__ss_ck.checksumAlgorithms().indexOf('CRC-16') >= 0"))
            .toBool());
}

void ChecksumApiTests::jsInstallIsIdempotent()
{
  QJSEngine engine;
  DataModel::ChecksumApi::installJS(&engine);
  const auto first = engine.globalObject().property(QStringLiteral("__ss_ck")).toQObject();

  DataModel::ChecksumApi::installJS(&engine);
  const auto second = engine.globalObject().property(QStringLiteral("__ss_ck")).toQObject();

  QVERIFY(first != nullptr);
  QCOMPARE(first, second);
}

QTEST_GUILESS_MAIN(ChecksumApiTests)
#include "tst_checksum_api.moc"
