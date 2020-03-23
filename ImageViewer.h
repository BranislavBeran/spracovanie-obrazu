#pragma once

#include <QtWidgets/QMainWindow>
//#include <QtWidgets>
#include "ui_ImageViewer.h"
#include "ViewerWidget.h"
#include "NewImageDialog.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

class ImageViewer : public QMainWindow
{
	Q_OBJECT

public:
	ImageViewer(QWidget* parent = Q_NULLPTR);

private:
	Ui::ImageViewerClass* ui;
	NewImageDialog* newImgDialog;
	int histogramRed[256], histogramGreen[256], histogramBlue[256];
	std::vector<double> mask;
	int lineLength;


	QSettings settings;
	QMessageBox msgBox;

	//ViewerWidget functions
	ViewerWidget* getViewerWidget(int tabId);
	ViewerWidget* getCurrentViewerWidget();

	//Event filters
	bool eventFilter(QObject* obj, QEvent* event);

	//ViewerWidget Events
	bool ViewerWidgetEventFilter(QObject* obj, QEvent* event);
	void ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event);
	void ViewerWidgetLeave(ViewerWidget* w, QEvent* event);
	void ViewerWidgetEnter(ViewerWidget* w, QEvent* event);
	void ViewerWidgetWheel(ViewerWidget* w, QEvent* event);

	//ImageViewer Events
	void closeEvent(QCloseEvent* event);

	//Image functions
	void openNewTabForImg(ViewerWidget* vW);
	bool openImage(QString filename);
	bool saveImage(QString filename);
	bool clearImage();
	bool invertColors();
	QImage* mirroring(int n);
	void convolution();
	void readCSV();
	void calcHist();
	void IsodataTresholding();
	double histogramMeanBetweenNumbers(int lower, int upper, int* hist);
	int findMin(uchar* img, int row, int width, int height);
	uchar* BersenTresholding(uchar* data, int cmin, int r,int row,int width,int height);


	//Inline functions
	inline bool isImgOpened() { return ui->tabWidget->count() == 0 ? false : true; }

private slots:
	//Tabs slots
	void on_tabWidget_tabCloseRequested(int tabId);
	void on_actionRename_triggered();

	//Image slots
	void on_actionNew_triggered();
	void newImageAccepted();
	void on_actionOpen_triggered();
	void on_actionSave_as_triggered();
	void on_actionClear_triggered();
	void on_actionInvert_colors_triggered();
	void on_actionMirroring_triggered();
	void on_actionHistogram_triggered();
	void on_actionConvolution_triggered();
	void on_actionIsodata_Treshold_triggered();
	void on_actionBersen_Treshold_triggered();
};
