#include "window.h"
#include "./ui_window.h"
#include <QRadioButton>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QGraphicsTextItem>
#include <QGraphicsItem>
#include <QRectF>
#include <QLineF>
#include <QString>
#include <QThread>
#include <algorithm>
#include <QPainter>
#include <QQueue>


window::window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::window)
{
    ui->setupUi(this);
    modcurent = Mode::NIMIC;
    ui->casetaGraf->setRenderHint(QPainter::Antialiasing);
    ui->casetaGraf->setRenderHint(QPainter::TextAntialiasing);

    ui->checkDarkMode->setText("Dark Mode");
    ui->checkDarkMode->setStyleSheet(
        "QCheckBox::indicator { width: 45px; height: 22px; }"
        "QCheckBox::indicator:unchecked { background-color: #ccc; border-radius: 11px; }"
        "QCheckBox::indicator:checked { background-color: #4CAF50; border-radius: 11px; }"
        "QCheckBox::indicator:unchecked:hover { background-color: #bbb; }"
        );

    connect(ui->checkDarkMode, &QCheckBox::toggled, this, &window::aplicaTema);

    this->setWindowTitle("Graph Editor by Birceanu Radu & Mario Stancu");
    ui->casetaGraf->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->casetaGraf->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->casetaGraf->setMouseTracking(true);
    ui->casetaGraf->viewport()->setMouseTracking(true);

    connect(ui->adaugaNod, &QRadioButton::toggled, this, [=](bool checked){ if(checked) modcurent = ADAUGA_NOD; });
    connect(ui->stergeNod, &QRadioButton::toggled, this, [=](bool checked){ if(checked) modcurent = STERGE_NOD; });
    connect(ui->reindexareStergere, &QCheckBox::toggled, this, [=](bool checked){ if(checked) reindexare = checked; });
    connect(ui->adaugaMuchie, &QRadioButton::toggled, this, [=](bool checked){ if(checked) modcurent = ADAUGA_MUCHIE; });
    connect(ui->selecteazaStart, &QRadioButton::toggled, this, [=](bool checked){ if(checked) modcurent = SELECTEAZA_START; });
    connect(ui->mutaNod, &QRadioButton::toggled, this, [=](bool checked){ if(checked) modcurent = MISCA_NOD; });

    connect(ui->selecteazaStart, &QPushButton::clicked, this, [=](){
        if(modcurent != SELECTEAZA_START) modPrecedent = modcurent;
        modcurent = SELECTEAZA_START;
        ui->startLabel->setText("Nod Start: ");
        ui->adaugaNod->setAutoExclusive(false);
        ui->adaugaNod->setChecked(false);
        ui->stergeNod->setChecked(false);
        ui->adaugaMuchie->setChecked(false);
        ui->mutaNod->setChecked(false);
        ui->adaugaNod->setAutoExclusive(true);
    });

    connect(ui->BFS, &QPushButton::clicked, this, &window::executaBFS);
    connect(ui->DFS, &QPushButton::clicked, this, &window::executaDFS);
    connect(ui->stergeGraf, &QPushButton::clicked, this, [=]() {
        scena->clear(); noduri.clear(); muchii.clear();
        nodStart = nullptr; nodSelectat = nullptr; primulNodPentruMuchie = 1;
        ui->startLabel->setText("Nod Start: "); scena->update();
    });

    scena = new QGraphicsScene(this);
    scena->setSceneRect(0, 0, ui->casetaGraf->width(), ui->casetaGraf->height());
    ui->casetaGraf->setScene(scena);
    ui->casetaGraf->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->casetaGraf->viewport()->installEventFilter(this);

    ui->checkDarkMode->blockSignals(true);
    ui->checkDarkMode->setChecked(false);
    ui->checkDarkMode->blockSignals(false);
    aplicaTema(false);
}

void window::actualizareMuchii() {
    for(auto &m : muchii) {
        Nod *nSursa = nullptr, *nDest = nullptr;
        for(auto &n : noduri) {
            if(n.id == m.idSursa) nSursa = &n;
            if(n.id == m.idDestinatie) nDest = &n;
        }
        if(nDest && nSursa) {
            QPointF p1 = nSursa->cerc->scenePos();
            QPointF p2 = nDest->cerc->scenePos();
            QLineF linieNoua(p1,p2);
            qreal r = 20;
            if(linieNoua.length() > r) {
                m.linie->setLine(QLineF(linieNoua.pointAt(r / linieNoua.length()), linieNoua.pointAt(1 - r / linieNoua.length())));
            }
        }
    }
}

bool window::eventFilter(QObject *obj, QEvent *event) {
    if(obj == ui->casetaGraf->viewport() && event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPointF pos = ui->casetaGraf->mapToScene(mouseEvent->pos());
        if(modcurent == MISCA_NOD) actualizareMuchii();

        Qt::CursorShape targetShape = Qt::ArrowCursor;
        if(modcurent == ADAUGA_MUCHIE) {
            QList<QGraphicsItem*> items = scena->items(QRectF(pos.x()-2, pos.y()-2, 4, 4));
            for(auto i : items) if(i->type() == QGraphicsLineItem::Type) { targetShape = Qt::PointingHandCursor; break; }
        }
        if(ui->casetaGraf->viewport()->cursor().shape() != targetShape) ui->casetaGraf->viewport()->setCursor(targetShape);
    }

    if(obj == ui->casetaGraf->viewport() && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPointF pos = ui->casetaGraf->mapToScene(mouseEvent->pos());
        bool dark = ui->checkDarkMode->isChecked();

        if(modcurent == MISCA_NOD) return false;

        if(modcurent == ADAUGA_NOD) {
            QGraphicsEllipseItem* cerc = new QGraphicsEllipseItem();
            cerc->setFlag(QGraphicsItem::ItemIsMovable);
            cerc->setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
            cerc->setRect(-20, -20, 40, 40);

            bool esteDark = ui->checkDarkMode->isChecked();
            cerc->setPen(QPen(esteDark ? Qt::white : Qt::black, 3));
            cerc->setBrush(QBrush(Qt::white));

            cerc->setPos(pos.x(), pos.y());
            scena->addItem(cerc);

            QGraphicsTextItem* text = new QGraphicsTextItem(QString::number(primulNodPentruMuchie), cerc);
            text->setDefaultTextColor(Qt::black);
            QFont font = text->font(); font.setPointSize(12); font.setBold(true); text->setFont(font);
            text->setPos(-text->boundingRect().width() / 2, -text->boundingRect().height() / 2);

            Nod n; n.id = primulNodPentruMuchie++; n.cerc = cerc; n.text = text;
            noduri.push_back(n);
        }

        if(modcurent == STERGE_NOD) {
            for(int i = 0; i < noduri.size(); i++) {
                if(noduri[i].cerc->contains(noduri[i].cerc->mapFromScene(pos))) {
                    QList<QGraphicsItem*> items = scena->items(noduri[i].cerc->sceneBoundingRect());
                    for(auto item : items) if(item->type() == QGraphicsLineItem::Type) {
                            for (int j = 0; j < muchii.size(); ++j) if (muchii[j].linie == item) { muchii.removeAt(j); break; }
                            scena->removeItem(item); delete item;
                        }
                    if(&noduri[i] == nodStart) { nodStart = nullptr; ui->startLabel->setText("Nod Start: "); }
                    scena->removeItem(noduri[i].cerc); delete noduri[i].cerc;
                    for(auto &n : noduri) n.vecini.removeAll(noduri[i].id);
                    noduri.remove(i);
                    if(reindexare) {
                        int idNou = 1;
                        for(auto &n : noduri) { n.id = idNou; n.text->setPlainText(QString::number(idNou++)); }
                        primulNodPentruMuchie = idNou;
                    }
                    break;
                }
            }
        }

        if(modcurent == ADAUGA_MUCHIE) {
            QList<QGraphicsItem*> itemsAt = scena->items(QRectF(pos - QPointF(5,5), QSizeF(10,10)));


            for(auto item : itemsAt) if(item->type() == QGraphicsLineItem::Type) {
                    for (int j = 0; j < muchii.size(); ++j) if (muchii[j].linie == item) {
                            // Stergem vecinii din listele nodurilor
                            int id1 = muchii[j].idSursa;
                            int id2 = muchii[j].idDestinatie;
                            for(auto &n : noduri) {
                                if(n.id == id1) n.vecini.removeAll(id2);
                                if(n.id == id2) n.vecini.removeAll(id1);
                            }

                            muchii.removeAt(j);
                            break;
                        }
                    scena->removeItem(item); delete item;
                    if(nodSelectat) reseteazaCulorile();
                    nodSelectat = nullptr; return true;
                }


            QGraphicsItem* cercGasit = nullptr;
            for(auto item : itemsAt) {
                if(item->type() == QGraphicsEllipseItem::Type) { cercGasit = item; break; }
                else if(item->parentItem() && item->parentItem()->type() == QGraphicsEllipseItem::Type) { cercGasit = item->parentItem(); break; }
            }
            if(!cercGasit) return true;
            Nod* nodCurent = nullptr;
            for(auto &n : noduri) if(n.cerc == cercGasit) { nodCurent = &n; break; }
            if(!nodCurent) return true;

            if(!nodSelectat) { nodSelectat = nodCurent; nodSelectat->cerc->setBrush(QBrush(Qt::yellow)); }
            else {
                if (nodCurent != nodSelectat) {
                    QLineF linie(nodSelectat->cerc->pos(), nodCurent->cerc->pos());
                    if(linie.length() > 0) {
                        QGraphicsLineItem* muchieItem = new QGraphicsLineItem(QLineF(linie.pointAt(20/linie.length()), linie.pointAt(1 - 20 / linie.length())));
                        muchieItem->setPen(QPen(dark ? Qt::white : Qt::black, 3));
                        scena->addItem(muchieItem);
                        Muchie m; m.linie = muchieItem; m.idSursa = nodSelectat->id; m.idDestinatie = nodCurent->id;
                        muchii.append(m);

                        // Verificare dubluri la adaugare
                        if(!nodSelectat->vecini.contains(nodCurent->id)) nodSelectat->vecini.append(nodCurent->id);
                        if(!nodCurent->vecini.contains(nodSelectat->id)) nodCurent->vecini.append(nodSelectat->id);
                    }
                }
                reseteazaCulorile(); nodSelectat = nullptr;
            }
        }

        if(modcurent == SELECTEAZA_START) {
            QList<QGraphicsItem*> itemsAt = scena->items(QRectF(pos - QPointF(5,5), QSizeF(10,10)));
            QGraphicsEllipseItem* cercLovit = nullptr;
            for(auto item : itemsAt) {
                if(item->type() == QGraphicsEllipseItem::Type) { cercLovit = static_cast<QGraphicsEllipseItem*>(item); break; }
                else if(item->parentItem() && item->parentItem()->type() == QGraphicsEllipseItem::Type) { cercLovit = static_cast<QGraphicsEllipseItem*>(item->parentItem()); break; }
            }
            if(cercLovit) {
                if(nodStart) nodStart->cerc->setBrush(QBrush(Qt::white));
                for(auto &n : noduri) if(n.cerc == cercLovit) {
                        nodStart = &n; nodStart->cerc->setBrush(QBrush(QColor(127, 194, 173)));
                        ui->startLabel->setText("Nod Start: " + QString::number(nodStart->id)); break;
                    }
                modcurent = modPrecedent;
                if(modcurent == ADAUGA_NOD) ui->adaugaNod->setChecked(true);
                else if(modcurent == STERGE_NOD) ui->stergeNod->setChecked(true);
                else if(modcurent == ADAUGA_MUCHIE) ui->adaugaMuchie->setChecked(true);
                else if(modcurent == MISCA_NOD) ui->mutaNod->setChecked(true);
            }
        }
        return true;
    }
    return false;
}

void window::reseteazaCulorile() {
    bool dark = ui->checkDarkMode->isChecked();
    for(auto &n : noduri) {
        n.cerc->setBrush(QBrush(Qt::white));
        n.cerc->setPen(QPen(dark ? Qt::white : Qt::black, 3));
    }
    if(nodStart) nodStart->cerc->setBrush(QColor(127, 194, 173));
}

void window::executaBFS() {
    if(!nodStart) return;
    reseteazaCulorile();
    QQueue<int> coada; QSet<int> vizitate;

    coada.enqueue(nodStart->id);
    vizitate.insert(nodStart->id);

    while(!coada.isEmpty()) {
        int idCurent = coada.dequeue();

        for(auto &n : noduri) if(n.id == idCurent) {
                n.cerc->setBrush(QBrush(QColor(135, 206, 235)));
                QCoreApplication::processEvents();
                QThread::msleep(500);

                QList<int> veciniSortati = n.vecini;
                std::sort(veciniSortati.begin(),veciniSortati.end());

                for(int idVecin : veciniSortati) if(!vizitate.contains(idVecin)) {
                        vizitate.insert(idVecin);
                        coada.enqueue(idVecin);
                    }
                break;
            }
    }

    QThread::msleep(1000);
    reseteazaCulorile();
}

void window::DFS(int idCurent, QSet<int> &vizitate) {
    vizitate.insert(idCurent);
    for(auto &n : noduri) if(n.id == idCurent) {
            n.cerc->setBrush(QBrush(QColor(135, 206, 235)));
            QCoreApplication::processEvents(); QThread::msleep(500);
            QList<int> veciniSortati = n.vecini; std::sort(veciniSortati.begin(),veciniSortati.end());
            for(int idVecin : veciniSortati) if(!vizitate.contains(idVecin)) DFS(idVecin,vizitate);
            break;
        }
}

void window::executaDFS() {
    if(!nodStart) return;
    reseteazaCulorile();
    QSet<int> vizitate; DFS(nodStart->id,vizitate);

    QThread::msleep(1000);
    reseteazaCulorile();
}

void window::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    if(scena) scena->setSceneRect(0, 0, ui->casetaGraf->width(), ui->casetaGraf->height());
}

void window::aplicaTema(bool dark) {
    if (!scena || !ui) return;

    QString indicatorStyle =
        "QRadioButton::indicator, QCheckBox::indicator {"
        "   width: 14px;"
        "   height: 14px;"
        "   border-radius: 7px;"
        "   border: 1px solid #888;"
        "   background-color: white;"
        "}"
        "QRadioButton::indicator:checked, QCheckBox::indicator:checked {"
        "   background-color: #0078D7;"
        "   border: 1px solid #005A9E;"
        "}";

    if (dark) {
        this->setStyleSheet(
            "QMainWindow { background-color: #121212; }"
            "QWidget#centralwidget { background-color: #121212; }"
            "QGraphicsView { background-color: #1e1e1e; border: 1px solid #333; }"
            "QLabel, QRadioButton, QCheckBox { color: white; }"
            "QPushButton { background-color: #333; color: white; border: 1px solid #555; border-radius: 4px; padding: 5px; }"
            "QPushButton:hover { background-color: #0078D7; color: white; }"
            + indicatorStyle
            );
        scena->setBackgroundBrush(QBrush(QColor(30, 30, 30)));
        for(auto &m : muchii) m.linie->setPen(QPen(Qt::white, 3));
    } else {
        this->setStyleSheet(
            "QMainWindow { background-color: #f2f2f2; }"
            "QWidget#centralwidget { background-color: #f2f2f2; }"
            "QGraphicsView { background-color: #FFFFFF; border: 1px solid #CCCCCC; }"
            "QLabel, QRadioButton, QCheckBox { color: black; }"
            "QPushButton { background-color: #E1E1E1; color: black; border: 1px solid #BBB; border-radius: 4px; padding: 5px; }"
            "QPushButton:hover { background-color: #0078D7; color: white; }"
            + indicatorStyle
            );
        scena->setBackgroundBrush(QBrush(Qt::white));
        for(auto &m : muchii) m.linie->setPen(QPen(Qt::black, 3));
    }
    reseteazaCulorile();
    scena->update();
}

window::~window() { delete ui; }
