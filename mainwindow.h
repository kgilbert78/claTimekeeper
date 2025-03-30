#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <chrono>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

struct TimeEntry
{
    std::string task;
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point stopTime;
    std::chrono::seconds duration;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartStopClicked();
    void updateTimer();
    void onSendToCSVClicked();
    void onTaskChanged(int index);

private:
    QLabel *logoLabel;
    QComboBox *taskComboBox;
    QPushButton *startStopButton;
    QLabel *timerLabel;
    QTableWidget *logTable;
    QPushButton *sendToCSVButton;
    QTimer *timer;
    bool isRunning;
    std::chrono::system_clock::time_point startTime;
    std::string currentTask;
    std::vector<TimeEntry> timeEntries;

    void setupUI();
    void initializeComboBox();
    void saveToCSV();
    std::string formatTimePoint(const std::chrono::system_clock::time_point &time);
    std::string formatDuration(const std::chrono::seconds &duration);
};
#endif // MAINWINDOW_H
