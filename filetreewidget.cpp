#include "filetreewidget.h"
#include "mainwindow.h"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QItemSelectionModel>

FileTreeWidget::FileTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

QTreeWidgetItem* FileTreeWidget::targetGroupItemForPos(const QPoint& viewportPos) const
{
    QTreeWidgetItem* target = itemAt(viewportPos);
    if (!target) return nullptr;
    if (target->data(0, ITEM_KIND_ROLE).toInt() == ITEM_GROUP) return target;
    return target->parent();
}

QString FileTreeWidget::targetGroupForPos(const QPoint& viewportPos) const
{
    QTreeWidgetItem* g = targetGroupItemForPos(viewportPos);
    return g ? g->text(0) : QString();
}

bool FileTreeWidget::isAcceptableDrop(const QPoint& viewportPos) const
{
    if (!m_dragSource) return false;
    if (m_dragSource->data(0, ITEM_KIND_ROLE).toInt() != ITEM_NOTE) return false;

    QString srcGroup = m_dragSource->parent() ? m_dragSource->parent()->text(0) : QString();
    QString dstGroup = targetGroupForPos(viewportPos);
    return dstGroup != srcGroup;
}

void FileTreeWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->source() != this)
    {
        event->ignore();
        return;
    }

    QList<QTreeWidgetItem*> sel = selectedItems();
    m_dragSource = sel.isEmpty() ? nullptr : sel.first();
    if (!m_dragSource || m_dragSource->data(0, ITEM_KIND_ROLE).toInt() != ITEM_NOTE)
    {
        event->ignore();
        return;
    }
    event->acceptProposedAction();
}

void FileTreeWidget::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->source() == this && isAcceptableDrop(event->pos()))
    {
        QTreeWidgetItem* g = targetGroupItemForPos(event->pos());
        if (g)
        {
            setCurrentItem(g);
        }
        else
        {
            selectionModel()->clearCurrentIndex();
            clearSelection();
        }
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void FileTreeWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    if (m_dragSource) setCurrentItem(m_dragSource);
    m_dragSource = nullptr;
    QTreeWidget::dragLeaveEvent(event);
}

void FileTreeWidget::dropEvent(QDropEvent* event)
{
    if (!isAcceptableDrop(event->pos()))
    {
        event->ignore();
        return;
    }

    QTreeWidgetItem* src = m_dragSource;
    QString dstGroup = targetGroupForPos(event->pos());
    m_dragSource = nullptr;

    event->setDropAction(Qt::IgnoreAction);
    event->accept();

    emit noteDropped(src, dstGroup);
}
