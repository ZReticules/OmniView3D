#ifndef ICONTOOLS_H
#define ICONTOOLS_H

#include <QPixmap>
#include <QIcon>
#include <QMenu>
#include <QColorDialog>

namespace IconTools{

template <typename T>
static inline void IconFill(T *widget, QColor newColor)
{
    QPixmap ipix(100, 100);
    ipix.fill(newColor);
    QIcon newIcon(ipix);
    widget->setIcon(newIcon);
};


static inline void UnsetActions(QMenu *menu)
{
    for(auto &i : menu->actions())
        i->setIconVisibleInMenu(false);
};

class IconizedColorDialog : public QColorDialog{
public:
    using QColorDialog::QColorDialog;
    void setIcon(QIcon icon){
        this->setWindowIcon(icon);
    }
};

}

#endif // ICONTOOLS_H
