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

#include "pch.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>


MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainWindow)
{
    setWindowFlags(Qt::Window);
    ui->setupUi(this);
    ui->tableView->verticalHeader()->setDefaultSectionSize(20);

    fm = new LogcatFilterProxy(this);
    ui->tableView->setModel(fm);

    dm = new LogcatDataModel(this);
    fm->setSourceModel(dm);

    loadSettings();

    connect(qApp, &QCoreApplication::aboutToQuit, this, &MainWindow::onAboutToQuit);
    connect(fm, &LogcatFilterProxy::rowsInserted, this, &MainWindow::onRowsInserted);
    connect(dm, &LogcatDataModel::dataChanged, fm, &LogcatFilterProxy::invalidate);
}


MainWindow::~MainWindow()
{
    delete ui;
}


static const auto app_name_str = QStringLiteral("qLogcat");
static const auto app_pos_str = QStringLiteral("pos");
static const auto app_size_str = QStringLiteral("size");
static const auto log_autoscroll_str = QStringLiteral("log_autoscroll");
static const auto log_col_width_str = QStringLiteral("log_column_widths");


void MainWindow::loadSettings()
{
    auto s = QSettings(QSettings::IniFormat, QSettings::UserScope, app_name_str, app_name_str);

    s.beginGroup(QStringLiteral("MainWindow"));
    move(s.value(app_pos_str, QPoint(200, 200)).toPoint());
    resize(s.value(app_size_str, QSize(800, 600)).toSize());
    ui->autoscrollFlag->setChecked(s.value(log_autoscroll_str, false).toBool());
    ui->tableView->horizontalHeader()->restoreState(s.value(log_col_width_str).toByteArray());
    s.endGroup();
}


void MainWindow::saveSettings() const
{
    auto s = QSettings(QSettings::IniFormat, QSettings::UserScope, app_name_str, app_name_str);

    s.beginGroup(QStringLiteral("MainWindow"));
    s.setValue(app_pos_str, pos());
    s.setValue(app_size_str, size());
    s.setValue(log_autoscroll_str, ui->autoscrollFlag->isChecked());
    s.setValue(log_col_width_str, ui->tableView->horizontalHeader()->saveState());
    s.endGroup();
}


void MainWindow::onAboutToQuit()
{
    saveSettings();
    if (dm) { dm->tearDown(); }
}


void MainWindow::onRowsInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);

    if (ui->autoscrollFlag->checkState() == Qt::Checked) {
        ui->tableView->scrollTo(fm->index(last, 0));
    }
}


void MainWindow::on_autosizeBtn_clicked()
{
    ui->tableView->resizeColumnsToContents();
}


void MainWindow::on_filterBtn_clicked()
{
    const auto regular = QStringLiteral("");
    const auto inverted = QStringLiteral("yes");

    auto get_inverted = [&regular, &inverted](auto cb) {
        return cb->checkState() == Qt::Checked ? inverted : regular;
    };

    fm->setFilterPattern({
        {PID_Regex, ui->pidFilterEdit->text()},
        {PID_Regex_Inverted, get_inverted(ui->pidFilterInvertedFlag)},
        {PRIORITY_Regex, ui->priorityFilterEdit->text()},
        {PRIORITY_Regex_Inverted, get_inverted(ui->priorityFilterInvertedFlag)},
        {TAG_Regex, ui->tagFilterEdit->text()},
        {TAG_Regex_Inverted, get_inverted(ui->tagFilterInvertedFlag)},
        {NAME_Regex, ui->nameFilterEdit->text()},
        {NAME_Regex_Inverted, get_inverted(ui->nameFilterInvertedFlag)},
        {PPID_Regex, ui->ppidFilterEdit->text()},
        {PPID_Regex_Inverted, get_inverted(ui->ppidFilterInvertedFlag)}
    });
}
