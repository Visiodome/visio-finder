#include "monitor.h"
#include "finder.h"

Monitor::Monitor(QWidget *parent)
    : QMainWindow(parent)
{
    m_finder = new Finder();
}

Monitor::~Monitor()
{
    delete m_finder;
}

