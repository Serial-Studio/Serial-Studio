/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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

#pragma once

#include <QObject>
#include <QSplashScreen>
#include <QQmlApplicationEngine>

#include <DataTypes.h>

namespace Misc
{
/**
 * @brief The ModuleManager class
 *
 * The @c ModuleManager class is in charge of initializing all the C++ modules that are
 * part of Serial Studio in the correct order.
 *
 * Also, the class configures the QML rendering engine during application startup and
 * displays a splash screen to entretain the user while the user interface is loaded.
 */
class ModuleManager : public QObject
{
    Q_OBJECT

public:
    ModuleManager();
    void configureUpdater();
    void registerQmlTypes();
    bool autoUpdaterEnabled();
    void initializeQmlInterface();
    QQmlApplicationEngine *engine();

public Q_SLOTS:
    void onQuit();
    void hideSplashscreen();

private:
    QSplashScreen m_splash;
    QQmlApplicationEngine m_engine;
};
}
