#ifndef FILETREEWIDGET_H
#define FILETREEWIDGET_H

#include <QTreeWidget>

class FileTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit FileTreeWidget(QWidget* parent = nullptr);

signals:
    void noteDropped(QTreeWidgetItem* noteItem, const QString& targetGroup);

protected:
    void dragEnterEvent(QDragEnterEvent* event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent* event) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent* event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent* event) Q_DECL_OVERRIDE;

private:
    QTreeWidgetItem* targetGroupItemForPos(const QPoint& viewportPos) const;
    QString targetGroupForPos(const QPoint& viewportPos) const;
    bool isAcceptableDrop(const QPoint& viewportPos) const;

    QTreeWidgetItem* m_dragSource = nullptr;
};

#endif // FILETREEWIDGET_H
