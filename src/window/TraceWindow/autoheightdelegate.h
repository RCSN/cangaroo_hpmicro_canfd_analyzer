#pragma once

#include <QStyledItemDelegate>
#include <QTreeView>

class AutoHeightDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit AutoHeightDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
        QSize size = QStyledItemDelegate::sizeHint(option, index);

        // 转换为QTreeView获取列宽
         if (const QTreeView *treeView = qobject_cast<const QTreeView*>(option.widget))  {
            int colWidth = treeView->columnWidth(index.column());

            // 计算文本显示需要的高度
            QString text = index.data(Qt::DisplayRole).toString();
            QFontMetrics fm(option.font);
            QRect textRect = fm.boundingRect(
                0, 0,
                colWidth - 4,  // 减去边距
                INT_MAX,       // 允许任意高度
                Qt::TextWordWrap,
                text
            );

            return QSize(size.width(), textRect.height() + 4);
        }

        return size;
    }
};
