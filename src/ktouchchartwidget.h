/***************************************************************************
 *   ktouchchartwidget.h                                                   *
 *   -------------------                                                   *
 *   Copyright (C) 2000 by Håvard Frøiland, 2004 by Andreas Nicolai        *
 *   ghorwin@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KTOUCHCHARTWIDGET
#define KTOUCHCHARTWIDGET

#include <kplotwidget.h>

class KTouchTrainer;

/// This is the chart widget and contains all chart drawing code.
class KTouchChartWidget : public KPlotWidget {
    Q_OBJECT
  public:
    /// Constructor.
    explicit KTouchChartWidget(QWidget* parent, const char *name=0);
    /// Destructor.
    ~KTouchChartWidget();
};

#endif  // KTOUCHCHARTWIDGET
