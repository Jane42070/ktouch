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

#include "resourceeditor.h"

#include <QUuid>
#include <QFile>
#include <QAbstractItemView>

#include <KToolBar>
#include <KAction>
#include <KActionCollection>
#include <KStandardAction>
#include <KIcon>
#include <KLocale>
#include <KMessageBox>

#include "core/dataindex.h"
#include "core/dataaccess.h"
#include "core/resource.h"
#include "core/course.h"
#include "core/keyboardlayout.h"
#include "resourceeditorwidget.h"
#include "resourcemodel.h"
#include "categorizedresourcesortfilterproxymodel.h"
#include "newresourceassistant.h"

ResourceEditor::ResourceEditor(QWidget *parent) :
    KMainWindow(parent),
    m_dataIndex(new DataIndex(this)),
    m_resourceModel(new ResourceModel(m_dataIndex, this)),
    m_currentResource(0),
    m_backupResource(0),
    m_actionCollection(new KActionCollection(this)),
    m_newResourceAction(KStandardAction::openNew(this, SLOT(newResource()), m_actionCollection)),
    m_deleteResourceAction(new KAction(KIcon("edit-delete"), i18n("Delete"), this)),
    m_undoAction(KStandardAction::undo(this, SLOT(undo()), m_actionCollection)),
    m_redoAction(KStandardAction::redo(this, SLOT(redo()), m_actionCollection)),
    m_importResourceAction(new KAction(KIcon("document-import"), i18n("Import"), this)),
    m_exportResourceAction(new KAction(KIcon("document-export"), i18n("Export"), this)),
    m_editorWidget(new ResourceEditorWidget(this))

{
    DataAccess dataAccess;
    dataAccess.loadDataIndex(m_dataIndex);
    CategorizedResourceSortFilterProxyModel* categorizedResourceModel = new CategorizedResourceSortFilterProxyModel(this);
    categorizedResourceModel->setSourceModel(m_resourceModel);
    categorizedResourceModel->setCategorizedModel(true);

    setMinimumSize(700, 500);
    setCaption(i18n("Course and Keyboard Layout Editor"));

    m_deleteResourceAction->setEnabled(false);
    m_actionCollection->addAction("deleteResource", m_deleteResourceAction);
    connect(m_deleteResourceAction, SIGNAL(triggered()), SLOT(deleteResource()));
    m_undoAction->setEnabled(false);
    m_redoAction->setEnabled(false);
    m_actionCollection->addAction("importResource", m_importResourceAction);
    connect(m_importResourceAction, SIGNAL(triggered()), SLOT(importResource()));
    m_actionCollection->addAction("exportResource", m_exportResourceAction);
    connect(m_exportResourceAction, SIGNAL(triggered()), SLOT(exportResource()));
    m_exportResourceAction->setEnabled(false);

    toolBar()->addAction(m_newResourceAction);
    toolBar()->addAction(m_deleteResourceAction);
    toolBar()->addSeparator();
    toolBar()->addAction(m_undoAction);
    toolBar()->addAction(m_redoAction);
    toolBar()->addSeparator();
    toolBar()->addAction(m_importResourceAction);
    toolBar()->addAction(m_exportResourceAction);

    setCentralWidget(m_editorWidget);

    QAbstractItemView* const resourceView = m_editorWidget->resourceView();
    resourceView->setModel(categorizedResourceModel);
    connect(resourceView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(onResourceSelected()));

    selectFirstResource();

    connect(m_editorWidget, SIGNAL(resourceRestorationRequested()), SLOT(restoreResourceBackup()));
    connect(m_editorWidget, SIGNAL(resourceRestorationDismissed()), SLOT(clearResourceBackup()));
}

ResourceEditor::~ResourceEditor()
{
    if (m_backupResource)
    {
        delete m_backupResource;
    }
}


void ResourceEditor::newResource()
{
    NewResourceAssistant assistant(m_resourceModel, this);

    if (assistant.exec() == QDialog::Accepted)
    {
        Resource* resource = assistant.createResource();
        if (Resource* dataIndexResource = addResource(resource))
        {
            selectDataResource(dataIndexResource);
        }
        delete resource;
    }
}

void ResourceEditor::deleteResource()
{
    Q_ASSERT(m_currentResource);

    DataAccess dataAccess;

    if (DataIndexCourse* course = qobject_cast<DataIndexCourse*>(m_currentResource))
    {
        for (int i = 0; i < m_dataIndex->courseCount(); i++)
        {
            if (m_dataIndex->course(i) == course)
            {
                Course* backup = new Course();
                if (!dataAccess.loadCourse(m_dataIndex->course(i)->path(), backup))
                {
                    KMessageBox::error(this, i18n("Error while opening course"));
                    return;
                }
                QFile file;
                file.setFileName(course->path());
                if (!file.remove())
                {
                    delete backup;
                    KMessageBox::error(this, i18n("Error while deleting course"));
                    return;
                }
                prepareResourceRestore(backup);
                m_dataIndex->removeCourse(i);
                if (!dataAccess.storeDataIndex(m_dataIndex))
                {
                    KMessageBox::error(this, i18n("Error while saving data index to disk."));
                    return;
                }
            }
        }
    }
    else if (DataIndexKeyboardLayout* keyboardLayout = qobject_cast<DataIndexKeyboardLayout*>(m_currentResource))
    {
        for (int i = 0; i < m_dataIndex->keyboardLayoutCount(); i++)
        {
            if (m_dataIndex->keyboardLayout(i) == keyboardLayout)
            {
                KeyboardLayout* backup = new KeyboardLayout();
                if (!dataAccess.loadKeyboardLayout(m_dataIndex->keyboardLayout(i)->path(), backup))
                {
                    KMessageBox::error(this, i18n("Error while opening keyboard layout"));
                    return;
                }
                QFile file;
                file.setFileName(keyboardLayout->path());
                if (!file.remove())
                {
                    KMessageBox::error(this, i18n("Error while deleting keyboard layout"));
                    return;
                }
                prepareResourceRestore(backup);
                m_dataIndex->removeKeyboardLayout(i);
                if (!dataAccess.storeDataIndex(m_dataIndex))
                {
                    KMessageBox::error(this, i18n("Error while saving data index to disk."));
                    return;
                }
            }
        }
    }

    selectFirstResource();
}

void ResourceEditor::undo()
{
}

void ResourceEditor::redo()
{
}

void ResourceEditor::importResource()
{
}

void ResourceEditor::exportResource()
{
}

void ResourceEditor::onResourceSelected()
{
    QAbstractItemView* const resourceView = m_editorWidget->resourceView();

    if (resourceView->selectionModel()->hasSelection())
    {
        QModelIndex current = resourceView->selectionModel()->selectedIndexes().first();
        const QVariant variant = resourceView->model()->data(current, ResourceModel::DataRole);
        QObject* const obj = variant.value<QObject*>();
        m_currentResource = qobject_cast<Resource*>(obj);
        const DataIndex::Source source = static_cast<DataIndex::Source>(resourceView->model()->data(current, ResourceModel::SourceRole).toInt());

        Q_ASSERT(m_currentResource);

        m_deleteResourceAction->setEnabled(source == DataIndex::UserResource);
        m_exportResourceAction->setEnabled(true);
        // TODO update editor
    }
    else
    {
        m_currentResource = 0;
        m_deleteResourceAction->setEnabled(false);
        m_exportResourceAction->setEnabled(false);
    }
}

void ResourceEditor::restoreResourceBackup()
{
    Q_ASSERT(m_backupResource);

    if (Resource* dataIndexResource = addResource(m_backupResource))
    {
        selectDataResource(dataIndexResource);
    }

    clearResourceBackup();
}

void ResourceEditor::clearResourceBackup()
{
    Q_ASSERT(m_backupResource);

    delete m_backupResource;
    m_backupResource = 0;
}

void ResourceEditor::prepareResourceRestore(Resource* backup)
{
    QString msg;

    if (Course* course = qobject_cast<Course*>(backup))
    {
        msg = i18n("Course \"%1\" deleted.").arg(course->title());
    }
    else if (KeyboardLayout* keyboardLayout = qobject_cast<KeyboardLayout*>(backup))
    {
        msg = i18n("Keyboard layout \"%1\" deleted.").arg(keyboardLayout->title());
    }

    m_editorWidget->showMessage(ResourceEditorWidget::ResourceDeletedMsg, msg);

    if (m_backupResource)
    {
        delete m_backupResource;
    }

    m_backupResource = backup;
}

Resource* ResourceEditor::addResource(Resource* resource)
{
    DataAccess dataAccess;
    Resource* dataIndexResource = 0;

    dataAccess.loadDataIndex(m_dataIndex);

    if (Course* course = qobject_cast<Course*>(resource))
    {
        const QString fileName = QString("%1.xml").arg(course->id());
        QString path = dataAccess.storeUserCourse(fileName, course);
        if (path.isNull())
        {
            KMessageBox::error(this, i18n("Error while saving course to disk."));
            return 0;
        }

        DataIndexCourse* dataIndexCourse = new DataIndexCourse();

        dataIndexCourse->setSource(DataIndex::UserResource);
        dataIndexCourse->setTitle(course->title());
        dataIndexCourse->setDescription(course->description());
        dataIndexCourse->setKeyboardLayoutName(course->keyboardLayoutName());
        dataIndexCourse->setPath(path);

        m_dataIndex->addCourse(dataIndexCourse);
        if (!dataAccess.storeDataIndex(m_dataIndex))
        {
            KMessageBox::error(this, i18n("Error while saving data index to disk."));
            return 0;
        }

        dataIndexResource = dataIndexCourse;
    }
    else if (KeyboardLayout* keyboardLayout = qobject_cast<KeyboardLayout*>(resource))
    {
        const QString fileName = QString("%1.xml").arg(QUuid::createUuid());
        QString path = dataAccess.storeUserKeyboardLayout(fileName, keyboardLayout);
        if (path.isNull())
        {
            KMessageBox::error(this, i18n("Error while saving keyboard layout to disk."));
            return 0;
        }

        DataIndexKeyboardLayout* dataIndexKeyboardLayout = new DataIndexKeyboardLayout();

        dataIndexKeyboardLayout->setSource(DataIndex::UserResource);
        dataIndexKeyboardLayout->setName(keyboardLayout->name());
        dataIndexKeyboardLayout->setTitle(keyboardLayout->title());
        dataIndexKeyboardLayout->setPath(path);

        m_dataIndex->addKeyboardLayout(dataIndexKeyboardLayout);
        if (!dataAccess.storeDataIndex(m_dataIndex))
        {
            KMessageBox::error(this, i18n("Error while saving data index to disk."));
            return 0;
        }

        dataIndexResource = dataIndexKeyboardLayout;
    }

    return dataIndexResource;
}


void ResourceEditor::selectDataResource(Resource* resource)
{
    QAbstractItemView* const resourceView = m_editorWidget->resourceView();
    QAbstractItemModel* const model = resourceView->model();

    for (int i = 0; i < model->rowCount(); i++)
    {
        const QModelIndex index = model->index(i, 0);
        const QVariant var = model->data(index, ResourceModel::DataRole);
        QObject* obj = var.value<QObject*>();

        if (obj == resource)
        {
            resourceView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
            break;
        }
    }
}

void ResourceEditor::selectFirstResource()
{
    QAbstractItemView* const resourceView = m_editorWidget->resourceView();

    if (resourceView->model()->rowCount() > 0)
    {
        resourceView->selectionModel()->select(resourceView->model()->index(0, 0), QItemSelectionModel::ClearAndSelect);
    }
}
