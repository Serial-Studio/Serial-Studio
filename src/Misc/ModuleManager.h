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

#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

#include <QObject>
#include <QSettings>
#include <QSplashScreen>
#include <QQmlApplicationEngine>

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

    Q_INVOKABLE int renderingEngine() const;
    Q_INVOKABLE QVector<QString> renderingEngines() const;

public slots:
    void quit();
    void stopOperations();
    void hideSplashscreen();
    void setRenderingEngine(const int engine);
    void setSplashScreenMessage(const QString &message);

private:
    QSettings m_settings;
    QSplashScreen m_splash;
    QQmlApplicationEngine m_engine;
};

#endif
