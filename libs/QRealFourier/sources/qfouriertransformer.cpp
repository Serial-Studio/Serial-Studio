/***********************************************************************

qfouriertransformer.cpp - Source file for QFourierTransformer

Facade class for calculating FFTs from a set of samples.

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

#include "qfouriertransformer.h"
#include "qfourierfixedcalculator.h"
#include "qfouriervariablecalculator.h"

QFourierTransformer::QFourierTransformer(int size, QString functionName)
{
	mWindowFunctions = QWindowFunctionManager<float>::functions();
	mWindowFunction = 0;
	mCalculator = 0;
	initialize();
	setSize(size);
	setWindowFunction(functionName);
}

QFourierTransformer::~QFourierTransformer()
{
	qDeleteAll(mFixedCalculators.begin(), mFixedCalculators.end());
	mFixedCalculators.clear();
	delete mVariableCalculator;
	if(mWindowFunction != 0)
	{
		delete mWindowFunction;
	}
}

QFourierTransformer::Initialization QFourierTransformer::setSize(int size)
{
	if(isValidSize(size))
	{
		mSize = size;
		if(mWindowFunction != 0)
		{
			mWindowFunction->create(mSize);
		}
		int key = sizeToKey(mSize);
		if(mFixedCalculators.contains(key))
		{
			mCalculator = mFixedCalculators[key];
			return QFourierTransformer::FixedSize;
		}
		else
		{
			mCalculator = mVariableCalculator;
			mCalculator->setSize(mSize);
			return QFourierTransformer::VariableSize;
		}
	}
	mSize = 0;
	return QFourierTransformer::InvalidSize;
}

bool QFourierTransformer::setWindowFunction(QString functionName)
{
	for(int i = 0; i < mWindowFunctions.size(); ++i)
	{
		if(functionName.trimmed().toLower().replace("function", "") == mWindowFunctions[i].trimmed().toLower().replace("function", ""))
		{
			if(mWindowFunction != 0)
			{
				delete mWindowFunction;
			}
			mWindowFunction = QWindowFunctionManager<float>::createFunction(functionName);
			if(mWindowFunction != 0 && isValidSize(mSize))
			{
				mWindowFunction->create(mSize);
			}
			return true;
		}
	}
	return false;
}

QStringList QFourierTransformer::windowFunctions()
{
	return mWindowFunctions;
}

void QFourierTransformer::transform(float input[], float output[], Direction direction)
{
	if(direction == QFourierTransformer::Forward)
	{
		forwardTransform(input, output);
	}
	else
	{
		inverseTransform(input, output);
	}
}

void QFourierTransformer::forwardTransform(float *input, float *output)
{
	if(mWindowFunction != 0)
	{
		mWindowFunction->apply(input, mSize);
	}
	mCalculator->setData(input, output);
	mCalculator->forward();
}

void QFourierTransformer::inverseTransform(float input[], float output[])
{
	mCalculator->setData(input, output);
	mCalculator->inverse();
}

void QFourierTransformer::rescale(float input[])
{
	mCalculator->setData(input);
	mCalculator->rescale();
}

void QFourierTransformer::initialize()
{
	mFixedCalculators.insert(3, new QFourierFixedCalculator<3>());
	mFixedCalculators.insert(4, new QFourierFixedCalculator<4>());
	mFixedCalculators.insert(5, new QFourierFixedCalculator<5>());
	mFixedCalculators.insert(6, new QFourierFixedCalculator<6>());
	mFixedCalculators.insert(7, new QFourierFixedCalculator<7>());
	mFixedCalculators.insert(8, new QFourierFixedCalculator<8>());
	mFixedCalculators.insert(9, new QFourierFixedCalculator<9>());
	mFixedCalculators.insert(10, new QFourierFixedCalculator<10>());
	mFixedCalculators.insert(11, new QFourierFixedCalculator<11>());
	mFixedCalculators.insert(12, new QFourierFixedCalculator<12>());
	mFixedCalculators.insert(13, new QFourierFixedCalculator<13>());
	mFixedCalculators.insert(14, new QFourierFixedCalculator<14>());

	mVariableCalculator = new QFourierVariableCalculator();
}

int QFourierTransformer::sizeToKey(int size)
{
	float result = log(float(size)) / log(2.0);
	if(result == float(int(result)))
	{
		return result;
	}
	return -1;
}

bool QFourierTransformer::isValidSize(int value)
{
	return ((value > 0) && ((value & (~value + 1)) == value));
}

void QFourierTransformer::conjugate(float input[])
{
	for(int i = mSize / 2 + 1; i < mSize; ++i)
	{
		input[i] = -input[i];
	}
}

QComplexVector QFourierTransformer::toComplex(float input[])
{
	int last = mSize / 2;
	QVector<QComplexFloat> result(last + 1);
	result[0] = QComplexFloat(input[0], 0);
	for(int i = 1; i < last; ++i)
	{
		result[i] = QComplexFloat(input[i], -input[last + i]);
	}
	result[last] = QComplexFloat(input[last], 0);
	return result;
}
