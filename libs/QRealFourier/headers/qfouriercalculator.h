/***********************************************************************

qfouriercalculator.h - Header file for QFourierCalculator

Abstract class providing a common interface for calculating FFTs of
fixed or variable size.

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


#ifndef QFOURIERCALCULATOR_H
#define QFOURIERCALCULATOR_H

class QFourierCalculator
{

	public:
		
		QFourierCalculator();
		virtual ~QFourierCalculator();
		virtual void setData(float input[], float output[] = 0);
		virtual void setSize(int size);
		virtual int size();

		virtual void forward() = 0;
		virtual void inverse() = 0;
		virtual void rescale() = 0;

	protected:

		float *mInput;
		float *mOutput;
		int mSize;

};

#endif
