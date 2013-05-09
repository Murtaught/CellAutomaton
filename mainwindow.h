#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void start();
    void advance();
    void randomize_map();
    void change_random_pixels(int amount);
    
private:
    Ui::MainWindow *ui;

    void resizeEvent(QResizeEvent*);
    void paintEvent(QPaintEvent*);
    void keyPressEvent(QKeyEvent* event);

    static const int DEFAULT_COLORS_AMOUNT = 21;
    std::vector<QColor> colors;

    static const int DEFAULT_SCALING_FACTOR = 3;
    int scaling_factor;

    typedef std::vector<std::vector<int> > map_type;
    map_type map, buf;

    inline int pixel_next(int y, int x);
    inline int map_get(int y, int x);
    inline int& get(map_type const& map, int y, int x);

    static const int TICK_LENGTH_MS = 25;
    static const int RESTART_TIME   = 800;

    QTimer* tick_timer;
    QTimer* restart_timer;

    static const int LIFE_LENGTH_MS = 30 * 60 * 1000; // 3 minutes
    QElapsedTimer life_timer;

    void (*gen_ptr)(std::vector<QColor>&);

    qint64 steps;
    qint64 dead_cells;
};

void random_colors(std::vector<QColor>& colors);
void melting_ice(std::vector<QColor>& colors);

#endif // MAINWINDOW_H
