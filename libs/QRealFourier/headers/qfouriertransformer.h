/***********************************************************************

qfouriertransformer.h - Header file for QFourierTransformer

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

#ifndef QFOURIERTRANSFORMER_H
#define QFOURIERTRANSFORMER_H

#include "qfouriercalculator.h"
#include "qwindowfunction.h"
#include "qcomplexnumber.h"
#include <QMap>

typedef QVector<QComplexFloat> QComplexVector;

class QFourierTransformer
{

	public:

		enum Direction
		{
			Forward = 0,
			Inverse = 1
		};

		enum Initialization
		{
			VariableSize = 0,
			FixedSize = 1,
			InvalidSize = 2
		};

	public:

		QFourierTransformer(int size = 0, QString functionName = "");
		~QFourierTransformer();

		Initialization setSize(int size);
		bool setWindowFunction(QString functionName);
		QStringList windowFunctions();

		void transform(float input[], float output[], Direction direction = QFourierTransformer::Forward);
		void forwardTransform(float *input, float *output);
		void inverseTransform(float input[], float output[]);
		void rescale(float input[]);

		void conjugate(float input[]);
		QComplexVector toComplex(float input[]);

	protected:

		void initialize();
		int sizeToKey(int size);
		bool isValidSize(int value);

	private:

		int mSize;
		QMap<int, QFourierCalculator*> mFixedCalculators;
		QFourierCalculator* mVariableCalculator;
		QFourierCalculator *mCalculator;
		QStringList mWindowFunctions;
		QWindowFunction<float> *mWindowFunction;

};

#endif
