#include "cell_automaton.h"
#include "ui_mainwindow.h"

CellAutomaton::CellAutomaton(QWidget *parent) :
    QWidget(parent),
    colors(DEFAULT_COLORS_AMOUNT),
    scaling_factor(DEFAULT_SCALING_FACTOR),
    tick_timer(new QTimer(this)),
    restart_timer(new QTimer(this)),
    gen_ptr(&random_colors),
    dead_cells(0)
{
    qsrand( QTime::currentTime().msec() );

    // Connecting components
    connect(restart_timer, SIGNAL(timeout()), this, SLOT(start()));
    connect(tick_timer, SIGNAL(timeout()), this, SLOT(advance()));

    restart_timer->setSingleShot(true);
    restart_timer->start(0);
}

void CellAutomaton::resizeEvent(QResizeEvent*)
{
    tick_timer->stop();
    restart_timer->start(200);
}

void CellAutomaton::start()
{
    tick_timer->stop();

    std::cout << "(Re)Starting!\n";
    std::cout << "    Resolution:     " << width() << "x" << height() << " px.\n";
    std::cout << "    Scaling factor: " << scaling_factor << "x\n";
    std::cout << "    Colors:         " << colors.size() << std::endl;

    // Prepare map
    int map_height = height() / scaling_factor + 1;
    int map_width  = width()  / scaling_factor + 1;
    map.resize(map_height);
    buf.resize(map_height);
    for (int i = 0; i < map_height; ++i)
    {
        map[i].resize(map_width);
        buf[i].resize(map_width);
    }

    // Light the fuses!
    randomize_map();
}

void CellAutomaton::randomize_map()
{
    // Reset timer(s)
    steps = 0;
    life_timer.start();
    tick_timer->start(TICK_LENGTH_MS);

    // Prepare colors
    gen_ptr(colors);

    // Generates random color field
    for (int y = 0; y < int(map.size()); ++y)
        for (int x = 0; x < int(map[0].size()); ++x)
            map[y][x] = qrand() % colors.size();
}

void CellAutomaton::change_random_pixels(int amount)
{
    for (int i = 0; i < amount; ++i)
        map[qrand() % map.size()][qrand() % map[0].size()] = qrand() % colors.size();
}

void CellAutomaton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    if ( scaling_factor == 1 )
    {
        QPen pen;

        // Scale map to pixmap
        for (int y = 0; y < int(map.size()); ++y)
            for (int x = 0; x < int(map[0].size()); ++x)
            {
                pen.setColor(colors[map[y][x]]);
                painter.setPen(pen);

                painter.drawPoint(x, y);
            }
    }
    else
    {
        QBrush brush(Qt::SolidPattern);
        painter.setPen(Qt::NoPen);

        // Scale map to pixmap
        for (int y = 0; y < int(map.size()); ++y)
            for (int x = 0; x < int(map[0].size()); ++x)
            {
                brush.setColor(colors[map[y][x]]);
                painter.setBrush(brush);

                painter.drawRect(x * scaling_factor, y * scaling_factor,
                                      scaling_factor,     scaling_factor);
            }
    }
}

void CellAutomaton::keyPressEvent(QKeyEvent *event)
{
    std::cout << "Key pressed (" << event->key() << ", " << char(event->key()) << ")." << std::endl;

    switch ( event->key() ) {
    case '+':
        ++scaling_factor;
        start();
        break;

    case '-':
        if (scaling_factor > 1)
        {
            --scaling_factor;
            start();
        }
        break;

    case ']':
        colors.resize(colors.size() + 1);
        start();
        break;

    case '[':
        if ( colors.size() > 1 )
        {
            colors.resize(colors.size() - 1);
            start();
        }
        break;

    case 32: // spacebar
        // switch color generator
        if ( gen_ptr == &random_colors )
            gen_ptr = &melting_ice;
        else
            gen_ptr = &random_colors;
        start();
        break;

    case 'R':
        // just restart
        start();
        break;


    default:
        break;
    }
}

int CellAutomaton::pixel_next(int y, int x)
{
    static int sides[8];

    int cur = map[y][x];
    int next = (cur + 1) % colors.size();

    sides[0] = map_get(y - 1, x - 1);
    sides[1] = map_get(y - 1, x);
    sides[2] = map_get(y - 1, x + 1);
    sides[3] = map_get(y, x - 1);
    sides[4] = map_get(y, x + 1);
    sides[5] = map_get(y + 1, x - 1);
    sides[6] = map_get(y + 1, x);
    sides[7] = map_get(y + 1, x + 1);

    for (int i = 0; i < 8; ++i)
        if (sides[i] == next)
        {
            --dead_cells;
            return next;
        }

    return cur;
}

void CellAutomaton::advance()
{
    ++steps;

    // In the beginning all cells are dead
    dead_cells = map.size() * map[0].size();

    // Update map to the next step
    bool changed = false;
    static int sides[8];

    int tmp_x = map[0].size() - 1, tmp_y = map.size() - 1;

    // Update corners
    for (int y = 0; y < int(map.size()); ++y)
    {
        buf[y][0]     = pixel_next(y, 0);
        buf[y][tmp_x] = pixel_next(y, tmp_x);
    }
    for (int x = 0; x < int(map[0].size()); ++x)
    {
        buf[0][x]     = pixel_next(0, x);
        buf[tmp_y][x] = pixel_next(tmp_y, x);
    }

    // Update interior
    for (int y = 1; y < tmp_y; ++y)
        for (int x = 1; x < tmp_x; ++x)
        {
            int cur = map[y][x];

            int next = cur + 1;
            if ( next >= int(colors.size()) )
                next = 0;

            sides[0] = map[y - 1][x - 1];
            sides[1] = map[y - 1][x];
            sides[2] = map[y - 1][x + 1];
            sides[3] = map[y][x - 1];
            sides[4] = map[y][x + 1];
            sides[5] = map[y + 1][x - 1];
            sides[6] = map[y + 1][x];
            sides[7] = map[y + 1][x + 1];

            buf[y][x] = cur;
            for (int i = 0; i < 8; ++i)
                if (sides[i] == next)
                {
                    changed = true;
                    --dead_cells;
                    buf[y][x] = next; //(next == 0) ? (qrand() % colors_amount) : next;
                    break;
                }
        }

    if ( changed )
    {
        //std::cout << dead_cells << std::endl;
        if ( dead_cells <= 0 && !restart_timer->isActive() )
        {
            std::cout << "Looks like automaton entered an infinite loop. Restarting in "
                      << RESTART_TIME << " ms." << std::endl;
            restart_timer->start(RESTART_TIME);
        }
        map.swap(buf);

        double fps = steps * 1000.0 / life_timer.elapsed();
        setWindowTitle(QString::number(fps, 'f', 2) + " FPS");

        if ( life_timer.elapsed() > LIFE_LENGTH_MS )
            start();

        update();
    }
    else
    {
        std::cout.precision(5);
        std::cout << "Done, this lasted " << (life_timer.elapsed() * 1.0 / 1000) << " seconds and "
                  << steps << " steps, that is, " << (life_timer.elapsed() * 1.0 / steps) << " ms per step." << std::endl;

        tick_timer->stop();
        restart_timer->start(RESTART_TIME);
    }
}

int CellAutomaton::map_get(int y, int x)
{
    if (y < 0) y = map.size() - 1;
    if (x < 0) x = map[0].size() - 1;
    if ( y >= int(map.size()) )     y = 0;
    if ( x >= int(map[0].size()) )  x = 0;

    return map[y][x];
}


void melting_ice(std::vector<QColor> &colors)
{
    int delim = colors.size() * 3 / 4;

    double coef = 255.0 / delim;
    for (int i = 0; i < delim; ++i)
        colors[i] = QColor(coef * i, coef * i, (200.0 / delim) * i + 50);

    coef = 255.0 / (colors.size() - delim);
    for (int i = delim; i < int(colors.size()); ++i)
    {
        int j = colors.size() - i;
        colors[i] = QColor((200.0 / (colors.size() - delim)) * j + 50, coef * j, coef * j);
    }
}


void random_colors(std::vector<QColor> &colors)
{
    for (int i = 0; i < int(colors.size()); ++i)
        colors[i] = QColor(qrand() % 256, qrand() % 256, qrand() % 256);
}
