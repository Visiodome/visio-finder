#include "monitor.h"

#include <QApplication>
#include <QTimer>
#include <QCommandLineParser>
#include <QTextStream>
#include <QErrorMessage>
#include <QMessageBox>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Visiofinder");
    QApplication::setApplicationDisplayName("Visiofinder");
    QApplication::setApplicationVersion("1.0");

    // Main window
    Monitor monitor;
    QObject::connect(&monitor, &Monitor::exit, &app, &QApplication::exit);

    // The application can be launched on the command line with some options.
    QCommandLineParser parser;
    parser.setApplicationDescription(
                "Find and create shortcuts for researched file/directory.");
    parser.addHelpOption();

    parser.addOptions({
        // Launch the application without GUI
        {"headless",
            QCoreApplication::translate(
                "main", "Start application without gui.")
        // Path to the JSON config file
        },{{"c", "config"},
            QCoreApplication::translate(
                "main", "Path to the JSON config file."),
            QCoreApplication::translate("main", "ConfigPath")
        }
    });

    parser.process(*qApp);

    QString config_path = parser.value("config");
    if(!config_path.isEmpty())
        monitor.config_path(config_path);

    if(parser.isSet("headless")){
        // Check if a config file has been given
        if(config_path.isEmpty()){
            // Show an errorm essage and exit the programm
            QMessageBox error(QMessageBox::NoIcon, "Error", "", QMessageBox::NoButton, nullptr, Qt::Dialog );
            error.setText("The --config option must be set for headless operations.");
            error.show();
            error.exec();
            return 1;
        }

        // Launch headless operations as soon as the application execute
        QTimer::singleShot(0, &monitor, SLOT(startHeadless()));
    }
    else{
        // Show the monitor GUI
        monitor.show();
    }

    return app.exec();
}
