#include "mainwindow.h"

#include "./ui_mainwindow.h"

#include "cape_utils.h"

#include "config.h"

#include <QMessageBox>
#include <QDesktopServices>
#include <QSettings>
#include <QFileDialog>
#include <QTextStream>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTableWidget>
#include <QThread>
#include <QInputDialog>
#include <QCommandLineParser>
#include <QTimer>

#include "spdlog/spdlog.h"

#include "spdlog/sinks/qt_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include <filesystem>

enum LibraryColumns { Name, Type, URL, Descr };

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
	QCoreApplication::setApplicationName(PROJECT_NAME);
    QCoreApplication::setApplicationVersion(PROJECT_VER);
    ui->setupUi(this);

	auto const log_name{ "log.txt" };

	appdir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	std::filesystem::create_directory(appdir.toStdString());
	QString logdir = appdir + "/log/";
	std::filesystem::create_directory(logdir.toStdString());

	try
	{
		auto file{ std::string(logdir.toStdString() + log_name) };
		auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>( file, 1024 * 1024, 5, false);

		logger = std::make_shared<spdlog::logger>("kicadhelper", rotating);
		logger->flush_on(spdlog::level::debug);
		logger->set_level(spdlog::level::debug);
		logger->set_pattern("[%D %H:%M:%S] [%L] %v");
	}
	catch (std::exception& /*ex*/)
	{
		QMessageBox::warning(this, "Logger Failed", "Logger Failed To Start.");
	}

	//logger->info("Program started v" + std::string( PROJECT_VER));
	//logger->info(std::string("Built with Qt ") + std::string(QT_VERSION_STR));

	setWindowTitle(windowTitle() + " v" + PROJECT_VER);

	settings = std::make_unique< QSettings>(appdir + "/settings.ini", QSettings::IniFormat);

	RedrawRecentList();

	int index = settings->value("table_index", -1).toInt();
	if (-1 != index)
	{
		ui->tabWidget->setCurrentIndex(index);
	}

	QTimer::singleShot( 500, this, SLOT( ProcessCommandLine() ) );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_EEPROM_triggered()
{
	QString const EEPROM = QFileDialog::getOpenFileName(this, "Select EEPROM File", settings->value("last_project").toString(), tr("EEPROM Files (*.bin);;All Files (*.*)"));
	if (!EEPROM.isEmpty())
	{
		LoadEEPROM(EEPROM);
	}
}

void MainWindow::on_actionClose_triggered()
{
	close();
}

void MainWindow::on_actionAbout_triggered()
{
	auto hpp {helpText.right(helpText.size() - helpText.indexOf("Options:"))};
	QString text {QString("Cape EEPROM Viewer v%1\nQT v%2\n\n\nCommandline %3\n\nIcons by:\n%4")
		.arg(PROJECT_VER).arg(QT_VERSION_STR)
		.arg(hpp)
		.arg(R"(http://www.famfamfam.com/lab/icons/silk/)")};
		//http://www.famfamfam.com/lab/icons/silk/
	QMessageBox::about( this, "About Cape EEPROM Viewer", text );
}

void MainWindow::on_actionOpen_Logs_triggered()
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(appdir + "/log/"));
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
	settings->setValue("table_index", index);
	settings->sync();
}

void MainWindow::on_menuRecent_triggered()
{
	auto recentItem = qobject_cast<QAction*>(sender());
	if (recentItem && !recentItem->data().isNull())
	{
		auto const project = qvariant_cast<QString>(recentItem->data());
		LoadEEPROM(project);
	}
}

void MainWindow::on_actionClear_triggered()
{
	ui->menuRecent->clear();
	settings->remove("Recent_ProjectsList");
	//settings->setValue("Recent_ProjectsList")

	ui->menuRecent->addSeparator();
	ui->menuRecent->addAction(ui->actionClear);
}

void MainWindow::LoadEEPROM(QString const& filepath)
{
	settings->setValue("last_project", filepath);
	settings->sync();

	QFileInfo proj(filepath);

	
	AddRecentList(proj.absoluteFilePath());
}

void MainWindow::RedrawStringPortList()
{
	auto SetItem = [&](int row, int col, QString const& text)
	{
		ui->twParts->setItem(row, col, new QTableWidgetItem());
		ui->twParts->item(row, col)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		ui->twParts->item(row, col)->setText(text);
	};
	//ui->twParts->clearContents();
	//ui->twParts->setRowCount(static_cast<int>(schematic_adder->getPartList().size()));
	int row{ 0 };

	//for (auto const& line : schematic_adder->getPartList())
	//{
	//	SetItem(row, 0, line.value);
	//	SetItem(row, 1, line.footPrint);
	//	SetItem(row, 2, line.digikey);
	//	SetItem(row, 3, line.lcsc);
	//	SetItem(row, 4, line.mpn);
	//	++row;
	//}
	//ui->twParts->resizeColumnsToContents();

}

void MainWindow::AddRecentList(QString const& file)
{
	auto recentProjectList = settings->value("Recent_ProjectsList").toStringList();

	recentProjectList.push_front(file);
	recentProjectList.removeDuplicates();
	if (recentProjectList.size() > 10)
	{
		recentProjectList.pop_back();
	}
	settings->setValue("Recent_ProjectsList", recentProjectList);
	settings->sync();
	RedrawRecentList();
}

void MainWindow::RedrawRecentList()
{
	ui->menuRecent->clear();
	auto recentProjectList = settings->value("Recent_ProjectsList").toStringList();
	for (auto const& file : recentProjectList)
	{
		QFileInfo fileInfo(file);
		auto* recentpn = new QAction(this);
		recentpn->setText(fileInfo.dir().dirName() + "/" + fileInfo.fileName());
		recentpn->setData(fileInfo.absoluteFilePath());
		ui->menuRecent->addAction(recentpn);
		connect(recentpn, &QAction::triggered, this, &MainWindow::on_menuRecent_triggered);
	}

	ui->menuRecent->addSeparator();
	ui->menuRecent->addAction(ui->actionClear);
}

void MainWindow::LogMessage(QString const& message, spdlog::level::level_enum llvl, QString const& file)
{
	logger->log(llvl, message.toStdString());
	/*
	trace = SPDLOG_LEVEL_TRACE,
	debug = SPDLOG_LEVEL_DEBUG,
	info = SPDLOG_LEVEL_INFO,
	warn = SPDLOG_LEVEL_WARN,
	err = SPDLOG_LEVEL_ERROR,
	critical = SPDLOG_LEVEL_CRITICAL,
	*/

	QList< QColor > msgColors;
	msgColors.append(Qt::darkBlue);
	msgColors.append(Qt::blue);
	msgColors.append(Qt::darkGreen);
	msgColors.append("#DC582A");
	msgColors.append(Qt::darkRed);
	msgColors.append(Qt::red);

	QListWidgetItem* it = new QListWidgetItem(message);
	it->setForeground(QBrush(QColor(msgColors.at(llvl))));
	it->setData(Qt::UserRole, file);
	ui->lwLogs->addItem(it);
	ui->lwLogs->scrollToBottom();

	qApp->processEvents();
}

void MainWindow::ProcessCommandLine()
{
	QCommandLineParser parser;
    parser.setApplicationDescription("Kicad Helper");

    QCommandLineOption projectOption(QStringList() << "p" << "project",
             "Project File to Load.",
            "project");
    parser.addOption(projectOption);

    QCommandLineOption mappingOption(QStringList() << "m" << "mapping",
             "Text Replace Mapping JSON to Load.",
            "mapping");
    parser.addOption(mappingOption);

    QCommandLineOption partlistOption(QStringList() << "t" << "partlist",
             "Part Number List JSON to Load.",
            "partlist");
    parser.addOption(partlistOption);

	QCommandLineOption libraryOption(QStringList() << "l" << "library",
		"Set Library Folder.",
		"library");
	parser.addOption(libraryOption);

	QCommandLineOption bomOption(QStringList() << "b" << "bom",
		"Export to CSV BOM.",
		"bom");
	parser.addOption(bomOption);

	QCommandLineOption reportOption(QStringList() << "o" << "report",
             "Save Check Schematic FootPrints Report.",
            "report");
    parser.addOption(reportOption);

	QCommandLineOption addOption(QStringList() << "a" << "add",
            "Add Parts Numbers to Schematic.");
	parser.addOption(addOption);

	QCommandLineOption checkOption(QStringList() << "c" << "check",
            "Check Schematic Symbols and Footprints.");
    parser.addOption(checkOption);

	QCommandLineOption fixOption(QStringList() << "f" << "find",
		"Attempt to Find Symbols and Footprints.");
	parser.addOption(fixOption);

	QCommandLineOption checkSymOption(QStringList() << "s" << "checksym",
            "Check Schematic Symbols.");
    parser.addOption(checkSymOption);

	QCommandLineOption checkFpOption(QStringList() << "n" << "checkfp",
            "Check Schematic Footprints.");
    parser.addOption(checkFpOption);

	QCommandLineOption check3dOption(QStringList() << "3" << "check3d",
            "Check 3D Model Paths.");
    parser.addOption(check3dOption);

	QCommandLineOption fixSymOption(QStringList() << "y" << "findsym",
		"Attempt to Find Schematic Symbols.");
	parser.addOption(fixSymOption);

	QCommandLineOption fixFpOption(QStringList() << "i" << "findfp",
		"Attempt to Find Schematic Footprints.");
	parser.addOption(fixFpOption);

	QCommandLineOption fix3dOption(QStringList() << "d" << "find3d",
            "Fix 3D Model Paths.");
    parser.addOption(fix3dOption);

	QCommandLineOption replaceOption(QStringList() << "r" << "replace",
            "File to do text Repace in.",
			"replace");

    parser.addOption(replaceOption);

	QCommandLineOption exitOption(QStringList() << "x" << "exit",
            "Exit Software when done.");
    parser.addOption(exitOption);

	helpText = parser.helpText();

    parser.parse(QCoreApplication::arguments());

	auto lastProject{ settings->value("last_project").toString() };


}
