#include "Surface.h"
#include <qdebug.h>
#include "ImageConversion.h"
#include <QPainter>
#include <QRect>
#include <qmessagebox.h>

//#define CHECK_QIMAGE

#ifdef CHECK_QIMAGE
#include <QtDebug.h>
#endif // CHECK_QIMAGE


QColor Surface::_myPenColor(0, 0, 0);
int Surface::_myPenRadius = 10;

Surface::Surface(QImage Img, QWidget*parent) :QLabel(parent), _blendImage(Img.height(), Img.width(), CV_8UC3)
{
	//labelImg() = Mat(400, 400, CV_8UC3);//TODO labelImg should be replace
	//_ImageDraw = ImageConversion::cvMat_to_QImage(labelImg());
	_oriImage = Img;
	setOriginalImage(Img);
	
	this->setMouseTracking(true);
	_bLButtonDown = false;
	
	_bSelectClass = false;
	setCursorInvisible(false);
	_bDrawCursor = true;
	_scaleRatioRank = 0;
	_scaleRatio = 1.0;
	_bEdit = false;
	_scrollArea = nullptr;
	fitSizeToImage();
}

Surface::~Surface()
{
#ifdef CHECK_QIMAGE
	cv::destroyAllWindows();
#endif
}

QImage Surface::getImage()
{
	return this->_ImageDraw;
}

QImage Surface::getOriImage()
{
	return this->_oriImage;
}

QImage Surface::getImageCpy()
{
	return this->_ImageDraw.copy();
}

QImage Surface::getOriImageCpy()
{
	return this->_oriImage.copy();
}

void Surface::fitSizeToImage()
{
	int width = _oriImage.width();
	int height = _oriImage.height();
	this->resize(width, height);
}

void Surface::setEditable(bool b)
{
	_bEdit = b;
	setCursorInvisible(b);
	_ImageDraw = _oriImage.copy();
}
void Surface::startLabel()
{
	setEditable(true);
}
void Surface::endLabel()
{
	setEditable(false);
}

bool Surface::isEditable()
{
	return _bEdit;
}

void Surface::setOriginalImage(QImage pOriginal)
{
	_oriImage = pOriginal;
	if (isEditable())
	{
		_ImageDraw = _oriImage.copy();
	}
	else
	{
		_ImageDraw = _oriImage;
	}
	//this->setPixmap(QPixmap::fromImage(_ImageDraw));
}

void Surface::setScrollArea(QScrollArea* pScrollArea)
{
	_scrollArea = pScrollArea;
}

void Surface::setReferenceImage(QImage* pReference)
{
	_referenceImage = pReference;
}

void Surface::changeClass(QString txt, QColor clr)
{
	if (isEditable())
	{
		qDebug() << "Surface::changeClass to: \n" << txt << clr ;
		_myPenColor = clr;
	}
}

void Surface::showNormal()
{
	this->setScaleRatio(1.0);
	this->applyScaleRatio();
	update();
}

void Surface::showScaled()
{
	this->updateScaleRatioByRank();
	this->applyScaleRatio();
	update();
}

void Surface::showScaledRefImg(QImage* Img)
{
	if (!_oriImage.isNull() && _scaleRatio != 1.0)
	{
		int width = _oriImage.width()*_scaleRatio;
		int height = _oriImage.height()*_scaleRatio;
		width = width < 1 ? 1 : width;
		height = height < 1 ? 1 : height;
		_ImageDraw = Img->scaled(QSize(width, height), Qt::AspectRatioMode::KeepAspectRatioByExpanding);
	}
	else
		_ImageDraw = Img->copy();
	update();
}

void Surface::showReferenceImg()
{
	if (_referenceImage)
	{
		QImage img;
		img = blendImage(_oriImage, 0.5, *_referenceImage, 0.5);
		showScaledRefImg(&img);
		//showScaledRefImg(_referenceImage); ??please uncomment this to restore
	}
	else
	{
		_ImageDraw.fill(0);
		update();
	}
}

void Surface::showInternalImg()
{
	showScaled();
}

bool Surface::eventFilter(QObject* obj, QEvent* evt)
{
	if (obj != nullptr)
	{
		if ((QScrollArea*)obj == _scrollArea)
		{
			if (evt->type() == QEvent::KeyPress)	
			{
				keyPressEvent((QKeyEvent *)evt);	
			}
			else if (evt->type() == QEvent::KeyRelease)
			{
				keyReleaseEvent((QKeyEvent *)evt);	
			}
		}
	}
	return false;
}

void Surface::keyPressEvent(QKeyEvent *ev)
{
	switch (ev->key())
	{
	case Qt::Key::Key_Shift:
		if (isEditable()) 
		{
			emit openClassSelection();
			qDebug() << "emit openClassSelection()" ;
		}
		break;
	case Qt::Key::Key_V:
		//qDebug() << "showNormal()";
		this->showNormal(); break;
	case Qt::Key::Key_Tab:
		//qDebug() << "showReferenceImg()";
		this->showReferenceImg(); break;
	case Qt::Key::Key_R:
		emit clearResult(); break;
	//case Qt::Key::Key_Space:qDebug() << "Key_Space" ; break;

	default:break;
	}
	QLabel::keyPressEvent(ev);
}
void Surface::keyReleaseEvent(QKeyEvent *ev)
{
	switch (ev->key())
	{
	case Qt::Key::Key_Shift:
		if (isEditable())
		{
			emit closeClassSelection();
			qDebug() << "emit closeClassSelection()" ;
		}
		break;
	case Qt::Key::Key_V:
		this->showScaled(); break;
	case Qt::Key::Key_Tab:
		this->showInternalImg(); break;
		//case Qt::Key::Key_Space:qDebug() << "Key_Space" ; break;
	default:break;
	}
	QLabel::keyReleaseEvent(ev);
}


void Surface::mousePressEvent(QMouseEvent *ev)
{
	switch (ev->buttons())
	{
	case Qt::LeftButton: 
		if (isEditable())
		{
			qDebug() << "Qt::LeftButton Press" ;
			_bLButtonDown = true; _lastPoint = ev->pos(); _paintPath = QPainterPath(); _tempDrawPath = QPainterPath();
		}
		break;
	case Qt::RightButton: qDebug() << "Qt::RightButton Press" ; 

		break;
	case Qt::MidButton://qDebug() << "Qt::MidButton Press" ; 
		break;
	default://qDebug() << "mousePress Default case." ; 
		break;
	}
	QLabel::mousePressEvent(ev);
}

void Surface::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);
	/* draw background image */
	QRect dirtyRect = ev->rect();
	painter.drawImage(dirtyRect, _ImageDraw, dirtyRect);

	if (isEditable())
	{
		/* draw stroke trace */
		if (_bLButtonDown)
		{
			painter.setPen(QPen(_myPenColor, (_myPenRadius * 2 + 1), Qt::SolidLine, Qt::RoundCap,
				Qt::RoundJoin));
			painter.drawPath(_tempDrawPath);
			qDebug() << "painter.drawPath";
		}
		/*draw cursor*/
		if (_bDrawCursor)
			paintCursor();
	}
	
	//qDebug() << "paintEvent";
	ev->accept();
	QLabel::paintEvent(ev);
}

void Surface::mouseMoveEvent(QMouseEvent *ev)
{
	//qDebug() << '(' << ev->x() << ':' << ev->y() << ')' ;
	if (isEditable())
	{
		if (_bLButtonDown)
		{
			qDebug() << "Draw: " << '(' << ev->x() << ':' << ev->y() << ')';
			QPoint endPoint = ev->pos();
			drawLineTo(endPoint);
			updateRectArea(QRect(_lastPoint, endPoint).normalized(), _myPenRadius, false);
			_lastPoint = endPoint;
		}

		updateCursorArea(false);
		_mousePos = ev->pos();
		updateCursorArea(true);
	}
	QLabel::mouseMoveEvent(ev);
}

void Surface::mouseReleaseEvent(QMouseEvent *ev)
{
	if (isEditable())
	{
		if (_bLButtonDown)
		{
			_bLButtonDown = false;
			emit painterPathCreated((_myPenRadius * 2 + 1) / _scaleRatio, _paintPath);
			updateRectArea(_tempDrawPath.boundingRect().toRect(), _myPenRadius, false);
			_paintPath = QPainterPath();
			_tempDrawPath = QPainterPath();
		}
//#ifdef CHECK_QIMAGE
//		Mat temp = ImageConversion::QImage_to_cvMat(_ImageDraw, false);
//		cv::imshow("QImage", temp);
//		cv::waitKey(1);
//#endif
	}
	QLabel::mouseReleaseEvent(ev);
}

void Surface::mouseDoubleClickEvent(QMouseEvent* ev)
{
	switch (ev->buttons())
	{
	case Qt::LeftButton:
		qDebug() << "Left Double Clicked"; break;
	case Qt::RightButton:
		qDebug() << "Right Double Clicked"; break;
	default:
		break;
	}
	QLabel::mouseDoubleClickEvent(ev);
}

void Surface::wheelEvent(QWheelEvent*ev)
{
	QPoint numDegrees = ev->angleDelta() / 8;
	QPoint numSteps = numDegrees / 15;
	qDebug() << "Surface::wheelEvent";
	qDebug() << "numDegrees:" << numDegrees;
	switch (ev->modifiers())
	{
	case Qt::KeyboardModifier::ControlModifier:
		if (isEditable())
		{
			updateCursorArea(false);
			_myPenRadius += numSteps.y() * 2;
			_myPenRadius = qMax(0, _myPenRadius);
			updateCursorArea(true);
			qDebug() << "myPenRadius" << _myPenRadius;
		}
		break;
	default:
		break;
	}
	if (ev->modifiers() == Qt::KeyboardModifier::AltModifier)
	{
		zoom(numSteps.x(), ev->pos());
		qDebug() << "AltModifier";
	}
	ev->accept();
}

void Surface::leaveEvent(QEvent*ev)
{
	if (isEditable())
	{
		_bDrawCursor = false;
		updateCursorArea(false);
	}
	//qDebug() << "leaveEvent";
	QLabel::leaveEvent(ev);
}

void Surface::enterEvent(QEvent*ev)
{
	if (isEditable())
	{
		_bDrawCursor = true;
	}
	QLabel::enterEvent(ev);
}

void Surface::updateRectArea(QRect rect, int rad, bool drawCursor)
{
	bool store = _bDrawCursor;
	_bDrawCursor = drawCursor;
	update(rect.adjusted(-rad, -rad, +rad, +rad).adjusted(-3, -3, 3, 3));
	_bDrawCursor = store;
}

QPoint Surface::FilterScalePoint(QPoint pt)
{
	QPoint rtv;
	rtv.setX(pt.x() / _scaleRatio);
	rtv.setY(pt.y() / _scaleRatio);
	return rtv;
}

void Surface::drawLineTo(const QPoint &endPoint)
{
	/*QPainter painter(&_ImageDraw);
	painter.setPen(QPen(myPenColor, myPenRadius, Qt::SolidLine, Qt::RoundCap,
	Qt::RoundJoin));*/
	_paintPath.moveTo(FilterScalePoint(_lastPoint));
	_paintPath.lineTo(FilterScalePoint(endPoint));
	_tempDrawPath.moveTo(_lastPoint);
	_tempDrawPath.lineTo(endPoint);
	//painter.drawLine(lastPoint, endPoint);

	//int rad = (myPenRadius / 2) + 2;
	//update(QRect(lastPoint, endPoint).normalized().adjusted(-rad, -rad, +rad, +rad));

}

void Surface::paintCursor()
{
	QPainter painter(this);
	painter.setPen(QPen(_myPenColor, _cursorEdgeWidth, Qt::SolidLine, Qt::RoundCap,
		Qt::RoundJoin));
	painter.setBrush(QBrush(_myPenColor, Qt::DiagCrossPattern));
	painter.drawEllipse(_mousePos, _myPenRadius, _myPenRadius);
}

void Surface::updateImage()
{
	_ImageDraw = _oriImage.copy();

#ifdef CHECK_QIMAGE
	qt_debug::showQImage(_oriImage);
#endif //CHECK_QIMAGE 

	applyScaleRatio();
}

void Surface::updateCursorArea(bool drawCursor)
{
	bool beforeCursor = _bDrawCursor;
	_bDrawCursor = drawCursor;
	this->update(QRect(_mousePos.x() - (_myPenRadius + _cursorEdgeWidth), _mousePos.y() - (_myPenRadius + _cursorEdgeWidth), \
		(_myPenRadius + _cursorEdgeWidth) * 2 + 3, (_myPenRadius + _cursorEdgeWidth) * 2 + 3));
	_bDrawCursor = beforeCursor;
}




void Surface::setCursorInvisible(bool b)
{
	if(b)
		this->setCursor(Qt::BlankCursor);
	else
		this->setCursor(Qt::ArrowCursor);
}

QPoint Surface::getPointAfterNewScale(QPoint pt, double scaleOld, double scaleNew)
{
	double x = pt.x();
	double y = pt.y();
	x = x / scaleOld*scaleNew;
	y = y / scaleOld*scaleNew;
	return QPoint(x, y);
}

void Surface::setScaleRatio(double ratio, double maximum, double minimum)
{
	if (ratio <= 0.0)
	{
		QMessageBox::warning(NULL,"ScaleRatio Error","The resize scale cannot be lower than 0",QMessageBox::StandardButton::Ok);
	}
	else
	{
		_scaleRatio = ratio;
		_scaleRatio = qMin(maximum, _scaleRatio);
		_scaleRatio = qMax(minimum, _scaleRatio);
		qDebug() << "_scaleRatio:" << _scaleRatio;
	}
}

double Surface::getScaleRatio()
{
	return _scaleRatio;
}

void Surface::setScaleRatioRank(int rank, int maximum, int minimum)
{
	_scaleRatioRank = rank;
	_scaleRatioRank = qMin(maximum, _scaleRatioRank);
	_scaleRatioRank = qMax(minimum, _scaleRatioRank);
	qDebug() << "setScaleRatioRank:" << _scaleRatioRank;
}

int Surface::getScaleRatioRank()
{
	qDebug() << "getScaleRatioRank:" << _scaleRatioRank;
	return _scaleRatioRank;
}

void Surface::updateScaleRatioByRank()
{
	if (_scaleRatioRank > 0)
	{
		setScaleRatio(pow(1.25, _scaleRatioRank));
	}
	else if (_scaleRatioRank < 0)
	{
		setScaleRatio(pow(0.8, -_scaleRatioRank));
	}
	else
	{
		setScaleRatio(1);
	}
}

void Surface::applyScaleRatio()
{

	if (!_oriImage.isNull() && _scaleRatio != 1.0)
	{
		int width = _oriImage.width()*_scaleRatio;
		int height = _oriImage.height()*_scaleRatio;
		width = width < 1 ? 1 : width;
		height = height < 1 ? 1 : height;
		_ImageDraw = _oriImage.scaled(QSize(width, height), Qt::AspectRatioMode::KeepAspectRatioByExpanding);
	}
	else
		_ImageDraw = _oriImage.copy();
	
}

void Surface::zoom(int step, QPoint pt)
{
	double oldScaleRatio = getScaleRatio();
	setScaleRatioRank(getScaleRatioRank() + step);
	updateScaleRatioByRank();
	double newScaleRatio = getScaleRatio();
	QPoint newPos = getPointAfterNewScale(pt, oldScaleRatio, newScaleRatio);
	emit mousePositionShiftedByScale(pt, oldScaleRatio, newScaleRatio);
	applyScaleRatio();
	this->resize(_ImageDraw.width(), _ImageDraw.height());
	update();
}

double Surface::getZoomRatio()
{
	return this->_scaleRatio;
}

QImage Surface::blendImage(QImage& img1, double ratio1, QImage& img2, double ratio2)
{
	assert(img1.height() == img2.height() && img1.width() == img2.width());
	Mat m1 = ImageConversion::QImage_to_cvMat(img1, false);
	Mat m2 = ImageConversion::QImage_to_cvMat(img2, false);
	assert(m1.type() == m2.type() && m1.type() == CV_8UC3);
	
#pragma omp parallel for
	for (int i = 0; i < m1.rows; i++)
	{
		Vec3b*p_m1_row = m1.ptr<Vec3b>(i);
		Vec3b*p_m2_row = m2.ptr<Vec3b>(i);
		Vec3b*p_m3_row = _blendImage.ptr<Vec3b>(i);
		for (int j = 0; j < m1.cols; j++)
		{
			p_m3_row[j] = p_m1_row[j] * ratio1 + p_m2_row[j] * ratio2;
		}
	}
	//cv::cvtColor(_blendImage, _blendImage, CV_BGR2RGB);
	QImage rvt = ImageConversion::cvMat_to_QImage(_blendImage, false);
	return rvt;
}