/*
 *  Copyright 2012  Sebastian Gottfried <sebastiangottfried@web.de>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DATAACCESS_H
#define DATAACCESS_H

#include <QObject>

class QXmlSchema;
class QDomDocument;
class QFile;
class DataIndex;
class KeyboardLayout;
class Course;

class DataAccess : public QObject
{
    Q_OBJECT
public:
    explicit DataAccess(QObject *parent = 0);
    Q_INVOKABLE bool loadDataIndex(DataIndex* target);
    Q_INVOKABLE bool storeDataIndex(DataIndex* source);
    Q_INVOKABLE bool loadKeyboardLayout(const QString& path, KeyboardLayout* target);
    Q_INVOKABLE bool loadCourse(const QString& path, Course* target);
    QString storeUserCourse(const QString& fileName, Course* source);

private:
    QXmlSchema loadXmlSchema(const QString& name);
    QDomDocument getDomDocument(QFile& file, QXmlSchema& schema);
    bool openResourceFile(const QString& relPath, QFile& file);
};

#endif // DATAACCESS_H
