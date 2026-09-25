#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QPlainTextEdit>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>

#include "CollatzSequenceProcessor.h"

class MainWindow : public QMainWindow
{
private:

    //[BUTTONS]
    QPushButton* startButton = nullptr;
    QPushButton* stopButton = nullptr;
    QPushButton* exitButton = nullptr;

    //[THREADS]
    QSlider* threadSlider = nullptr;
    QLabel* threadValueLabel = nullptr;

    //[MAX NUMBER]
    QSpinBox* maxNumberSpinBox = nullptr;

    //[OUTPUT]
    QPlainTextEdit* outputField = nullptr;

    //[CALCULATION WATCHER]
    QTimer* calculationWatcher = nullptr;

    //[TIME]
    QElapsedTimer elapsedTimer;

    //[STATE]
    bool stoppedByUser = false;

    //Methods
    void setRunningState(bool running);

    void startCalculation();
    void stopCalculation();
    void checkCalculationState();

public:

    explicit MainWindow(QWidget* parent = nullptr);

    ~MainWindow() override = default;
};

#endif