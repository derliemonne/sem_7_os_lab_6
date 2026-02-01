#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    series = new QLineSeries(this);

    axisX = new QDateTimeAxis(this);
    axisX->setFormat("HH:mm:ss");

    axisY = new QValueAxis(this);
    axisY->setTitleText("Temperature");
    axisY->setRange(0, 100);

    chart = new QChart();
    chart->addSeries(series);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart, ui->chartWidget);
    chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout *layout = new QVBoxLayout(ui->chartWidget);
    layout->addWidget(chartView);
    ui->chartWidget->setLayout(layout);


    this->setWindowFlags(Qt::FramelessWindowHint);
    this->showFullScreen();
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open log file"),
        "",
        tr("Text Files (*.txt *.log);;All Files (*.*)")
        );

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this,
                              tr("Error"),
                              tr("Cannot open file:\n%1").arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString fullText = in.readAll();
    ui->textEdit->setPlainText(fullText);

    series->clear();
    QStringList lines = fullText.split('\n', Qt::SkipEmptyParts);

    QDateTime minTime, maxTime;
    double minTemp = 1e9;
    double maxTemp = -1e9;

    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty())
            continue;

        QStringList parts = trimmed.split(',');
        if (parts.size() != 2)
            continue;

        QDateTime dt = QDateTime::fromString(
            parts[0].trimmed(),
            "yyyy-MM-dd HH:mm:ss"
            );

        bool ok;
        double temp = parts[1].trimmed().toDouble(&ok);

        if (!dt.isValid() || !ok)
            continue;

        qint64 x = dt.toMSecsSinceEpoch();
        series->append(x, temp);

        if (!minTime.isValid() || dt < minTime) minTime = dt;
        if (!maxTime.isValid() || dt > maxTime) maxTime = dt;

        minTemp = qMin(minTemp, temp);
        maxTemp = qMax(maxTemp, temp);
    }

    file.close();

    if (series->count() == 0) {
        QMessageBox::warning(this,
                             tr("Warning"),
                             tr("No valid data found in file"));
        return;
    }

    axisX->setRange(minTime, maxTime);
    axisY->setRange(minTemp - 1, maxTemp + 1);
}



