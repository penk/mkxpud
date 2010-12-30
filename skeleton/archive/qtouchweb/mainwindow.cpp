/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>
#include <QtWebKit>
#include "mainwindow.h"
#include "qtouchweb.h"

#include <QToolButton>
#include <QStyle>

LineEdit::LineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    clearButton = new QToolButton(this);
    QPixmap pixmap("reload.png");
    clearButton->setIcon(QIcon(pixmap));
    clearButton->setIconSize(QSize(16, 16));
    clearButton->setCursor(Qt::ArrowCursor);
    clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");

    connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateCloseButton(const QString&)));

    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(clearButton->sizeHint().width() + frameWidth + 1));
    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width(), clearButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), clearButton->sizeHint().height() + frameWidth * 2 + 2));
}

void LineEdit::resizeEvent(QResizeEvent *)
{
    QSize sz = clearButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    clearButton->move(rect().right() - frameWidth - sz.width(),
                      (rect().bottom() + 1 - sz.height())/2);
}

void LineEdit::updateCloseButton(const QString& text)
{
    clearButton->setVisible(!text.isEmpty());
}

QWebPage *WebPage::createWindow(QWebPage::WebWindowType)
{
	MainWindow *mw = new MainWindow;
	mw->show();
	return mw->webPage();
}

QWebPage *MainWindow::webPage()
{
	return view->page();
}

MainWindow::MainWindow()
{
    progress = 0;

    QNetworkProxyFactory::setUseSystemConfiguration(true);
	setStyleSheet("QPushButton { border: none; padding: 5px; }");

    view = new QWebView(this);
	WebPage* page = new WebPage(view);
	view->setPage(page);
	view->load(QUrl("http://www.google.com/ncr"));

	FlickCharm *flickCharm = new FlickCharm(this);
	flickCharm->activateOn(view);

    connect(view, SIGNAL(loadFinished(bool)), SLOT(adjustLocation()));
    connect(view, SIGNAL(titleChanged(QString)), SLOT(adjustTitle()));
    connect(view, SIGNAL(loadProgress(int)), SLOT(setProgress(int)));
    connect(view, SIGNAL(loadFinished(bool)), SLOT(finishLoading(bool)));

    locationEdit = new LineEdit(this);
    locationEdit->setSizePolicy(QSizePolicy::Expanding, locationEdit->sizePolicy().verticalPolicy());
    connect(locationEdit, SIGNAL(returnPressed()), SLOT(changeLocation()));

    QToolBar *toolBar = addToolBar(tr("Navigation"));
	toolBar->setMovable(false);

	QPushButton *backBtn = new QPushButton(QPixmap("arrowleft.png"), "");
	QAction *backAction = view->pageAction(QWebPage::Back);
	connect(backBtn, SIGNAL(clicked()), backAction, SLOT(trigger()));

	QPushButton *forwardBtn = new QPushButton(QPixmap("arrowright.png"), "");
	QAction *forwardAction = view->pageAction(QWebPage::Forward);
	connect(forwardBtn, SIGNAL(clicked()), forwardAction, SLOT(trigger()));

	QPushButton *tabBtn = new QPushButton(QPixmap("plus.png"), "");
	connect(tabBtn, SIGNAL(clicked()), this, SLOT(newWindow()));

	toolBar->addWidget(backBtn);
	toolBar->addWidget(forwardBtn);
    toolBar->addWidget(locationEdit);
    toolBar->addWidget(tabBtn);
//	toolBar->addAction(view->pageAction(QWebPage::Reload));

    setCentralWidget(view);
}

void MainWindow::adjustLocation()
{
    locationEdit->setText(view->url().toString());
}

void MainWindow::newWindow()
{
	MainWindow* mw = new MainWindow();
	mw->show();
}

void MainWindow::changeLocation()
{
	QString string = locationEdit->text();
	QUrl url = QUrl::fromUserInput(string);
        if (!url.isValid())
            url = QUrl("http://" + string + "/");
	locationEdit->setText(url.toEncoded());
    view->load(url);
}

void MainWindow::adjustTitle()
{
    if (progress <= 0 || progress >= 100)
        setWindowTitle(view->title());
    else
        setWindowTitle(QString("%1 (%2%)").arg(view->title()).arg(progress));
}

void MainWindow::setProgress(int p)
{
    progress = p;
    adjustTitle();
}

void MainWindow::finishLoading(bool)
{
    progress = 100;
    adjustTitle();
}
