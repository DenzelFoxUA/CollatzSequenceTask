#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <QPlainTextEdit>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>
//замість <QSpinBox> ввів QLine щоб отримувати стрінгу з 
//лімітом 20 символів і RegEx щоб тільки цифри можна було вводити
//бо, я знаю що ми маємо обмеження 10 000 000 але ж і працюємо з uint64
#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

//#include <thread>
//#include <cstdint>

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
    QLineEdit* maxNumberLimit = nullptr;

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