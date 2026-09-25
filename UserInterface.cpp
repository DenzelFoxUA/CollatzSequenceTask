#include "UserInterface.h"



MainWindow::MainWindow(QWidget* parent): QMainWindow(parent)
{
    const int width = 650;
    const int height = 420;

    setWindowTitle("Collatz Sequence Processor");
    resize(width, height);

    //=========================================
    // CENTRAL WIDGET
    //=========================================

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    //=========================================
    // MAX NUMBER
    //=========================================

    QLabel* maxNumberLabel = new QLabel("Upper search limit:", this);

    maxNumberLimit = new QLineEdit(this);

    maxNumberLimit->setText("1000");
    maxNumberLimit->setMaxLength(20);

    auto* validator = new QRegularExpressionValidator(
        QRegularExpression("[0-9]{1,20}"),
        maxNumberLimit
    );

    maxNumberLimit->setValidator(validator);

    QHBoxLayout* maxNumberLayout = new QHBoxLayout();

    maxNumberLayout->addWidget(maxNumberLabel);
    maxNumberLayout->addWidget(maxNumberLimit);

    mainLayout->addLayout(maxNumberLayout);

    //=========================================
    // THREAD SLIDER
    //=========================================

    unsigned int hardwareThreads = GlobalFunctions::getHardwareNumOfThreads();

    QLabel* threadLabel = new QLabel("Calculation threads:", this);
    threadSlider = new QSlider(Qt::Horizontal, this);

    threadSlider->setMinimum(1);
    threadSlider->setMaximum(static_cast<int>(hardwareThreads));

    threadSlider->setValue(1);

    threadSlider->setTickPosition(QSlider::TicksBelow);

    threadValueLabel = new QLabel(QString::number(threadSlider->value()),this);


    QHBoxLayout* threadLayout = new QHBoxLayout();

    threadLayout->addWidget(threadLabel);
    threadLayout->addWidget(threadSlider);
    threadLayout->addWidget(threadValueLabel);
    mainLayout->addLayout(threadLayout);

    //=========================================
    // BUTTONS
    //=========================================

    startButton = new QPushButton("Start", this);

    stopButton = new QPushButton("Stop", this);

    exitButton = new QPushButton("Exit", this);


    QHBoxLayout* buttonLayout = new QHBoxLayout();

    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addWidget(exitButton);

    mainLayout->addLayout(buttonLayout);


    //=========================================
    // OUTPUT
    //=========================================

    QLabel* outputLabel = new QLabel("Output:", this);

    outputField = new QPlainTextEdit(this);

    outputField->setReadOnly(true);

    mainLayout->addWidget(outputLabel);
    mainLayout->addWidget(outputField);


    //=========================================
    // CALCULATION WATCHER
    //=========================================

    calculationWatcher =
        new QTimer(this);

    calculationWatcher->setInterval(50);


    //=========================================
    // CONNECTIONS
    //=========================================

    QObject::connect(
        threadSlider,
        &QSlider::valueChanged,
        this,
        [this](int value)
        {
            threadValueLabel->setText(
                QString::number(value)
            );
        }
    );


    QObject::connect(
        startButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            startCalculation();
        }
    );


    QObject::connect(
        stopButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            stopCalculation();
        }
    );


    QObject::connect(
        exitButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            auto& processor =
                CollatzSequenceProcessor::getInstance();

            processor.stop();

            close();
        }
    );


    QObject::connect(
        calculationWatcher,
        &QTimer::timeout,
        this,
        [this]()
        {
            checkCalculationState();
        }
    );


    //Initial UI state
    setRunningState(false);
}


//==================================================
// UI STATE
//==================================================

void MainWindow::setRunningState(bool running)
{
    startButton->setEnabled(!running);
    stopButton->setEnabled(running);

    threadSlider->setEnabled(!running);
    maxNumberLimit->setEnabled(!running);
}


//==================================================
// START
//==================================================

void MainWindow::startCalculation()
{
    auto& processor =
        CollatzSequenceProcessor::getInstance();


    bool isNumOk = false;

    qulonglong value =
        maxNumberLimit->text().toULongLong(&isNumOk);

    if (!isNumOk || value == 0)
    {
        outputField->setPlainText(
            "Invalid maximum number."
        );

        return;
    }

    std::uint64_t maxNumber = static_cast<std::uint64_t>(value);
    unsigned int threadCount = static_cast<unsigned int>(threadSlider->value());

    stoppedByUser = false;

    outputField->clear();

    outputField->appendPlainText(
        "Calculation started..."
    );


    setRunningState(true);


    //Start timer before calculation
    elapsedTimer.start();


    processor.start(
        maxNumber,
        threadCount
    );


    //Start checking processor state
    calculationWatcher->start();
}


//==================================================
// STOP
//==================================================

void MainWindow::stopCalculation()
{
    auto& processor =
        CollatzSequenceProcessor::getInstance();


    if (!processor.isRunning())
        return;


    stoppedByUser = true;


    processor.stop();


    //Do not allow Stop to be pressed repeatedly
    stopButton->setEnabled(false);


    outputField->appendPlainText(
        "Stopping calculation..."
    );
}


//==================================================
// CALCULATION STATE CHECK
//==================================================

void MainWindow::checkCalculationState()
{
    auto& processor =
        CollatzSequenceProcessor::getInstance();


    //Calculation is still running
    if (processor.isRunning())
        return;


    //Calculation finished
    calculationWatcher->stop();


    qint64 elapsedMilliseconds =
        elapsedTimer.elapsed();


    setRunningState(false);


    //=========================================
    // STOPPED BY USER
    //=========================================

    if (stoppedByUser)
    {
        outputField->appendPlainText(
            "Calculation stopped by user."
        );

        return;
    }


    //=========================================
    // ERROR
    //=========================================

    if (processor.hasError())
    {
        outputField->appendPlainText(
            "Calculation failed."
        );

        outputField->appendPlainText(
            "Intermediate Collatz value "
            "exceeded uint64_t range."
        );

        return;
    }


    //=========================================
    // SUCCESS
    //=========================================

    CollatzSequence result =
        processor.getBestResult();


    outputField->clear();


    outputField->appendPlainText(
        "Number with longest Collatz sequence: "
        + QString::number(
            static_cast<qulonglong>(result.num)
        )
    );


    outputField->appendPlainText(
        "Sequence length: "
        + QString::number(
            static_cast<qulonglong>(result.sequence_l)
        )
    );


    outputField->appendPlainText(
        "Calculation time: "
        + QString::number(elapsedMilliseconds)
        + " ms"
    );
}