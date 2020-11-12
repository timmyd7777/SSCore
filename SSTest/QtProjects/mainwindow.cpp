#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include "SSTest.cpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    string inpath = ui->lineEdit->text().toStdString();
    string outpath = ui->lineEdit_2->text().toStdString();
    TestDeepSky(inpath, outpath);
}

void MainWindow::on_pushButton_2_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"));
    ui->lineEdit->setText(dir);
}

void MainWindow::on_pushButton_3_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"));
    ui->lineEdit_2->setText(dir);
}


void MainWindow::on_pushButton_4_clicked()
{
    string inpath = ui->lineEdit->text().toStdString();
    string outpath = ui->lineEdit_2->text().toStdString();
    TestStars (inpath, outpath);
}

void MainWindow::on_pushButton_5_clicked()
{
    string inpath = ui->lineEdit->text().toStdString();
    string outpath = ui->lineEdit_2->text().toStdString();
    TestEphemeris ( inpath, outpath );
}

void MainWindow::on_pushButton_6_clicked()
{
    string inpath = ui->lineEdit->text().toStdString();
    string outpath = ui->lineEdit_2->text().toStdString();
    TestSatellites ( inpath, outpath );
}

void MainWindow::on_pushButton_7_clicked()
{
    string inpath = ui->lineEdit->text().toStdString();
    string outpath = ui->lineEdit_2->text().toStdString();
    TestSolarSystem( inpath, outpath );
}

void MainWindow::on_pushButton_8_clicked()
{
    string inpath = ui->lineEdit->text().toStdString();
    string outpath = ui->lineEdit_2->text().toStdString();
    TestConstellations( inpath, outpath );
}

void MainWindow::on_pushButton_9_clicked()
{
    string inpath = ui->lineEdit->text().toStdString();
    string outpath = ui->lineEdit_2->text().toStdString();
    TestJPLDEphemeris( inpath);
}

void MainWindow::on_pushButton_10_clicked()
{
    string inpath = ui->lineEdit->text().toStdString();
    string outpath = ui->lineEdit_2->text().toStdString();
    TestELPMPP02( inpath);
}

void MainWindow::on_pushButton_11_clicked()
{
    string inpath = ui->lineEdit->text().toStdString();
    string outpath = ui->lineEdit_2->text().toStdString();
    TestVSOP2013( inpath);
}

void MainWindow::on_pushButton_12_clicked()
{
    TestPrecession();
}

void MainWindow::on_pushButton_13_clicked()
{
    TestTime();
}
