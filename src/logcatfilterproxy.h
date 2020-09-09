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

#ifndef LOGCATFILTERPROXY_H
#define LOGCATFILTERPROXY_H

#include <unordered_map>

#include <QSortFilterProxyModel>
#include <QRegularExpression>


const int PID_Regex = 1;
const int PID_Regex_Inverted = 2;
const int PRIORITY_Regex = 3;
const int PRIORITY_Regex_Inverted = 4;
const int TAG_Regex = 5;
const int TAG_Regex_Inverted = 6;
const int NAME_Regex = 7;
const int NAME_Regex_Inverted = 8;
const int PPID_Regex = 9;
const int PPID_Regex_Inverted = 10;


using LogcatFilterPattern_t = std::unordered_map<int, QString>;
using LogcatFilterRegex_t = std::unordered_map<int, QRegularExpression>;


class LogcatFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    LogcatFilterProxy(QObject *parent);
    virtual ~LogcatFilterProxy();

  protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

  public:
    void setFilterPattern(LogcatFilterPattern_t&& pattern);

  protected:
    LogcatFilterPattern_t pattern_;
    LogcatFilterRegex_t regex_;
};


#endif // LOGCATFILTERPROXY_H
