/***********************************************************************

qwindowfunction.cpp - Source file for QWindowFunction,
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

#include "qwindowfunction.h"

/***********************************************************************
QWindowFunction
***********************************************************************/

template <typename T>
void QWindowFunction<T>::apply(T *data, int size)
{
	for(int i = 0; i < size; ++i)
	{
		data[i] *= mWindow[i];
	}
}

template <typename T>
void QWindowFunction<T>::create(int size)
{
	if(size != mWindow.size())
	{
		mWindow.clear();
		mWindow.resize(size);
		fillWindow(size);
	}
}

template <typename T>
void QWindowFunction<T>::fillWindow(int size)
{
	for(int i = 0; i < size; ++i)
	{
		mWindow[i] = calculate(i, size);
	}
}

template class QWindowFunction<short>;
template class QWindowFunction<int>;
template class QWindowFunction<long>;
template class QWindowFunction<float>;
template class QWindowFunction<double>;

/***********************************************************************
QWindowFunctionManager
***********************************************************************/

template <typename T>
QWindowFunction<T>* QWindowFunctionManager<T>::createFunction(QString functionName)
{
	functionName = functionName.trimmed().toLower().replace("function", "");
	if(functionName == "hamming")
	{
		return new QHammingFunction<T>;
	}
	else if(functionName == "hann")
	{
		return new QHannFunction<T>;
	}
	return NULL;
}

template <typename T>
QStringList QWindowFunctionManager<T>::functions()
{
	QStringList result;
	result << "Rectangular" << "Hamming" << "Hann";
	return result;
}

template class QWindowFunctionManager<short>;
template class QWindowFunctionManager<int>;
template class QWindowFunctionManager<long>;
template class QWindowFunctionManager<float>;
template class QWindowFunctionManager<double>;

/***********************************************************************
QRectangularFunction
***********************************************************************/

template <typename T>
T QRectangularFunction<T>::calculate(int currentSample, int totalSamples)
{
    (void) currentSample;
    (void) totalSamples;

	return 1.0;
}

template class QRectangularFunction<short>;
template class QRectangularFunction<int>;
template class QRectangularFunction<long>;
template class QRectangularFunction<float>;
template class QRectangularFunction<double>;

/***********************************************************************
QHammingFunction
***********************************************************************/

template <typename T>
T QHammingFunction<T>::calculate(int currentSample, int totalSamples)
{
	return 0.54 + (0.46 * qCos((2 * M_PI * currentSample) / (totalSamples - 1)));
}

template class QHammingFunction<short>;
template class QHammingFunction<int>;
template class QHammingFunction<long>;
template class QHammingFunction<float>;
template class QHammingFunction<double>;

/***********************************************************************
QHannFunction
***********************************************************************/

template <typename T>
T QHannFunction<T>::calculate(int currentSample, int totalSamples)
{
	return 0.5 * (1 - qCos((2 * M_PI * currentSample) / (totalSamples - 1)));
}

template class QHannFunction<short>;
template class QHannFunction<int>;
template class QHannFunction<long>;
template class QHannFunction<float>;
template class QHannFunction<double>;
