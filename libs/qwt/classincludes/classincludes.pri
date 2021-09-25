################################################################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
################################################################

INCLUDEPATH += $$PWD

CLASSHEADERS = \
    $$PWD/QwtAbstractScaleDraw \
    $$PWD/QwtAlphaColorMap \
    $$PWD/QwtAxis \
    $$PWD/QwtAxisId \
    $$PWD/QwtBezier \
    $$PWD/QwtClipper \
    $$PWD/QwtColorMap \
    $$PWD/QwtColumnRect \
    $$PWD/QwtColumnSymbol \
    $$PWD/QwtDate \
    $$PWD/QwtDateScaleDraw \
    $$PWD/QwtDateScaleEngine \
    $$PWD/QwtDynGridLayout \
    $$PWD/QwtGlobal \
    $$PWD/QwtGraphic \
    $$PWD/QwtHueColorMap \
    $$PWD/QwtInterval \
    $$PWD/QwtIntervalSymbol \
    $$PWD/QwtLinearColorMap \
    $$PWD/QwtLinearScaleEngine \
    $$PWD/QwtLogScaleEngine \
    $$PWD/QwtLogTransform \
    $$PWD/QwtMagnifier \
    $$PWD/QwtMath \
    $$PWD/QwtNullPaintDevice \
    $$PWD/QwtNullTransform \
    $$PWD/QwtPainter \
    $$PWD/QwtPainterCommand \
    $$PWD/QwtPanner \
    $$PWD/QwtPicker \
    $$PWD/QwtPickerClickPointMachine \
    $$PWD/QwtPickerClickRectMachine \
    $$PWD/QwtPickerDragLineMachine \
    $$PWD/QwtPickerDragPointMachine \
    $$PWD/QwtPickerDragRectMachine \
    $$PWD/QwtPickerMachine \
    $$PWD/QwtPickerPolygonMachine \
    $$PWD/QwtPickerTrackerMachine \
    $$PWD/QwtPixelMatrix \
    $$PWD/QwtPlainTextEngine \
    $$PWD/QwtPoint3D \
    $$PWD/QwtPointPolar \
    $$PWD/QwtPowerTransform \
    $$PWD/QwtRichTextEngine \
    $$PWD/QwtRoundScaleDraw \
    $$PWD/QwtSaturationValueColorMap \
    $$PWD/QwtScaleArithmetic \
    $$PWD/QwtScaleDiv \
    $$PWD/QwtScaleDraw \
    $$PWD/QwtScaleEngine \
    $$PWD/QwtScaleMap \
    $$PWD/QwtSimpleCompassRose \
    $$PWD/QwtSplineBasis \
    $$PWD/QwtSpline \
    $$PWD/QwtSplineC1 \
    $$PWD/QwtSplineC2 \
    $$PWD/QwtSplineCubic \
    $$PWD/QwtSplineG1 \
    $$PWD/QwtSplineInterpolating \
    $$PWD/QwtSplineLocal \
    $$PWD/QwtSplineParameter \
    $$PWD/QwtSplineParametrization \
    $$PWD/QwtSplinePleasing \
    $$PWD/QwtSplinePolynomial \
    $$PWD/QwtSymbol \
    $$PWD/QwtSystemClock \
    $$PWD/QwtText \
    $$PWD/QwtTextEngine \
    $$PWD/QwtTextLabel \
    $$PWD/QwtTransform \
    $$PWD/QwtWidgetOverlay

contains(QWT_CONFIG, QwtPlot) {
    CLASSHEADERS += \
        $$PWD/QwtAbstractLegend \
        $$PWD/QwtCurveFitter \
        $$PWD/QwtEventPattern \
        $$PWD/QwtIntervalSample \
        $$PWD/QwtLegend \
        $$PWD/QwtLegendData \
        $$PWD/QwtLegendLabel \
        $$PWD/QwtPointMapper \
        $$PWD/QwtMatrixRasterData \
        $$PWD/QwtOHLCSample \
        $$PWD/QwtPlot \
        $$PWD/QwtPlotAbstractBarChart \
        $$PWD/QwtPlotAbstractCanvas \
        $$PWD/QwtPlotBarChart \
        $$PWD/QwtPlotCanvas \
        $$PWD/QwtPlotCurve \
        $$PWD/QwtPlotDict \
        $$PWD/QwtPlotDirectPainter \
        $$PWD/QwtPlotGraphicItem \
        $$PWD/QwtPlotGrid \
        $$PWD/QwtPlotHistogram \
        $$PWD/QwtPlotIntervalCurve \
        $$PWD/QwtPlotItem \
        $$PWD/QwtPlotLayout \
        $$PWD/QwtPlotLegendItem \
        $$PWD/QwtPlotMagnifier \
        $$PWD/QwtPlotMarker \
        $$PWD/QwtPlotMultiBarChart \
        $$PWD/QwtPlotPanner \
        $$PWD/QwtPlotPicker \
        $$PWD/QwtPlotRasterItem \
        $$PWD/QwtPlotRenderer \
        $$PWD/QwtPlotRescaler \
        $$PWD/QwtPlotScaleItem \
        $$PWD/QwtPlotSeriesItem \
        $$PWD/QwtPlotShapeItem \
        $$PWD/QwtPlotSpectroCurve \
        $$PWD/QwtPlotSpectrogram \
        $$PWD/QwtPlotTextLabel \
        $$PWD/QwtPlotTradingCurve \
        $$PWD/QwtPlotVectorField \
        $$PWD/QwtPlotZoneItem \
        $$PWD/QwtPlotZoomer \
        $$PWD/QwtScaleWidget \
        $$PWD/QwtRasterData \
        $$PWD/QwtSeriesData \
        $$PWD/QwtSetSample \
        $$PWD/QwtSamplingThread \
        $$PWD/QwtSplineCurveFitter \
        $$PWD/QwtWeedingCurveFitter \
        $$PWD/QwtIntervalSeriesData \
        $$PWD/QwtPoint3DSeriesData \
        $$PWD/QwtPointSeriesData \
        $$PWD/QwtSetSeriesData \
        $$PWD/QwtSyntheticPointData \
        $$PWD/QwtPointArrayData \
        $$PWD/QwtTradingChartData \
        $$PWD/QwtVectorFieldSymbol \
        $$PWD/QwtVectorFieldArrow \
        $$PWD/QwtVectorFieldThinArrow \
        $$PWD/QwtVectorFieldData \
        $$PWD/QwtVectorFieldSample \
        $$PWD/QwtCPointerData
}

contains(QWT_CONFIG, QwtOpenGL) {

    CLASSHEADERS += \
        $$PWD/QwtPlotGLCanvas

    greaterThan(QT_MAJOR_VERSION, 4) {

        greaterThan(QT_MINOR_VERSION, 3) {

            CLASSHEADERS += \
                $$PWD/QwtPlotOpenGLCanvas
        }
    }
}

contains(QWT_CONFIG, QwtWidgets) {
    CLASSHEADERS += \
        $$PWD/QwtAbstractSlider \
        $$PWD/QwtAbstractScale \
        $$PWD/QwtArrowButton \
        $$PWD/QwtAnalogClock \
        $$PWD/QwtCompass \
        $$PWD/QwtCompassMagnetNeedle \
        $$PWD/QwtCompassRose \
        $$PWD/QwtCompassScaleDraw \
        $$PWD/QwtCompassWindArrow \
        $$PWD/QwtCounter \
        $$PWD/QwtDial \
        $$PWD/QwtDialNeedle \
        $$PWD/QwtDialSimpleNeedle \
        $$PWD/QwtKnob \
        $$PWD/QwtSlider \
        $$PWD/QwtThermo \
        $$PWD/QwtWheel
}

contains(QWT_CONFIG, QwtSvg) {
    CLASSHEADERS += \
        $$PWD/QwtPlotSvgItem
}

