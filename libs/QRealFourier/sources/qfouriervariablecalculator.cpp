/***********************************************************************

qfouriervariablecalculator.cpp - Source file for
								 QFourierVariableCalculator

Class for calculating FFts of a variable size.

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

#include "qfouriervariablecalculator.h"

QFourierVariableCalculator::QFourierVariableCalculator()
	: QFourierCalculator()
{
	mFourierTransform = 0;
}

QFourierVariableCalculator::~QFourierVariableCalculator()
{
	if(mFourierTransform != 0)
	{
		delete mFourierTransform;
	}
}

void QFourierVariableCalculator::setSize(int size)
{
	QFourierCalculator::setSize(size);
	if(mFourierTransform == 0)
	{
		mFourierTransform = new ffft::FFTReal<float>(mSize);
	}
	else if(mFourierTransform->get_length() != mSize)
	{
		delete mFourierTransform;
		mFourierTransform = new ffft::FFTReal<float>(mSize);
	}
}

void QFourierVariableCalculator::forward()
{
	mFourierTransform->do_fft(mOutput, mInput);
}

void QFourierVariableCalculator::inverse()
{
	mFourierTransform->do_ifft(mInput, mOutput);
}

void QFourierVariableCalculator::rescale()
{
	mFourierTransform->rescale(mInput);
}
