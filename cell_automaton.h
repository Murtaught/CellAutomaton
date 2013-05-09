#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

#include <vector>

#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QColor>
#include <QTime>
#include <QTimer>
#include <QElapsedTimer>
#include <QResizeEvent>

#include <cmath>
#include <iostream>

class CellAutomaton : public QWidget
{
    Q_OBJECT
    static const int DEFAULT_COLORS_AMOUNT = 21;
    static const int DEFAULT_SCALING_FACTOR = 3;
    static const int TICK_LENGTH_MS = 25;
    static const int RESTART_TIME   = 800;
    static const int LIFE_LENGTH_MS = 30 * 60 * 1000; // 3 minutes

public:
    explicit CellAutomaton(QWidget *parent = 0);

public slots:
    void start();
    void advance();
    
private:
    void resizeEvent(QResizeEvent*);
    void paintEvent(QPaintEvent*);
    void keyPressEvent(QKeyEvent* event);
    void randomize_map();
    void change_random_pixels(int amount);

    typedef std::vector<std::vector<int> > map_type;

    inline int pixel_next(int y, int x);
    inline int map_get(int y, int x);
    inline int& get(map_type const& map, int y, int x);

    std::vector<QColor> colors;
    int scaling_factor;
    map_type map, buf;
    QTimer* tick_timer;
    QTimer* restart_timer;
    QElapsedTimer life_timer;
    void (*gen_ptr)(std::vector<QColor>&);
    qint64 steps;
    qint64 dead_cells;
};

void random_colors(std::vector<QColor>& colors);
void melting_ice(std::vector<QColor>& colors);

#endif // MAINWINDOW_H
