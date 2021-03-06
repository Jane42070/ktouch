/*
 * Copyright 2012  Sebastian Gottfried <sebastiangottfried@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CHARACTERSVIEWDELEGATE_H
#define CHARACTERSVIEWDELEGATE_H

#include <QStyledItemDelegate>

class KeyboardLayout;

class CharactersViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CharactersViewDelegate(QObject* parent = nullptr);
    KeyboardLayout* keyboardLayout() const;
    void setKeyboardLayout(KeyboardLayout* keyboardLayout);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const  override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel* model, const QModelIndex& index) const override;
private:
    KeyboardLayout* m_keyboardLayout;
};

#endif // CHARACTERSVIEWDELEGATE_H
