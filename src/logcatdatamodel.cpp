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
#include "logcatdatamodel.h"
#include "logcatdatamodel_def.h"

#include <regex>

#include <QProcessEnvironment>
#include <QMessageBox>
#include <QTimer>
#include <QTextStream>


#if defined(Q_OS_WIN)
static const auto APP_SUFFIX = QStringLiteral(".exe");
#else
static const auto APP_SUFFIX = QStringLiteral("");
#endif

static const auto SDK_ROOT = QStringLiteral("ANDROID_SDK_ROOT");


LogcatDataModel::LogcatDataModel(QObject* parent)
        : QAbstractTableModel(parent)
{
    const auto env = QProcessEnvironment::systemEnvironment();

    if (! env.contains(SDK_ROOT)) {
        QMessageBox msgBox;
        msgBox.setText(tr("Environment variable ANDROID_SDK_ROOT is not set. Fix it and re-launch the app."));
        msgBox.exec();
        QTimer::singleShot(0, qApp, &QCoreApplication::quit);
    }

    updateLogcatProcessList(QVector<int>());

    connect(&logcat_proc_, &QProcess::readyReadStandardOutput,
            this, &LogcatDataModel::onReadLogcatStdout);
    connect(&logcat_proc_, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, &LogcatDataModel::onLogcatFinished);

    const auto [cmd, args] = logcatCommand();
    logcat_proc_.start(cmd, args);
}


LogcatDataModel::~LogcatDataModel()
{}


int LogcatDataModel::rowCount(const QModelIndex&) const
{
    return logcat_data_.size();
}


int LogcatDataModel::columnCount(const QModelIndex&) const
{
    return Column_Count;
}


QString get_field(const std::string& raw_data, const LogcatField_t& f)
{
    auto res = QString(raw_data.substr(f[0], f[1]).c_str());
    //qDebug() << "  data: " << raw_data.c_str() << "\n  chars [" << f[0] << "," << f[1] << "]:" << res;
    return res;
}


QVariant LogcatDataModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole) {
        //qDebug() << "Getting row" << index.row();
        const auto& rec = logcat_data_.at(index.row());
        switch (index.column()) {
        case DATE_Column: return get_field(rec.raw_data, rec.date);
        case TIME_Column: return get_field(rec.raw_data, rec.time);
        case PID_Column: return get_field(rec.raw_data, rec.pid);
        case TID_Column: return get_field(rec.raw_data, rec.tid);
        case PPID_Column: return findProcessPPID(get_field(rec.raw_data, rec.pid));
        case NAME_Column: return findProcessName(get_field(rec.raw_data, rec.pid));
        case PRIORITY_Column: return get_field(rec.raw_data, rec.priority);
        case TAG_Column: return get_field(rec.raw_data, rec.tag);
        case MESSAGE_Column: return get_field(rec.raw_data, rec.message);
        }
        return QStringLiteral("???");
    }
    return QVariant();
}


QVariant LogcatDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case DATE_Column: return tr("Date");
        case TIME_Column: return tr("Time");
        case PID_Column: return QStringLiteral("PID");
        case TID_Column: return QStringLiteral("TID");
        case PPID_Column: return QStringLiteral("PPID");
        case NAME_Column: return tr("Name");
        case PRIORITY_Column: return tr("Priority");
        case TAG_Column: return tr("Tag");
        case MESSAGE_Column: return tr("Message");
        }
    }
    return QVariant();
}


void LogcatDataModel::onReadLogcatStdout()
{
    static int counter = 0;
    static const auto re = std::regex(R"(^(\d+-\d+)\s+(\d+:\d+:\d+\.\d+)\s+(\d+)\s+(\d+)\s+(\w+)\s+([^:]+?)\s*:\s(.+)$)");

    auto unknown_pids = QVector<int>();

    logcat_proc_.setReadChannel(QProcess::StandardOutput);
    auto stream = QTextStream(&logcat_proc_);
    while (! stream.atEnd()) {
        std::smatch match;
        auto line = stream.readLine().toStdString();
        if (std::regex_match(line, match, re)) {
            beginInsertRows(index(rowCount(), 0), rowCount(), rowCount());
            auto rec = LogcatRecord_t {
                line,
                {match.position(1), match.length(1)},
                {match.position(2), match.length(2)},
                {match.position(3), match.length(3)},
                {match.position(4), match.length(4)},
                {match.position(5), match.length(5)},
                {match.position(6), match.length(6)},
                {match.position(7), match.length(7)}
            };
            logcat_data_.emplace(counter, rec);
            counter += 1;
            int pid = std::stoi(match.str(3));
            auto it = logcat_proc_list_.find(pid);
            if (it == logcat_proc_list_.end()) { unknown_pids.push_back(pid); }
            endInsertRows();
        }
        if (0 == counter % 10) {
            break;  // to process other events in the app queue
        }
    }

    if (unknown_pids.size() > 0) {
        updateLogcatProcessList(unknown_pids);
    }
}


void LogcatDataModel::onLogcatFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    // TODO: handle reconnect
}


void LogcatDataModel::tearDown()
{
    logcat_proc_.kill();
}


std::tuple<QString, QStringList> LogcatDataModel::logcatCommand() const
{
    return {
        QString("%1/platform-tools/adb%2").arg(QProcessEnvironment::systemEnvironment().value(SDK_ROOT), APP_SUFFIX),
        {QStringLiteral("shell"), QStringLiteral("logcat"), QStringLiteral("-b"), QStringLiteral("default,events")}
    };
}


std::tuple<QString, QStringList> LogcatDataModel::psCommand() const
{
    return {
        QString("%1/platform-tools/adb%2").arg(QProcessEnvironment::systemEnvironment().value(SDK_ROOT), APP_SUFFIX),
        {QStringLiteral("shell"), QStringLiteral("ps"), QStringLiteral("-o"), QStringLiteral("USER,PID,PPID,NAME")}
    };
}


void LogcatDataModel::updateLogcatProcessList(const QVector<int>& pids)
{
    const auto [cmd, args] = psCommand();
    QProcess ps;
    ps.start(cmd, args);
    if (!ps.waitForStarted()) { return; }
    ps.waitForFinished();

    static const auto re = std::regex(R"(^(\S+)\s+(\d+)\s+(\d+)\s+(.+)$)");

    ps.setReadChannel(QProcess::StandardOutput);
    auto stream = QTextStream(&ps);
    while (! stream.atEnd()) {
        std::smatch match;
        auto line = stream.readLine().toStdString();
        if (std::regex_match(line, match, re)) {
            auto rec = LogcatProcessInfo_t {
                line,
                {match.position(1), match.length(1)},
                {match.position(2), match.length(2)},
                {match.position(3), match.length(3)},
                {match.position(4), match.length(4)}
            };
            auto pid = std::stoi(match.str(2));
            auto it = logcat_proc_list_.find(pid);
            if (it == logcat_proc_list_.end()) {
                logcat_proc_list_.emplace(pid, rec);
            } else {
                it->second = rec;
            }
        }
    }

    for (auto pid: pids) {
        auto it = logcat_proc_list_.find(pid);
        if (it == logcat_proc_list_.end()) {
            std::smatch match;
            auto line = QString("unknown %2 0 unknown").arg(pid).toStdString();
            if (std::regex_match(line, match, re)) {
                auto rec = LogcatProcessInfo_t {
                    line,
                    {match.position(1), match.length(1)},
                    {match.position(2), match.length(2)},
                    {match.position(3), match.length(3)},
                    {match.position(4), match.length(4)}
                };
                logcat_proc_list_.emplace(pid, rec);
            }
        }
    }
}


QString LogcatDataModel::findProcessName(const QString& pid) const
{
    auto it = logcat_proc_list_.find(pid.toInt());
    if (it != logcat_proc_list_.end()) {
        return get_field(it->second.raw_data, it->second.name);
    }
    return QStringLiteral("");
}


QString LogcatDataModel::findProcessPPID(const QString& pid) const
{
    auto it = logcat_proc_list_.find(pid.toInt());
    if (it != logcat_proc_list_.end()) {
        return get_field(it->second.raw_data, it->second.ppid);
    }
    return QStringLiteral("");
}
