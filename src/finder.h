#ifndef FINDER_H
#define FINDER_H

#include <QObject>
#include <QString>
#include <QRegularExpression>
#include <QFileSystemModel>

/*! The finder class.
 */
class Finder : public QObject
{
    Q_OBJECT

    enum ShortcutType{SYMLINK, SCRIPT};

    struct RootPath{
        QString root_path; /*!< Path of the root folder to search in */
        int recursion_level; /*!< Number of layers of subfolder to search */
        /*! List of regexs for folder matching.
         *
         *  This list should either be empty, or have a size of `recursion_level`
         */
        QList<QRegularExpression> regexs;
    };

    /*! Container of an object to search for.
     */
    struct SearchObject{
        QList<RootPath> root_paths; /*!< List of root paths to search */
        QRegularExpression target_name; /*!< Regex of the target name */
        QString link_name; /*!< Name of the shortcut */
        QString path; /*!< Path of target when found */
        int recursion_level; /*!< Recursion level for the search */
        ShortcutType type;
        QString custom_command;
    };

private:
    QList<SearchObject> m_searches; /*!< List of objects to search for */
    QString m_target_folder; /*!< Target folder to create the shortcuts */



signals:
    void exit();

public:
    Finder(QObject *parent = nullptr);
    ~Finder();

    /*! Load the finder settings from a json file.
     *
     *  The JSON file to load must contains at least 1 valid searchObject to
     *  consider the loading as a success.
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
     *  @param path The path to the JSON file to load
     *
     *  @return `true` if the settings have been succesfully loaded, `false` if
     *      not.
     */
    bool loadJson(QString path);

    /*! Search every target
     *
     *  For every target set by `loadJson`, try to search the target in
     *  the root paths defined associated.
     */
    void search_targets();

    /*! Create shortcuts for targtes found.
     *
     *  This create a shortcut for each target found in the target folder set
     *  by `loadJson`. This should be called after `search_targets`.
     */
    void create_shortcuts();

    /*! Search for target in root path
     *
     *  Search for the target in the folder `root_path`. This method support
     *  regex for the target name.
     *
     *  @param target_name The regex of the target name.
     *  @param root_path The path in which to search for the target.
     *  @param recursion_level Number of recursion layer to search for. 0 means
     *      only checking if `root_path` is valid and match `target_name`. Set
     *      to -1 to check recursively trough all subfolders. Default to 1.
     *
     *  @return The path to the target. Return an emptz QString if not found.
     */
    static QString search_target(
            QRegularExpression target_name,
            QFileInfo root_path,
            int = 1,
            QList<QRegularExpression> regexs = QList<QRegularExpression>());

    /*! Replace the markers
     *
     *  Replace the markers found in the string path.
     *
     *  Available markers:
     *  @definition{<AppData>} The path to the lAppData folder
     *  @definition{<AppDataLocal>} The path to the local AppData folder of the user
     *  @definition{<AppDataStartDir>} The path to the microsoft start menu programs folder
     *  @definition{<ProgramFiles>} The path to the Program Files folder
     *  @definition{<ProgramFilesX86>} The path to the Program Files x86 folder
     *  @definition{<Home>} The path to the user home folder
     *
     *  @param path The string containing the markers to replace
     */
    static void replaceConstants(QString &path);
};
#endif // FINDER_H
