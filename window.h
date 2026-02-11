#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QQueue>
#include <QSet>

QT_BEGIN_NAMESPACE
namespace Ui {
class window;
}
QT_END_NAMESPACE

class window : public QMainWindow
{
    Q_OBJECT
protected:
    void resizeEvent(QResizeEvent *event) override;

public:
    window(QWidget *parent = nullptr);
    ~window();

private:
    Ui::window *ui;
    enum Mode{ADAUGA_NOD, STERGE_NOD, ADAUGA_MUCHIE,SELECTEAZA_START, MISCA_NOD, NIMIC};
    Mode modcurent = ADAUGA_NOD;
    Mode modPrecedent;
    int primulNodPentruMuchie = 1;
    struct Nod{
        int id;
        QGraphicsEllipseItem* cerc;
        QGraphicsTextItem* text;
        QList<int> vecini;
    };
    struct Muchie {
        QGraphicsLineItem* linie;
        int idSursa;
        int idDestinatie;
    };

    QVector<Nod> noduri;
    QList<Muchie> muchii;
    bool eventFilter(QObject *obj, QEvent *event)override;
    QGraphicsScene *scena;
    bool reindexare = false;
    Nod* nodSelectat = nullptr;
    Nod* nodStart = nullptr;
    void actualizareMuchii();
    void reseteazaCulorile();
    void executaBFS();
    void executaDFS();
    void DFS(int idCurent, QSet<int> &vizitate);
    void aplicaTema(bool dark);
};
#endif // WINDOW_H
