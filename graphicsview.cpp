#include "graphicsview.h"

#include <QWheelEvent>
#include <QPushButton>
#include <QLabel>
#include <qmath.h>

#include "IManager.h"
#include "clonedwidget.h"

extern void myImageCleanupHandler(void *info);

CSceneWidget::CSceneWidget(qint32 compkey, qint32 width, qint32 height, QWidget *parent) :
    QGraphicsView(parent),
    _compkey(compkey),
    _currentItem(NULL),
    m_currentImage(NULL),
    _resizeBegin(false),
    m_gridEnabled(false),
    m_cellWidth(10),
    _timerId(0)
{
    initMenu();

    _scene = new QGraphicsScene(this);
    _scene->setSceneRect(0,0,width,height);
    _scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    //fitInView(0,0,width,height,Qt::KeepAspectRatio);
    setSceneRect(0,0,width,height);
    setScene(_scene);
    setTransformationAnchor(AnchorUnderMouse);
    //setDragMode(ScrollHandDrag);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing
                                      | QGraphicsView::DontClipPainter
                                      | QGraphicsView::DontSavePainterState);

    //setRenderHint(QPainter::Antialiasing);
    _timerId = startTimer(1000 / 25);
    setMouseTracking(true);

    CGraphicsItem *background = new CGraphicsItem(_compkey);
    background->setPos(0,0);
    background->setSize(QSize(width,height));
    background->setImageFitMode(CGraphicsItem::ImageFit);
    _scene->addItem(background);

    new QLabel("use +/- for zoming", this);
}

void CSceneWidget::initMenu()
{
    _menu = new QMenu(viewport());

    QAction *action;

    action = new QAction(tr("Apply"), this);
    connect(action, SIGNAL(triggered()), SLOT(onApplyTriggered()));
    _menu->addAction(action);

    action = new QAction(tr("Hide boxs"), this);
    action->setCheckable(true);
    connect(action, SIGNAL(triggered(bool)), SLOT(onHideBoxTriggerd(bool)));
    _menu->addAction(action);

    action = new QAction(tr("Show grid"), this);
    action->setCheckable(true);
    connect(action, SIGNAL(triggered(bool)), SLOT(setGridVisible(bool)));
    _menu->addAction(action);

    action = new QAction(tr("Clone"), this);
    connect(action, SIGNAL(triggered()), SLOT(onCloneTriggered()));
    _menu->addAction(action);
}

void CSceneWidget::showBox(qint32 compkey)
{
    QListIterator<QGraphicsItem*> it(_scene->items());
    while(it.hasNext())
    {
        CGraphicsItem *gi = qgraphicsitem_cast<CGraphicsItem*>(it.next());
        if(gi->getCompkey() == compkey)
        {
            gi->setEditMode(true);
            gi->setImageFitMode(CGraphicsItem::ImageStretch);
            return;
        }
    }

    CGraphicsItem *item = new CGraphicsItem(compkey);
    item->setEditMode(true);
    item->setPos(_scene->items().count() * 10,_scene->items().count() * 10);
    item->setImageFitMode(CGraphicsItem::ImageStretch);
    item->setZValue(100.0);
    _scene->addItem(item);
}

qint32 CSceneWidget::getCompkey() const
{
    return _compkey;
}

void CSceneWidget::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    QListIterator<QGraphicsItem*> it(_scene->items());
    while(it.hasNext())
    {
        it.next()->update();
    }
}

void CSceneWidget::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);
}

void CSceneWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->fillRect(rect, Qt::black);
    QGraphicsView::drawBackground(painter, rect);
}

void CSceneWidget::drawForeground(QPainter *painter, const QRectF &rect)
{
    if (m_gridEnabled && m_cellWidth > 0) {
        drawGrid(painter);
    }
    QGraphicsView::drawForeground(painter, rect);
}

void CSceneWidget::mouseMoveEvent(QMouseEvent * event)
{
    if(_currentItem != 0)
    {
        if(_resizeBegin)
        {
            QPointF p = mapToScene(event->pos()) - _currentItem->pos();
            _currentItem->setSize(QSize(p.x(), p.y()));
        }
        else
            _currentItem->setPos(event->pos() - _offsetMove);
    }
    QGraphicsView::mouseMoveEvent(event);
}

void CSceneWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
    {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    if(QGraphicsItem *item = itemAt(event->pos()))
    {
        _currentItem = qgraphicsitem_cast<CGraphicsItem*>(item);

        if(_currentItem != 0 && !_currentItem->isEditMode())
        {
            _currentItem = 0;
            return;
        }

        QSize size = _currentItem->size();

        if((size.width() - 15) < event->pos().x() && (size.height() - 15) < event->pos().y())
            _resizeBegin = true;

        _offsetMove = event->pos() - item->pos();
    }
    QGraphicsView::mousePressEvent(event);
}

void CSceneWidget::mouseReleaseEvent ( QMouseEvent * event )
{
    _resizeBegin = false;
    _currentItem = NULL;
    QGraphicsView::mouseReleaseEvent(event);
}

void CSceneWidget::contextMenuEvent(QContextMenuEvent *event)
{
    bool founded = false;
    if(QGraphicsItem *item = itemAt(event->pos()))
    {
        if(!qFuzzyCompare(item->zValue(), qreal(0))) // ignore first item, first item is background
            founded = true;
    }
    if(founded)
        QGraphicsView::contextMenuEvent(event);
    else
        _menu->exec(event->globalPos());

    update();
}

void CSceneWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Plus:
            onZoomIn();
        break;
        case Qt::Key_Minus:
            onZoomOut();
        break;
        default:
            QGraphicsView::keyPressEvent(event);
    }
}

void CSceneWidget::scaleView(qreal scaleFactor)
{
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    scale(scaleFactor, scaleFactor);
}

void CSceneWidget::onZoomIn()
{
    scaleView(qreal(1.2));
}

void CSceneWidget::onZoomOut()
{
    scaleView(1 / qreal(1.2));
}


void CSceneWidget::wheelEvent(QWheelEvent *event)
{
//    qreal factor = qPow(1.2, event->delta() / 240.0);
//    scale(factor, factor);
//    event->accept();
    QGraphicsView::wheelEvent(event);
}

void CSceneWidget::setGridVisible(bool visible)
{
    m_gridEnabled = visible;
    update();
}

void CSceneWidget::setCellWidth(quint32 arg)
{
    m_cellWidth = arg;
}

void CSceneWidget::onApplyTriggered()
{
    qDebug() << "TODO: onApplyTriggered()";
    apply();
}

void CSceneWidget::onHideBoxTriggerd(bool triggerd)
{
    QListIterator<QGraphicsItem*> it(_scene->items());
    while(it.hasNext())
    {
        CGraphicsItem *gi = qgraphicsitem_cast<CGraphicsItem*>(it.next());
        gi->setEditMode(!triggerd);
    }
    if(QAction *action = qobject_cast<QAction*>(sender()))
    {
        if(triggerd)
            action->setText(tr("Show boxs"));
        else
            action->setText(tr("Hide boxs"));
    }
    update();
}

void CSceneWidget::drawGrid(QPainter *painter)
{
    qint32 m_cellWidth = 10;
    QPen p = painter->pen();
    p.setColor(Qt::gray);
    painter->setPen(p);
    QRectF r = sceneRect();
    painter->drawRect(r);
    for (int i = m_cellWidth; i < r.width(); i += m_cellWidth) {
        painter->drawLine(i, r.bottom(), i, r.top());
    }
    for (int i = m_cellWidth; i < r.height(); i += m_cellWidth) {
        painter->drawLine(r.left(), i, r.right(), i);
    }
}

void CSceneWidget::onCloneTriggered()
{
    qDebug() << "Clone";

    ClonedWidget * clone = new ClonedWidget(this->getCompkey());
    clone->setAttribute(Qt::WA_DeleteOnClose);
    clone->show();
}

QStringList CSceneWidget::apply()
{
    QStringList list;

    QListIterator<QGraphicsItem*> it(_scene->items());

    while(it.hasNext())
    {
        CGraphicsItem *gi = qgraphicsitem_cast<CGraphicsItem*>(it.next());
        gi->setEditMode(false);
        list.push_back(QString("%1x%2").arg(gi->pos().x()).arg(gi->pos().y()));
    }
    return list;
}

void CSceneWidget::start()
{
    // only one timer
    if(_timerId == 0)
        startTimer(1000 / 25);
}

void CSceneWidget::stop()
{
    killTimer(_timerId);
    qDebug() << "_timerId " << _timerId;
}

void CSceneWidget::startBox()
{
    QListIterator<QGraphicsItem*> it(_scene->items());

    while(it.hasNext())
    {
        CGraphicsItem *gi = qgraphicsitem_cast<CGraphicsItem*>(it.next());
        gi->setEditMode(true);
    }
}

void CSceneWidget::stopBox()
{
    QListIterator<QGraphicsItem*> it(_scene->items());

    while(it.hasNext())
    {
        CGraphicsItem *gi = qgraphicsitem_cast<CGraphicsItem*>(it.next());
        gi->setEditMode(false);
    }
}














