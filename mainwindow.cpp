#include "mainwindow.h"
#include <iomanip>
#include <ctime>
#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), logoLabel(nullptr), taskComboBox(nullptr), startStopButton(nullptr), timerLabel(nullptr), logTable(nullptr), sendToCSVButton(nullptr), timer(nullptr), isRunning(false)
{
    setupUI();
    initializeComboBox();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // Create central widget and main layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    // Create and setup logo
    logoLabel = new QLabel(this);
    QPixmap logo(":/resources/claLogo.png");
    logoLabel->setPixmap(logo.scaled(200, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(logoLabel);

    // Create top row layout
    QHBoxLayout *topLayout = new QHBoxLayout();

    // Create and setup task combo box
    taskComboBox = new QComboBox(this);
    connect(taskComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onTaskChanged);
    topLayout->addWidget(taskComboBox);

    // Create and setup start/stop button
    startStopButton = new QPushButton("Start", this);
    connect(startStopButton, &QPushButton::clicked, this, &MainWindow::onStartStopClicked);
    topLayout->addWidget(startStopButton);

    // Create and setup timer label
    timerLabel = new QLabel("00:00:00", this);
    timerLabel->setAlignment(Qt::AlignCenter);
    topLayout->addWidget(timerLabel);

    // Add top layout to main layout
    mainLayout->addLayout(topLayout);

    // Create and setup log table
    logTable = new QTableWidget(this);
    logTable->setColumnCount(4);
    logTable->setHorizontalHeaderLabels({"Task", "Duration", "Started", "Stopped"});
    logTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(logTable);

    // Create and setup send to CSV button
    sendToCSVButton = new QPushButton("Send to CSV", this);
    connect(sendToCSVButton, &QPushButton::clicked, this, &MainWindow::onSendToCSVClicked);
    mainLayout->addWidget(sendToCSVButton);

    // Initialize timer
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTimer);

    // Set window properties
    setWindowTitle("CLA Timekeeper");
    resize(800, 600);

    // Set application icon
    QIcon appIcon(":/resources/claLogo.png");
    setWindowIcon(appIcon);
    QApplication::setWindowIcon(appIcon);
}

void MainWindow::initializeComboBox()
{
    taskComboBox->addItems({"Donation Entry & Letters",
                            "Records Management",
                            "Minutes",
                            "File Scanning",
                            "Web Admin"});
}

void MainWindow::onTaskChanged(int index)
{
    if (!isRunning)
        return; // Only show dialog if a task is currently running

    QString newTask = taskComboBox->itemText(index);
    QString message = QString("Stop working on %1 and start working on %2?")
                          .arg(QString::fromStdString(currentTask), newTask);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Change Task",
        message,
        QMessageBox::Yes | QMessageBox::Cancel);

    if (reply == QMessageBox::Yes)
    {
        // Stop current task
        onStartStopClicked();

        // Set the new task and start it
        currentTask = newTask.toStdString();
        onStartStopClicked();
    }
    else
    {
        // Revert the combo box selection
        taskComboBox->setCurrentText(QString::fromStdString(currentTask));
    }
}

void MainWindow::onStartStopClicked()
{
    if (!isRunning)
    {
        // Start timing
        startTime = std::chrono::system_clock::now();
        currentTask = taskComboBox->currentText().toStdString();
        startStopButton->setText("Stop");
        timer->start(1000); // Update every second
        isRunning = true;
    }
    else
    {
        // Stop timing
        timer->stop();
        auto stopTime = std::chrono::system_clock::now();

        // Create and store time entry
        TimeEntry entry;
        entry.task = currentTask;
        entry.startTime = startTime;
        entry.stopTime = stopTime;
        entry.duration = std::chrono::duration_cast<std::chrono::seconds>(stopTime - startTime);
        timeEntries.push_back(entry);

        // Update log table
        int row = logTable->rowCount();
        logTable->insertRow(row);
        logTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(entry.task)));
        logTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(formatDuration(entry.duration))));
        logTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(formatTimePoint(entry.startTime))));
        logTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(formatTimePoint(entry.stopTime))));

        startStopButton->setText("Start");
        isRunning = false;
    }
}

void MainWindow::updateTimer()
{
    auto currentTime = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);

    // Convert elapsed seconds to hours:minutes:seconds
    int hours = elapsed.count() / 3600;
    int minutes = (elapsed.count() % 3600) / 60;
    int seconds = elapsed.count() % 60;

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << hours << ":"
       << std::setfill('0') << std::setw(2) << minutes << ":"
       << std::setfill('0') << std::setw(2) << seconds;
    timerLabel->setText(QString::fromStdString(ss.str()));
}

void MainWindow::onSendToCSVClicked()
{
    saveToCSV();
}

std::string MainWindow::formatTimePoint(const std::chrono::system_clock::time_point &time)
{
    auto timeT = std::chrono::system_clock::to_time_t(time);
    std::stringstream ss;
    std::tm tm;
    localtime_r(&timeT, &tm);
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string MainWindow::formatDuration(const std::chrono::seconds &duration)
{
    int hours = duration.count() / 3600;
    int minutes = (duration.count() % 3600) / 60;
    int seconds = duration.count() % 60;

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << hours << ":"
       << std::setfill('0') << std::setw(2) << minutes << ":"
       << std::setfill('0') << std::setw(2) << seconds;
    return ss.str();
}

void MainWindow::saveToCSV()
{
    std::ofstream file("time_log.csv", std::ios::app);
    if (file.is_open())
    {
        // Write header if file is empty
        file.seekp(0, std::ios::end);
        if (file.tellp() == 0)
        {
            file << "Task,Duration,Start Time,Stop Time\n";
        }
        file.seekp(0, std::ios::beg);

        // Write entries
        for (const TimeEntry &entry : timeEntries)
        {
            file << entry.task << ","
                 << formatDuration(entry.duration) << ","
                 << formatTimePoint(entry.startTime) << ","
                 << formatTimePoint(entry.stopTime) << "\n";
        }

        file.close();
        QMessageBox::information(this, "Success", "Data saved to time_log.csv");
    }
    else
    {
        QMessageBox::warning(this, "Error", "Could not open file for writing");
    }
}
