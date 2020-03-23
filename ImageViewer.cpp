#include "ImageViewer.h"

ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);
}

//ViewerWidget functions
ViewerWidget* ImageViewer::getViewerWidget(int tabId)
{
	QScrollArea* s = static_cast<QScrollArea*>(ui->tabWidget->widget(tabId));
	if (s) {
		ViewerWidget* vW = static_cast<ViewerWidget*>(s->widget());
		return vW;
	}
	return nullptr;
}
ViewerWidget* ImageViewer::getCurrentViewerWidget()
{
	return getViewerWidget(ui->tabWidget->currentIndex());
}

// Event filters
bool ImageViewer::eventFilter(QObject* obj, QEvent* event)
{
	if (obj->objectName() == "ViewerWidget") {
		return ViewerWidgetEventFilter(obj, event);
	}
	return false;
}

//ViewerWidget Events
bool ImageViewer::ViewerWidgetEventFilter(QObject* obj, QEvent* event)
{
	ViewerWidget* w = static_cast<ViewerWidget*>(obj);

	if (!w) {
		return false;
	}

	if (event->type() == QEvent::MouseButtonPress) {
		ViewerWidgetMouseButtonPress(w, event);
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		ViewerWidgetMouseButtonRelease(w, event);
	}
	else if (event->type() == QEvent::MouseMove) {
		ViewerWidgetMouseMove(w, event);
	}
	else if (event->type() == QEvent::Leave) {
		ViewerWidgetLeave(w, event);
	}
	else if (event->type() == QEvent::Enter) {
		ViewerWidgetEnter(w, event);
	}
	else if (event->type() == QEvent::Wheel) {
		ViewerWidgetWheel(w, event);
	}

	return QObject::eventFilter(obj, event);
}
void ImageViewer::ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	if (e->button() == Qt::LeftButton) {
		w->setFreeDrawBegin(e->pos());
		w->setFreeDrawActivated(true);
	}
}
void ImageViewer::ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	if (e->button() == Qt::LeftButton && w->getFreeDrawActivated()) {
		w->freeDraw(e->pos(), QPen(Qt::red));
		w->setFreeDrawActivated(false);
	}
}
void ImageViewer::ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	if (e->buttons() == Qt::LeftButton && w->getFreeDrawActivated()) {
		w->freeDraw(e->pos(), QPen(Qt::red));
		w->setFreeDrawBegin(e->pos());
	}
}
void ImageViewer::ViewerWidgetLeave(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetEnter(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetWheel(ViewerWidget* w, QEvent* event)
{
	QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
}

//ImageViewer Events
void ImageViewer::closeEvent(QCloseEvent* event)
{
	if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
	{
		event->accept();
	}
	else {
		event->ignore();
	}
}

//Image functions
void ImageViewer::openNewTabForImg(ViewerWidget* vW)
{
	QScrollArea* scrollArea = new QScrollArea;
	scrollArea->setObjectName("QScrollArea");
	scrollArea->setWidget(vW);

	scrollArea->setBackgroundRole(QPalette::Dark);
	scrollArea->setWidgetResizable(true);
	scrollArea->installEventFilter(this);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	QString name = vW->getName();

	ui->tabWidget->addTab(scrollArea, name);
}
bool ImageViewer::openImage(QString filename)
{
	QFileInfo fi(filename);

	QString name = fi.baseName();
	openNewTabForImg(new ViewerWidget(name, QSize(0, 0)));
	ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);

	ViewerWidget* w = getCurrentViewerWidget();

	QImage loadedImg(filename);
	return w->setImage(loadedImg);
}
bool ImageViewer::saveImage(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();
	ViewerWidget* w = getCurrentViewerWidget();

	QImage* img = w->getImage();
	return img->save(filename, extension.toStdString().c_str());
}
bool ImageViewer::clearImage()
{
	ViewerWidget* w = getCurrentViewerWidget();
	w->clear();
	w->update();
	return true;
}
bool ImageViewer::invertColors()
{
	ViewerWidget* w = getCurrentViewerWidget();
	uchar* data = w->getData();
	
	int row = w->getImage()->bytesPerLine();
	int depth = w->getImage()->depth();

	for (int i = 0; i < w->getImgHeight(); i++)
	{
		for (int j = 0; j < w->getImgWidth(); j++)
		{
			if (depth == 8) {
				w->setPixel(j, i, static_cast<uchar>(255 - data[i * row + j ]));
			}
			else {
				uchar r = static_cast<uchar>(255 - data[i * row + j * 4]);
				uchar g = static_cast<uchar>(255 - data[i * row + j * 4 + 1]);
				uchar b = static_cast<uchar>(255 - data[i * row + j * 4 + 2]);
				w->setPixel(j, i, r, g, b);
			}			
		}
	}
	w->update();
	return true;
}

QImage* ImageViewer::mirroring(int n)
{

	
	ViewerWidget* w = getCurrentViewerWidget();
	uchar* data = w->getData();
	QImage* mirrored;
	int row = w->getImage()->bytesPerLine();
	int depth = w->getImage()->depth();
	QColor c;


	if (depth == 8) { //greyscale
		mirrored = new QImage(w->getImgWidth() + 2 * n, w->getImgHeight() + 2 * n, QImage::Format_Grayscale8);

		for (int j = 0; j < w->getImgWidth() + 2 * n; j++)
		{
			for (int i = 0; i < w->getImgHeight() + 2 * n; i++)
			{
				if ((i >= n && i < w->getImgHeight() + n) && (j < w->getImgWidth() + n && j >= n)) {//Stred


					uchar c = static_cast<uchar>(data[(i - n) * row + j - n]);

					mirrored->setPixelColor(j, i, QColor(c, c, c));
				}
				else if ((i <= n && j <= n)) {//vlavo hore


					uchar c = static_cast<uchar>(data[(-i + n) * row + (-j + n)]);

					mirrored->setPixelColor(j, i, QColor(c, c, c));

				}
				else if ((i > n&& j <= n && i < w->getImgHeight() + n)) {//vlavo stred

					uchar c = static_cast<uchar>(data[(i - n) * row - j + n]);

					mirrored->setPixelColor(j, i, QColor(c, c, c));

				}
				else if ((i >= w->getImgHeight() + n && j <= n)) {//vlavo dole

					uchar c = static_cast<uchar>(data[(w->getImgHeight() + w->getImgHeight() - i + n - 1) * row - j + n]);

					mirrored->setPixelColor(j, i, QColor(c, c, c));

				}
				else if (i >= w->getImgHeight() + n && j > n&& j < w->getImgWidth() + n) {//dole stred

					uchar c = static_cast<uchar>(data[(w->getImgHeight() + w->getImgHeight() - i + n - 1) * row + (j - n)]);

					mirrored->setPixelColor(j, i, QColor(c, c, c));

				}
				else if (i >= w->getImgHeight() + n && j >= w->getImgWidth() + n) {//vpravo dole

					uchar c = static_cast<uchar>(data[(w->getImgHeight() + w->getImgHeight() - i + n - 1) * row + (w->getImgWidth() - 1 + w->getImgWidth() - j + n)]);

					mirrored->setPixelColor(j, i, QColor(c, c, c));
				}
				else if (i > n&& j >= w->getImgWidth() + n && i < w->getImgHeight() + n) {//vpravo stred

					uchar c = static_cast<uchar>(data[(i - n) * row + (w->getImgWidth() - 1 + w->getImgWidth() - j + n)]);

					mirrored->setPixelColor(j, i, QColor(c, c, c));

				}
				else if (i <= n && j >= w->getImgWidth() + n) {//vpravo hore

					uchar c = static_cast<uchar>(data[(-i + n) * row + (w->getImgWidth() - 1 + w->getImgWidth() - j + n)]);

					mirrored->setPixelColor(j, i, QColor(c, c, c));

				}
				else if (i <= n && j < w->getImgWidth() + n && j >= n) {//stred hore

					uchar c = static_cast<uchar>(data[(-i + n) * row + (j - n)]);

					mirrored->setPixelColor(j, i, QColor(c, c, c));

				}

				//else {
				//	mirrored->setPixelColor(j, i, QColor(0, 0, 0));
				//}

			}
		}
	}
	else { //ARGB
		mirrored = new QImage(w->getImgWidth() + 2 * n, w->getImgHeight() + 2 * n, QImage::Format_ARGB32);

		for (int j = 0; j < w->getImgWidth() + 2 * n; j++)
		{
			for (int i = 0; i < w->getImgHeight() + 2 * n; i++)
			{
				if ((i >= n && i < w->getImgHeight() + n) && (j < w->getImgWidth() + n && j >= n)) {//stred

					uchar r = static_cast<uchar>(data[(i - n) * row + (j - n) * 4 + 0]);
					uchar g = static_cast<uchar>(data[(i - n) * row + (j - n) * 4 + 1]);
					uchar b = static_cast<uchar>(data[(i - n) * row + (j - n) * 4 + 2]);

					//w->setPixel(j, i, r, g, b);

					mirrored->setPixelColor(j, i, QColor(qRgba(b, g, r, 255)));
				}
				else if ((i <= n && j <= n)) {//vlavo hore

					int r = static_cast<int>(data[(0) * row + (0) * 4 + 0]);
					int g = static_cast<int>(data[(0) * row + (0) * 4 + 1]);
					int b = static_cast<int>(data[(0) * row + (0) * 4 + 2]);

					mirrored->setPixelColor(j, i, QColor(qRgba(b, g, r, 255)));

				}
				else if ((i > n&& j <= n && i < w->getImgHeight() + n)) {//vlavo stred

					int r = static_cast<int>(data[(i - n) * row + (0) * 4 + 0]);
					int g = static_cast<int>(data[(i - n) * row + (0) * 4 + 1]);
					int b = static_cast<int>(data[(i - n) * row + (0) * 4 + 2]);

					mirrored->setPixelColor(j, i, QColor(qRgba(b, g, r, 255)));

				}
				else if ((i >= w->getImgHeight() + n && j <= n)) {//vlavo dole

					int r = static_cast<int>(data[(w->getImgHeight() - 1) * row + (0) * 4 + 0]);
					int g = static_cast<int>(data[(w->getImgHeight() - 1) * row + (0) * 4 + 1]);
					int b = static_cast<int>(data[(w->getImgHeight() - 1) * row + (0) * 4 + 2]);

					mirrored->setPixelColor(j, i, QColor(qRgba(b, g, r, 255)));

				}
				else if (i >= w->getImgHeight() + n && j > n&& j < w->getImgWidth() + n) {//dole stred

					int r = static_cast<int>(data[(w->getImgHeight() - 1) * row + (j - n) * 4 + 0]);
					int g = static_cast<int>(data[(w->getImgHeight() - 1) * row + (j - n) * 4 + 1]);
					int b = static_cast<int>(data[(w->getImgHeight() - 1) * row + (j - n) * 4 + 2]);

					mirrored->setPixelColor(j, i, QColor(qRgba(b, g, r, 255)));

				}
				else if (i >= w->getImgHeight() + n && j >= w->getImgWidth() + n) {//vpravo dole

					int r = static_cast<int>(data[(w->getImgHeight() - 1) * row + (w->getImgWidth() - 1) * 4 + 0]);
					int g = static_cast<int>(data[(w->getImgHeight() - 1) * row + (w->getImgWidth() - 1) * 4 + 1]);
					int b = static_cast<int>(data[(w->getImgHeight() - 1) * row + (w->getImgWidth() - 1) * 4 + 2]);

					mirrored->setPixelColor(j, i, QColor(qRgba(b, g, r, 255)));

				}
				else if (i > n&& j >= w->getImgWidth() + n && i < w->getImgHeight() + n) {//vpravo stred

					int r = static_cast<int>(data[(i - n) * row + (w->getImgWidth() - 1) * 4 + 0]);
					int g = static_cast<int>(data[(i - n) * row + (w->getImgWidth() - 1) * 4 + 1]);
					int b = static_cast<int>(data[(i - n) * row + (w->getImgWidth() - 1) * 4 + 2]);

					mirrored->setPixelColor(j, i, QColor(qRgba(b, g, r, 255)));

				}
				else if (i <= n && j >= w->getImgWidth() + n) {//vpravo hore

					int r = static_cast<int>(data[(0) * row + (w->getImgWidth() - 1) * 4 + 0]);
					int g = static_cast<int>(data[(0) * row + (w->getImgWidth() - 1) * 4 + 1]);
					int b = static_cast<int>(data[(0) * row + (w->getImgWidth() - 1) * 4 + 2]);

					mirrored->setPixelColor(j, i, QColor(qRgba(b, g, r, 255)));

				}
				else if (i <= n && j < w->getImgWidth() + n && j >= n) {//stred hore

					int r = static_cast<int>(data[(0) * row + (j - n) * 4 + 0]);
					int g = static_cast<int>(data[(0) * row + (j - n) * 4 + 1]);
					int b = static_cast<int>(data[(0) * row + (j - n) * 4 + 2]);

					mirrored->setPixelColor(j, i, QColor(qRgba(b, g, r, 255)));

				}

				//else {
				//	mirrored->setPixelColor(j, i, QColor(0, 0, 0));
				//}

			}
		}
	}

	
	return mirrored;
}

void ImageViewer::convolution()
{

}

void ImageViewer::readCSV()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Convolution mask (*.csv);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load mask", folder, fileFilter);
	if (fileName.isEmpty()) { return; }

	QFile f(fileName);
	

	if (f.open(QIODevice::ReadOnly)) {
		

		QTextStream stream(&f);
		QString line;
		QStringList list;
		int j = 0;
		do {
			line = stream.readLine();
			//qDebug() << line;
			if (line != "") {
				list=line.split(",");
				lineLength = list.length();
				for (int i = 0; i < list.length(); i++) {
					if (list.value(i) != "") {
						mask.push_back(list.value(i).toDouble());
						qDebug() << mask[lineLength * j + i];

					}
				}
				j++;
			}

		} while (!line.isNull());
	}
}

void ImageViewer::calcHist()
{
	ViewerWidget* w = getCurrentViewerWidget();
	uchar* data = w->getData();
	int row = w->getImage()->bytesPerLine();
	int depth = w->getImage()->depth();
	int calc = 0;

	for (int i = 0; i <= 255; i++) {
		histogramRed[i] = 0;
		histogramGreen[i] = 0;
		histogramBlue[i] = 0;
	}

	if (depth == 8) {
		for (int i = 0; i < w->getImgHeight(); i++)
		{
			for (int j = 0; j < w->getImgWidth(); j++)
			{
				uchar c = static_cast<uchar>(data[i * row + j]);
				histogramRed[c]++;
				histogramGreen[c]++;
				histogramBlue[c]++;

				calc++;
			}
		}

	}
	else {
		for (int i = 0; i < w->getImgHeight(); i++)
		{
			for (int j = 0; j < w->getImgWidth(); j++)
			{
				uchar b = static_cast<uchar>(data[i * row + j * 4]);
				uchar g = static_cast<uchar>(data[i * row + j * 4 + 1]);
				uchar r = static_cast<uchar>(data[i * row + j * 4 + 2]);

				histogramRed[r]++;
				histogramGreen[g]++;
				histogramBlue[b]++;
				calc = calc + 4;
			}
		}
	}


	//for (int i = 0; i <= 255; i++) {
	//	qDebug() << "red" << i << histogramRed[i];
	//	qDebug() << "green" << i << histogramGreen[i];
	//	qDebug() << "blue" << i << histogramBlue[i];
	//}

	//qDebug() << "height" << w->getImgHeight();
	//qDebug() << "width" << w->getImgWidth();

	//qDebug() << "calc" << calc;
}

void ImageViewer::IsodataTresholding()
{
	calcHist();

	ViewerWidget* w = getCurrentViewerWidget();
	uchar* data = w->getData();
	int row = w->getImage()->bytesPerLine();
	int depth = w->getImage()->depth();

	double epst = 0.001;
	double t = 100;
	double tnew=100;
	double meanLower, meanUpper;

	if (depth == 8) {
		do {
			t = tnew;
			meanLower = histogramMeanBetweenNumbers(0, t,histogramRed);
			meanUpper = histogramMeanBetweenNumbers(t, 255, histogramRed);
			tnew = (meanLower + meanUpper) / 2;
			/*qDebug() << "mean low" << meanLower;
			qDebug() << "mean up" << meanUpper;
			qDebug() << "mean tnew" << tnew;*/
			
		} while (abs(t - tnew) > epst);
	}
	qDebug() << "threshold" << tnew;
	QImage* newImage = new QImage(w->getImgWidth(), w->getImgHeight(), QImage::Format_Grayscale8);
	for (int j = 0; j < w->getImgWidth() ; j++)
	{
		for (int i = 0; i < w->getImgHeight(); i++)
		{
			uchar c = static_cast<uchar>(data[i * row + j ]);
			if (c < tnew) {
				newImage->setPixelColor(j, i, QColor(0, 0, 0));
			}
			else{
				newImage->setPixelColor(j, i, QColor(255, 255, 255));
			}

		}
	}
	w->setImage(*newImage);
	w->update();
}

double ImageViewer::histogramMeanBetweenNumbers(int lower, int upper,int* hist)
{
	double sum=0,wSum=0;

	for (int i = lower; i <= upper; i++) {
		sum += hist[i];
		wSum += hist[i] * i;
	}

	return  wSum / sum;
}

int ImageViewer::findMin(uchar* img, int row,int width,int height)
{
	int cmin=9999;

	for (int j = 0; j < width; j++)
	{
		for (int i = 0; i < height; i++)
		{
			if (cmin > img[i * row + j]) cmin = img[i * row + j];
		}
	}
	return cmin;
}

uchar* ImageViewer::BersenTresholding(uchar* data, int cmin, int r, int row, int width, int height)
{

	uchar q = 0;
	uchar* Q=new uchar[(width)*(height)];
	std::vector<uchar> mask;
	uchar min, max;
	double c;
	qDebug() << "uchar";

	for (int i = 0; i < height - 2 * r+1; i++) {
		for (int j = 0; j < width - 2 * r+1; j++) {
			min = 256;
			max = -1;

			for (int k = 0; k < 2 * r + 1; k++) {
				for (int l = 0; l < 2 * r + 1; l++) {

					mask.push_back(data[(i + k) * row + j + l]);

				}
			}
			max = *std::max_element(mask.begin(), mask.end());
			min = *std::min_element(mask.begin(), mask.end());

			mask.clear();
			//qDebug() << "min" << min;
			//qDebug() << "max" << max;
			//if(i>600 && j>600)qDebug() << i << j;
			c = max - min;

			if (c >= cmin) {
				
				Q[i * row + j] = (uchar)(min + max) / 2;
			}
			else {
				Q[i * row + j] = q;
			}
			//qDebug() << "uchar" << Q[i * row + j];
		}
	}
	qDebug() << "uchar";
	return Q;
}

//Slots

//Tabs slots
void ImageViewer::on_tabWidget_tabCloseRequested(int tabId)
{
	ViewerWidget* vW = getViewerWidget(tabId);
	vW->~ViewerWidget();
	ui->tabWidget->removeTab(tabId);
}
void ImageViewer::on_actionRename_triggered()
{
	if (!isImgOpened()) {
		msgBox.setText("No image is opened.");
		msgBox.setIcon(QMessageBox::Information);
		msgBox.exec();
		return;
	}
	ViewerWidget* w = getCurrentViewerWidget();
	bool ok;
	QString text = QInputDialog::getText(this, QString("Rename"), tr("Image name:"), QLineEdit::Normal, w->getName(), &ok);
	if (ok && !text.trimmed().isEmpty())
	{
		w->setName(text);
		ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), text);
	}
}

//Image slots
void ImageViewer::on_actionNew_triggered()
{
	newImgDialog = new NewImageDialog(this);
	connect(newImgDialog, SIGNAL(accepted()), this, SLOT(newImageAccepted()));
	newImgDialog->exec();
}
void ImageViewer::newImageAccepted()
{
	NewImageDialog* newImgDialog = static_cast<NewImageDialog*>(sender());

	int width = newImgDialog->getWidth();
	int height = newImgDialog->getHeight();
	QString name = newImgDialog->getName();
	openNewTabForImg(new ViewerWidget(name, QSize(width, height)));
	ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
}
void ImageViewer::on_actionOpen_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load image", folder, fileFilter);
	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	if (!openImage(fileName)) {
		msgBox.setText("Unable to open image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}	
}
void ImageViewer::on_actionSave_as_triggered()
{
	if (!isImgOpened()) {
		msgBox.setText("No image to save.");
		msgBox.setIcon(QMessageBox::Information);
		msgBox.exec();
		return;
	}
	QString folder = settings.value("folder_img_save_path", "").toString();

	ViewerWidget* w = getCurrentViewerWidget();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder + "/" + w->getName(), fileFilter);
	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

	if (!saveImage(fileName)) {
		msgBox.setText("Unable to save image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
	else {
		msgBox.setText(QString("File %1 saved.").arg(fileName));
		msgBox.setIcon(QMessageBox::Information);
		msgBox.exec();
	}
}
void ImageViewer::on_actionClear_triggered()
{
	if (!isImgOpened()) {
		msgBox.setText("No image is opened.");
		msgBox.setIcon(QMessageBox::Information);
		msgBox.exec();
		return;
	}
	clearImage();
}
void ImageViewer::on_actionInvert_colors_triggered() {
	if (!isImgOpened()) {
		msgBox.setText("No image is opened.");
		msgBox.setIcon(QMessageBox::Information);
		msgBox.exec();
		return;
	}
	invertColors();
}

void ImageViewer::on_actionMirroring_triggered()
{
	ViewerWidget* w = getCurrentViewerWidget();

	w->setImage(*mirroring(100));
	w->update();
}

void ImageViewer::on_actionHistogram_triggered()
{
	calcHist();
}

void ImageViewer::on_actionConvolution_triggered()
{
	readCSV();

	ViewerWidget* w = getCurrentViewerWidget();
	QImage *mirrored;
	QImage *convoluted;
	uchar* data;
	int n = 1;
	double c, vaha = 0, r, g, b;
	
	for (int k = 0; k < mask.size(); k++) {
		vaha += mask[k];
	}	

	mirrored = mirroring(1);
	
	data=mirrored->bits();
	
	int row = mirrored->bytesPerLine();
	int depth = mirrored->depth();

	if (depth == 8) {
		convoluted = new QImage(mirrored->width() - 2*n, mirrored->height() - 2*n, QImage::Format_Grayscale8);

		for (int i = 0; i < convoluted->height(); i++) {
			for (int j = 0; j < convoluted->width(); j++) {

				c = 0;

				for (int k = 0; k < lineLength; k++) {
					for (int l = 0; l < lineLength; l++) {

						c +=(double) data[(i+k) * row + j + l] *( mask[ k * lineLength + l]/vaha);
						
					}
				}
				
				if (c > 255) {
					c = 255;
				}
				if (c < 0) {
					c = 0;
				}
				//newData[i * newRow + j] = c;
				convoluted->setPixelColor(j, i, QColor(qRgba((uchar)c, (uchar)c, (uchar)c,255)));
				//w->setPixel(j, i, (uchar)c);
			}
		}
	}
	else {
		convoluted = new QImage(mirrored->width() - 2 * n, mirrored->height() - 2 * n, QImage::Format_ARGB32);

		for (int i = 0; i < convoluted->height() - 2 * n; i++) {
			for (int j = 0; j < convoluted->width() - 2 * n; j++) {

				r = 0;
				g = 0;
				b = 0;

				for (int k = 0; k < lineLength; k++) {
					for (int l = 0; l < lineLength; l++) {

						//c += data[i * row + j /* potialto xy suradnica*/ /*+ n * row + n*/ /*pripocitame aby sme sa dostali do laveho horneho rohu masky*/ + k * row + l] * mask[(lineLength * lineLength - 1) - (k * lineLength + l)];
						r += data[(i + k) * row + (j + l) * 4 + 0] * (mask[/*(lineLength * lineLength - 1) -*/ (k * lineLength + l)])/vaha;
						g += data[(i + k) * row + (j + l) * 4 + 1] * (mask[/*(lineLength * lineLength - 1) -*/ (k * lineLength + l)])/vaha;
						b += data[(i + k) * row + (j + l) * 4 + 2] * (mask[/*(lineLength * lineLength - 1) -*/ (k * lineLength + l)])/vaha;

					}
				}
				if (r > 255)r = 255;
				else if (r < 0)r = 0;

				if (g > 255)g = 255;
				else if (g < 0)g = 0;

				if (b > 255)b = 255;
				else if (b < 0)b = 0;

				convoluted->setPixelColor(j, i, QColor(qRgba((uchar)b,( uchar)g, (uchar)r,255)));
			}

		}
	}

	

	w->setImage(*convoluted);
	w->update();
}

void ImageViewer::on_actionIsodata_Treshold_triggered()
{
	IsodataTresholding();
}

void ImageViewer::on_actionBersen_Treshold_triggered()
{

	ViewerWidget* w = getCurrentViewerWidget();
	int cmin;
	int r = 5;
	uchar* data;
	uchar* threshold;
	int row;
	

	QImage* mirrored = mirroring(r);
	row = mirrored->bytesPerLine();

	data = mirrored->bits();
	cmin = findMin(data, row, mirrored->width(), mirrored->height());
	
	threshold=BersenTresholding(data, cmin, r, mirrored->bytesPerLine(), mirrored->width(), mirrored->height());


	QImage* newImage = new QImage(mirrored->width() - 2 * r, mirrored->height() - 2 * r, QImage::Format_Grayscale8);
	for (int i = 0; i < newImage->height(); i++) {
		for (int j = 0; j < newImage->width(); j++) {
			
			uchar c = static_cast<uchar>(data[i * row + j]);
			if (c < threshold[i*row+j]) {
				newImage->setPixelColor(j, i, QColor(0, 0, 0));
			}
			else {
				newImage->setPixelColor(j, i, QColor(255, 255, 255));
			}

		}
	}
	w->setImage(*newImage);
	w->update();

}
