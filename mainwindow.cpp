#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>

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


void MainWindow::on_pushExit_clicked()
{
    QApplication::exit();
}

void MainWindow::on_pushAbout_clicked()
{
    QMessageBox::about(this, tr("About Redux"),
             tr("Dear Users,<br>                                    "
                "       <b>Redux</b> is a cross platform application"
                "that will store your password in files that will be"
                " protected with Encryption. Feel free to contact   "
                "us on any reported bug.<br>                        "
                "Git-Hub: <a href='https://github.com/garvit-joshi/Redux\'>Click Here</a><br>"
                "Developers: <b>Saurav Jha</b>, <b>Garvit Joshi</b>"));
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About Redux"),
             tr("Dear Users,<br>                                    "
                "       <b>Redux</b> is a cross platform application"
                "that will store your password in files that will be"
                " protected with Encryption. Feel free to contact   "
                "us on any reported bug.<br>                        "
                "Git-Hub: <a href='https://github.com/garvit-joshi/Redux\'>Click Here</a><br>"
                "Developers: <b>Saurav Jha</b>, <b>Garvit Joshi</b>"));
}

void MainWindow::on_actionLicense_triggered()
{
    QMessageBox::about(this, tr("MIT License - Redux"),
             tr("MIT License<br><br>"

                "Copyright (c) 2020 Garvit Joshi<br><br>"

                "Permission is hereby granted, free of charge, to any person obtaining a copy"
                "of this software and associated documentation files (the 'Software'), to deal"
                "in the Software without restriction, including without limitation the rights"
                "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell"
                "copies of the Software, and to permit persons to whom the Software is"
                "furnished to do so, subject to the following conditions:<br><br>"

                "The above copyright notice and this permission notice shall be included in all"
                "copies or substantial portions of the Software.<br><br>"

                "THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR"
                "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,"
                "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE"
                "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER"
                "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,"
                "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE"
                "SOFTWARE.<br>"));
}
