/*
 * Copyright (c) 2020 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "JsonParser.h"
#include "SerialManager.h"

/*
 * Only instance of the class
 */
static JsonParser *INSTANCE = nullptr;

/**
 * Initializes the JSON Parser class and connects
 * appropiate SIGNALS/SLOTS
 */
JsonParser::JsonParser()
{
   auto sm = SerialManager::getInstance();
   connect(sm, SIGNAL(packetReceived(QByteArray)), this, SLOT(readData(QByteArray)));
}

/**
 * Returns the only instance of the class
 */
JsonParser *JsonParser::getInstance()
{
   if (!INSTANCE)
      INSTANCE = new JsonParser();

   return INSTANCE;
}

/**
 * Returns the parsed JSON document from the received packet
 */
QJsonDocument JsonParser::document()
{
   return m_document;
}

/**
 * Tries to parse the given data as a JSON document.
 * If JSON parsing is successfull, then the class shall notify the rest of the
 * application in order to process packet data.
 */
void JsonParser::readData(const QByteArray &data)
{
   if (!data.isEmpty())
   {
      QJsonParseError error;
      auto document = QJsonDocument::fromJson(data, &error);

      if (error.error == QJsonParseError::NoError)
      {
         m_document = document;
         emit packetReceived();
      }
   }
}
