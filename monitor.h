#ifndef MONITOR_H
#define MONITOR_H

#include <QMainWindow>

class Finder;

class Monitor : public QMainWindow
{
    Q_OBJECT

public:
    Monitor(QWidget *parent = nullptr);
    ~Monitor();

private:
    Finder *m_finder;
};
#endif // MONITOR_H
