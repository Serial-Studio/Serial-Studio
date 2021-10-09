/***********************************************************************

qwindowfunction.h - Header file for QWindowFunction,
					QWindowFunctionManager, QRectangularFunction,
					QHammingFunction, QHannFunction

Template classes necessary for applying a window function to a discrete
set of samples.

************************************************************************

This file is part of QRealFourier.

QRealFourier is free software: you can redistribute it and/or modify it
under the terms of the Lesser GNU General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Foobar is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the Lesser GNU General Public
License for more details.

You should have received a copy of the Lesser GNU General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

************************************************************************

Copyright © 2012 - 2013 Christoph Stallmann, University of Pretoria

Developer: Christoph Stallmann
University of Pretoria
Department of Computer Science

http://www.visore.org
http://sourceforge.net/projects/qrealfourier
http://github.com/visore/QRealFourier

qrealfourier@visore.org
qrealfourier@gmail.com

***********************************************************************/

#ifndef QWINDOWFUNCTION_H
#define QWINDOWFUNCTION_H

#include <QList>
#include <QStringList>
#include <QVector>
#include "math.h"
#include <qmath.h>

/**********************************************************
QWindowFunction
**********************************************************/

template <typename T>
class QWindowFunction
{

	typedef QVector<T> QWindow;

	public:

		void apply(T *data, int size);
		void create(int size);

	protected:

		void fillWindow(int size);
		virtual T calculate(int currentSample, int totalSamples) = 0;

	protected:

		QWindow mWindow;

};

/**********************************************************
QWindowFunctionManager
**********************************************************/

template <typename T>
class QWindowFunctionManager
{

	public:

		static QWindowFunction<T>* createFunction(QString functionName);
		static QStringList functions();

};

/**********************************************************
QRectangularFunction
**********************************************************/

template <typename T>
class QRectangularFunction : public QWindowFunction<T>
{

	protected:

		T calculate(int currentSample, int totalSamples);

};

/**********************************************************
QHammingFunction
**********************************************************/

template <typename T>
class QHammingFunction : public QWindowFunction<T>
{

	protected:

		T calculate(int currentSample, int totalSamples);

};

/**********************************************************
QHannFunction
**********************************************************/

template <typename T>
class QHannFunction : public QWindowFunction<T>
{

	protected:

		T calculate(int currentSample, int totalSamples);

};

#endif
