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
#include "logcatfilterproxy.h"
#include "logcatdatamodel_def.h"


LogcatFilterProxy::LogcatFilterProxy(QObject* parent)
        : QSortFilterProxyModel(parent)
{
    const auto empty = QStringLiteral("");
    setFilterPattern({
        {PID_Regex, empty},
        {PID_Regex_Inverted, empty},
        {PRIORITY_Regex, empty},
        {PRIORITY_Regex_Inverted, empty},
        {TAG_Regex, empty},
        {TAG_Regex_Inverted, empty},
        {NAME_Regex, empty},
        {NAME_Regex_Inverted, empty},
        {PPID_Regex, empty},
        {PPID_Regex_Inverted, empty}
    });
}


LogcatFilterProxy::~LogcatFilterProxy()
{}


bool LogcatFilterProxy::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    auto contains = [this](const QString& s, int regex_id, int flag_id) {
        const bool inverted = pattern_.at(flag_id).size() > 0;
        const bool contains = s.contains(regex_.at(regex_id));
        return (contains && ! inverted) || (! contains && inverted);
    };

    const auto model = sourceModel();
    const auto pidIndex = model->index(source_row, PID_Column, source_parent);
    const auto ppidIndex = model->index(source_row, PPID_Column, source_parent);
    const auto nameIndex = model->index(source_row, NAME_Column, source_parent);
    const auto priorityIndex = model->index(source_row, PRIORITY_Column, source_parent);
    const auto tagIndex = model->index(source_row, TAG_Column, source_parent);

    if (! contains(model->data(pidIndex).toString(), PID_Regex, PID_Regex_Inverted)) {return false;}
    if (! contains(model->data(ppidIndex).toString(), PPID_Regex, PPID_Regex_Inverted)) {return false;}
    if (! contains(model->data(nameIndex).toString(), NAME_Regex, NAME_Regex_Inverted)) {return false;}
    if (! contains(model->data(priorityIndex).toString(), PRIORITY_Regex, PRIORITY_Regex_Inverted)) {return false;}
    if (! contains(model->data(tagIndex).toString(), TAG_Regex, TAG_Regex_Inverted)) {return false;}

    return true;
}


void LogcatFilterProxy::setFilterPattern(LogcatFilterPattern_t&& pattern)
{
    pattern_ = std::move(pattern);
    regex_[PID_Regex] = QRegularExpression(pattern_[PID_Regex]);
    regex_[PPID_Regex] = QRegularExpression(pattern_[PPID_Regex]);
    regex_[NAME_Regex] = QRegularExpression(pattern_[NAME_Regex]);
    regex_[PRIORITY_Regex] = QRegularExpression(pattern_[PRIORITY_Regex]);
    regex_[TAG_Regex] = QRegularExpression(pattern_[TAG_Regex]);

    invalidate();
}
