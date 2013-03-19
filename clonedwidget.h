#ifndef CLONEDWIDGET_H
#define CLONEDWIDGET_H

#include <QWidget>

#include "scenepanel.h"
#include "scenewidget.h"

namespace Ui {
class ClonedWidget;
}

class ClonedWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ClonedWidget(qint32 compkey, QWidget *parent = 0);
    ~ClonedWidget();
    
private:
    Ui::ClonedWidget *ui;
    //CScenePanel *_scenePanel;
    CSceneWidget * _sceneWidget;

    void resizeEvent(QResizeEvent * event);
};

#endif // CLONEDWIDGET_H
