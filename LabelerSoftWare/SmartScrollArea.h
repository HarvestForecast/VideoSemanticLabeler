#pragma once
#include "qscrollarea.h"
#include <QWheelEvent>
#include <QKeyEvent>

class SmartScrollArea : public QScrollArea
{
	Q_OBJECT
public:
	SmartScrollArea(QWidget*parent = NULL);
	virtual ~SmartScrollArea();
protected:
	void wheelEvent(QWheelEvent* ev) Q_DECL_OVERRIDE;
	void keyPressEvent(QKeyEvent*ev) Q_DECL_OVERRIDE;
	void keyReleaseEvent(QKeyEvent*ev) Q_DECL_OVERRIDE;
public:
public slots :
	void gentleShiftScrollAreaWhenScaled(QPoint mousePt, double oldRatio, double newRatio);
private:
	Qt::Key keyPressed;
};

