#include   "ViewerWidget.h"

ViewerWidget::ViewerWidget(QString viewerName, QSize imgSize, QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	name = viewerName;
	if (imgSize != QSize(0, 0)) {
		img = new QImage(imgSize, QImage::Format_ARGB32);
		img->fill(Qt::white);
		resizeWidget(img->size());
		setPainter();
		setDataPtr();
	}
}
ViewerWidget::~ViewerWidget()
{
	delete painter;
	delete img;
}
void ViewerWidget::resizeWidget(QSize size)
{
	this->resize(size);
	this->setMinimumSize(size);
	this->setMaximumSize(size);
}

//Image functions
bool ViewerWidget::setImage(const QImage& inputImg)
{
	if (img != nullptr) {
		delete img;
	}
	img = new QImage(inputImg);
	if (!img) {
		return false;
	}
	resizeWidget(img->size());
	setPainter();
	setDataPtr();
	return true;
}

void ViewerWidget::setPixel(const int& x, const int& y, const uchar& r, const uchar& g, const uchar& b, const uchar& a)
{
	if (isInside(x, y)) {
		int startbyte = y * img->bytesPerLine() + x * 4;
		data[startbyte] = r;
		data[startbyte + 1] = g;
		data[startbyte + 2] = b;
		data[startbyte + 3] = a;
	}
}
void ViewerWidget::setPixel(const int& x, const int& y, const uchar& val)
{
	if (isInside(x, y)) {
		data[y * img->bytesPerLine() + x] = val;
	}
}
void ViewerWidget::setPixel(const int& x, const int& y, double& val)
{
	if (isInside(x, y)) {
		if (val > 1) val = 1;
		if (val < 0) val = 0;
		setPixel(x, y, static_cast<uchar>(255 * val));
	}
}
void ViewerWidget::setPixel(const int& x, const int& y,  double& valR,  double& valG,  double& valB,const double& valA)
{
	if (isInside(x, y)) {
		if (valR > 1) valR = 1;
		if (valG > 1) valG = 1;
		if (valB > 1) valB = 1;
		

		if (valR < 0) valR = 0;
		if (valG < 0) valG = 0;
		if (valB < 0) valB = 0;
		
		double newValA = valA;
		if (newValA > 1) newValA = 1;
		if (newValA < 0) newValA = 0;
		setPixel(x, y, static_cast<uchar>(255 * valR), static_cast<uchar>(255 * valG), static_cast<uchar>(255 * valB), static_cast<uchar>(255 * valA));
	}
}

//Draw functions
void ViewerWidget::freeDraw(const QPoint& end, const QPen& pen)
{
	painter->setPen(pen);
	painter->drawLine(freeDrawBegin, end);
	update();
}

//Slots
void ViewerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}
