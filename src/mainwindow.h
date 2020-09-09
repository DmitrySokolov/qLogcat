//
// Copyright 2020 Dmitry Sokolov <mr.dmitry.sokolov@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include "logcatfilterproxy.h"
#include "logcatdatamodel.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QDialog
{
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  private slots:
    void onAboutToQuit();
    void onRowsInserted(const QModelIndex &parent, int first, int last);

    void on_autosizeBtn_clicked();
    void on_filterBtn_clicked();

  private:
    Ui::MainWindow* ui;
    LogcatFilterProxy* fm;
    LogcatDataModel* dm;
};


#endif // MAINWINDOW_H
