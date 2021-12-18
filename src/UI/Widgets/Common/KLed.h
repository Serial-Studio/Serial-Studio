/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1998 Jörg Habenicht <j.habenicht@europemail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QWidget>
#include <memory>

class QColor;

/**
 * @class KLed kled.h KLed
 *
 * @short An LED widget.
 *
 * Displays a round or rectangular light emitting diode.
 *
 * It is configurable to arbitrary colors, the two on/off states and three
 * styles (or "looks");
 *
 * It may display itself in a performant flat view, a round view with
 * light spot or a round view sunken in the screen.
 *
 * \image html kled.png "KLed Widget"
 *
 * @author Joerg Habenicht, Richard J. Moore (rich@kde.org) 1998, 1999
 */
class KLed : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(State state READ state WRITE setState)
    Q_PROPERTY(Shape shape READ shape WRITE setShape)
    Q_PROPERTY(Look look READ look WRITE setLook)
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(int darkFactor READ darkFactor WRITE setDarkFactor)

public:
    /**
     * Status of the light is on/off.
     * @short LED on/off.
     */
    enum State
    {
        Off,
        On
    };
    Q_ENUM(State)

    /**
     * Shades of the lamp.
     * @short LED shape
     */
    enum Shape
    {
        Rectangular,
        Circular
    };
    Q_ENUM(Shape)

    /**
     * Displays a flat, round or sunken LED.
     *
     * @short LED look.
     */
    enum Look
    {
        Flat,
        Raised,
        Sunken,
    };
    Q_ENUM(Look)

    /**
     * Constructs a green, round LED widget which will initially
     * be turned on.
     *
     * @param parent The parent widget.
     */
    explicit KLed(QWidget *parent = Q_NULLPTR);

    /**
     * Constructs a round LED widget with the supplied color which will
     * initially be turned on.
     *
     * @param color Initial color of the LED.
     * @param parent The parent widget.
     * @short Constructor
     */
    explicit KLed(const QColor &color, QWidget *parent = Q_NULLPTR);

    /**
     * Constructor with the color, state and look.
     *
     * Differs from above only in the parameters, which configure all settings.
     *
     * @param color  Initial color of the LED.
     * @param state  Sets the State.
     * @param look   Sets the Look.
     * @param shape  Sets the Shape (rectangular or circular).
     * @param parent The parent widget.
     * @short Constructor
     */
    KLed(const QColor &color, KLed::State state, KLed::Look look, KLed::Shape shape,
         QWidget *parent = Q_NULLPTR);

    /**
     * Destroys the LED widget.
     * @short Destructor
     */
    ~KLed() override;

    /**
     * Returns the current color of the widget.
     *
     * @returns LED color
     * @see setColor()
     */
    QColor color() const;

    /**
     * Returns the current state of the widget (on/off).
     * @returns LED state
     *
     * @see State
     */
    State state() const;

    /**
     * Returns the current look of the widget.
     * @returns LED look
     *
     * @see Look
     */
    Look look() const;

    /**
     * Returns the current shape of the widget.
     * @returns LED shape
     *
     * @see Shape
     */
    Shape shape() const;

    /**
     * Returns the factor to darken the LED.
     * @returns dark factor
     *
     * @see setDarkFactor()
     */
    int darkFactor() const;

    /**
     * Set the color of the widget.
     *
     * The LED is shown with @p color when in the KLed::On state
     * or with the darken color in KLed::Off state.
     *
     * The widget calls the update() method, so it will
     * be updated when entering the main event loop.
     *
     * @param color New color of the LED.
     *
     * @see color() darkFactor()
     */
    void setColor(const QColor &color);

    /**
     * Sets the state of the widget to On or Off.
     *
     * @param state The LED state: on or off.
     *
     * @see on() off() toggle()
     */
    void setState(State state);

    /**
     * Sets the look of the widget.
     *
     * The look may be Flat, Raised or Sunken.
     *
     * The widget calls the update() method, so it will
     * be updated when entering the main event loop.
     *
     * @param look New look of the LED.
     *
     * @see Look
     */
    void setLook(Look look);

    /**
     * Set the shape of the LED.
     *
     * @param shape The LED shape.
     * @short Set LED shape.
     */
    void setShape(Shape shape);

    /**
     * Sets the factor to darken the LED in KLed::Off state.
     *
     * The @p darkFactor should be greater than 100, otherwise the LED
     * becomes lighter in KLed::Off state.
     *
     * Defaults to 300.
     *
     * @param darkFactor Sets the factor to darken the LED.
     *
     * @see setColor
     */
    void setDarkFactor(int darkFactor);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public Q_SLOTS:

    /**
     * Toggles the state of the led from Off to On or vice versa.
     */
    void toggle();

    /**
     * Sets the state of the widget to On.
     *
     * @see off() toggle()  setState()
     */
    void on();

    /**
     * Sets the state of the widget to Off.
     *
     * @see on() toggle()  setState()
     */
    void off();

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;

    /**
     * @internal
     * invalidates caches after property changes and calls update()
     */
    void updateCachedPixmap();

private:
    void updateAccessibleName();

private:
    std::unique_ptr<class KLedPrivate> const d;
};
