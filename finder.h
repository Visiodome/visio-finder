#ifndef FINDER_H
#define FINDER_H

#include <QObject>

class Finder : public QObject
{
    Q_OBJECT

public:
    Finder(QObject *parent = nullptr);
    ~Finder();
};
#endif // FINDER_H
