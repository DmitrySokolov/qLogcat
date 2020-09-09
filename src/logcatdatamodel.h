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

#ifndef LOGCATDATAMODEL_H
#define LOGCATDATAMODEL_H

#include <unordered_map>
#include <tuple>

#include <QAbstractTableModel>
#include <QProcess>


using LogcatField_t = ptrdiff_t[2];


struct LogcatRecord_t
{
    std::string raw_data;
    LogcatField_t date;
    LogcatField_t time;
    LogcatField_t pid;
    LogcatField_t tid;
    LogcatField_t priority;
    LogcatField_t tag;
    LogcatField_t message;
};

using LogcatData_t = std::unordered_map<int, LogcatRecord_t>;


struct LogcatProcessInfo_t
{
    std::string raw_data;
    LogcatField_t user;
    LogcatField_t pid;
    LogcatField_t ppid;
    LogcatField_t name;
};

using LogcatProcessList_t = std::unordered_map<int, LogcatProcessInfo_t>;


class LogcatDataModel : public QAbstractTableModel
{
    Q_OBJECT

  public:
    LogcatDataModel(QObject *parent);
    virtual ~LogcatDataModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  public slots:
    void onReadLogcatStdout();
    void onLogcatFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void tearDown();

  protected:
    virtual std::tuple<QString, QStringList> logcatCommand() const;
    virtual std::tuple<QString, QStringList> psCommand() const;
    virtual void updateLogcatProcessList(const QVector<int>& pids);
    virtual QString findProcessName(const QString& pid) const;
    virtual QString findProcessPPID(const QString& pid) const;

  protected:
    QProcess logcat_proc_;
    LogcatData_t logcat_data_;
    LogcatProcessList_t logcat_proc_list_;
};


#endif // LOGCATDATAMODEL_H
