#include "monitor.h"
#include "finder.h"
#include <QTextStream>
#include <QErrorMessage>

Monitor::Monitor(QWidget *parent)
    : QMainWindow(parent)
    , m_finder(new Finder())
{
}

Monitor::~Monitor()
{
    delete m_finder;
}

void Monitor::startHeadless(){
    if(!m_finder->loadJson(m_config_path)){
        QTextStream{stderr} << "Config file not found or ill-formed.\n";
        emit exit(1);
        close();
    }

    m_finder->search_targets();
    m_finder->create_shortcuts();

    emit exit(0);
    close();
}

void Monitor::errorMessage(QString message, bool exit){
    QErrorMessage err_win;
    err_win.showMessage(message);
    err_win.exec();
    if(exit){
        emit this->exit(1);
        close();
    }
}
