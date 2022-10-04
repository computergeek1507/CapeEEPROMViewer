#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


#include "spdlog/spdlog.h"
#include "spdlog/common.h"

#include <memory>
#include <filesystem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }

class QListWidgetItem;
class QListWidget;
class QTableWidget;
class QSettings;
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public Q_SLOTS:

    void on_actionOpen_EEPROM_triggered();
    void on_actionClose_triggered();

    void on_actionAbout_triggered();
    void on_actionOpen_Logs_triggered();

    void on_tabWidget_currentChanged(int row);

    void on_menuRecent_triggered();
    void on_actionClear_triggered();

    void RedrawStringPortList();

    void LogMessage(QString const& message , spdlog::level::level_enum llvl = spdlog::level::level_enum::debug, QString const& file = QString());

    void ProcessCommandLine();

private:
    Ui::MainWindow *ui;

    std::shared_ptr<spdlog::logger> logger{ nullptr };
    std::unique_ptr<QSettings> settings{ nullptr };
    QString appdir;
    QString helpText;


    void AddRecentList(QString const& project);
    void RedrawRecentList();

    void LoadEEPROM(QString const& filepath);

};
#endif // MAINWINDOW_H
