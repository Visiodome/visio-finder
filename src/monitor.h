#ifndef MONITOR_H
#define MONITOR_H

#include <QMainWindow>
#include "finder.h"

class Monitor : public QMainWindow
{
    Q_OBJECT

private:
    Finder *m_finder;

    /*! Path to the JSON config file.
     *
     *  The JSON file must contains at least 1 valid searchObject.
     *
     *  The root of the JSON file can contain a field "targetFolder" containing
     *  a string to the path of the target folder in which to create the
     *  shortcuts.
     *
     *  The root of the JSON file must contain a list named "searchObjects".
     *  Each element of this list must be an object with the following fields:
     *  @definition{targetName} the name of the file to be searched. It can be
     *      a regex.
     *  @definition{rootPaths} a list of string of root path to search for.
     *
     *  The following fields are optionnals:
     *  @definition{isRegex} A bool. Set to true if targetName is a regex.
     *      Default to false.
     *  @definition{recursionLevel} The number of recursion layer to search for
     *      the target. A value of 0 just check if a rootPath is the target,
     *      without checking inside if it is a folder, while a value of -1
     *      means searching all subfolder, without recursion limit. Default to
     *      1.
     *
     *  Example of valid JSON file:
     *  @include searchObjects.json
     *
     *  @sa config_path()
     *  @sa config_path(QString value)
     */
    QString m_config_path;

signals:
    void exit(int exit_code);

public slots:
    /*! Start headless operations.
     *
     *  This will load the configuration file set in m_config_path and create
     *  the shortcuts for each searchObject defined in it.
     */
    void startHeadless();

public:
    Monitor(QWidget *parent = nullptr);
    ~Monitor();

    /*!@{*/
    /*! Get m_config_path
     * @return m_config_path
     * @sa m_config_path
     */
    QString config_path() const {return m_config_path;};
    /*! Set m_config_path
     * @param value New m_config_path
     * @sa m_config_path
     */
    void config_path(QString value){m_config_path = value;};
    /*!@}*/

    void errorMessage(QString message, bool exit=false);

};
#endif // MONITOR_H
