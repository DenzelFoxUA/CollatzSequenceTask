#include "UserInterface.h"

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent)
{

    //WINDOW
    const int width = 650;
    const int height = 450;
    setWindowTitle("Collatz Sequence Processor");
    resize(width, height);

    // MAIN WIDGET
    QWidget* mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);

    //MAX NUMBER
    QLabel* maxNumberLabel = new QLabel("Upper search limit:", this);

    maxNumberLimit = new QLineEdit(this);

    maxNumberLimit->setText(QString::number(GlobalConstants::CALC_DEFAULT_NUMBER));
    maxNumberLimit->setMaxLength(GlobalConstants::NUM_OF_DIGITS_MAX);

    auto* validator = new QRegularExpressionValidator(
        QRegularExpression(RegExPatterns::NUM_INPUT),maxNumberLimit);

    maxNumberLimit->setValidator(validator);

    QHBoxLayout* maxNumberLayout = new QHBoxLayout();

    maxNumberLayout->addWidget(maxNumberLabel);
    maxNumberLayout->addWidget(maxNumberLimit);
    mainLayout->addLayout(maxNumberLayout);

    // THREAD SLIDER
    unsigned int hardwareThreads = GlobalFunctions::getHardwareNumOfThreads();

    QLabel* threadLabel = new QLabel("Calculation threads num:", this);
    threadSlider = new QSlider(Qt::Horizontal, this);

    threadSlider->setMinimum(GlobalConstants::MIN_VALUE_LIMIT);
    threadSlider->setMaximum(static_cast<int>(hardwareThreads));

    threadSlider->setValue(GlobalConstants::MIN_VALUE_LIMIT);

    threadSlider->setTickPosition(QSlider::TicksBelow);
    threadSlider->setTickInterval(1);

    threadValueLabel = new QLabel(QString::number(threadSlider->value()),this);

    QHBoxLayout* threadLayout = new QHBoxLayout();

    threadLayout->addWidget(threadLabel);
    threadLayout->addWidget(threadSlider);
    threadLayout->addWidget(threadValueLabel);

    mainLayout->addLayout(threadLayout);

    // BUTTONS
    startButton = new QPushButton("Start", this);
    stopButton = new QPushButton("Stop", this);
    exitButton = new QPushButton("Exit", this);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addWidget(exitButton);

    mainLayout->addLayout(buttonLayout);

    // OUTPUT BOX
    QLabel* outputLabel = new QLabel("RESULT:", this);

    outputField = new QPlainTextEdit(this);

    outputField->setReadOnly(true);

    mainLayout->addWidget(outputLabel);
    mainLayout->addWidget(outputField);


    // CALCULATION OBSERVER
    runtimeObserver = new QTimer(this);
    runtimeObserver->setInterval(GlobalConstants::DEFAULT_TIMER_INTERVAL_MSEC);


    // CONNECTIONS
    QObject::connect(threadSlider,&QSlider::valueChanged,this,
        [this](int value)
        {
            threadValueLabel->setText(QString::number(value));
        });

    QObject::connect(startButton,&QPushButton::clicked,this,
        [this]()
        {
            startCalculation();
        });

    QObject::connect(stopButton,&QPushButton::clicked,this,
        [this]()
        {
            stopCalculation();
        });

    QObject::connect(exitButton,&QPushButton::clicked,this,
        [this]()
        {
            auto& processor = CollatzSequenceProcessor::getInstance();
            processor.stop();
            close();
        });

    QObject::connect(runtimeObserver,&QTimer::timeout,this,
        [this]()
        {
            //кожні дефолтні мілісекунди меревіряти стан
            checkCalculationState();
        });

    //Initial UI state
    setRunningState(false);
}


// Methods ------------------------------------------ 

// UI STATE
void MainWindow::setRunningState(bool running)
{
    startButton->setEnabled(!running);
    stopButton->setEnabled(running);

    threadSlider->setEnabled(!running);
    maxNumberLimit->setEnabled(!running);
}

// START
void MainWindow::startCalculation()
{
    auto& processor = CollatzSequenceProcessor::getInstance();

    bool isNumOk = false;

    //отримання ulonglong числа і перевірка за допомогою метода QString, який може міняти bool
    qulonglong value = maxNumberLimit->text().toULongLong(&isNumOk);

    if (!isNumOk || value == 0)
    {
        outputField->setPlainText("Invalid maximum number.");

        return;
    }

    std::uint64_t maxNumber = static_cast<std::uint64_t>(value);
    unsigned int threadCount = static_cast<unsigned int>(threadSlider->value());

    stoppedByUser = false;
    outputField->clear();
    outputField->appendPlainText("Calculation started...");

    setRunningState(true);
    elapsedTimer.start();
    processor.start(maxNumber,threadCount);
    runtimeObserver->start();
}

// STOP
void MainWindow::stopCalculation()
{
    auto& processor = CollatzSequenceProcessor::getInstance();

    if (!processor.isRunning())
        return;

    stoppedByUser = true;

    processor.stop();

    stopButton->setEnabled(false);

    outputField->appendPlainText("Stopping calculation...");
}

// CALCULATION STATE CHECK
void MainWindow::checkCalculationState()
{
    auto& processor = CollatzSequenceProcessor::getInstance();

    if (processor.isRunning())
        return;

    runtimeObserver->stop();


    qint64 elapsedMilliseconds = elapsedTimer.elapsed();
    setRunningState(false);

    // Якщо STOPPED BY USER
    if (stoppedByUser)
    {
        outputField->appendPlainText("Calculation stopped by user.");
        return;
    }

    // Якщо ERROR STOP
    if (processor.hasError())
    {
        outputField->appendPlainText("Calculation failed.");
        outputField->appendPlainText("Intermediate Collatz value exceeded uint64_t range.");
        return;
    }

    // Якщо SUCCESS
    CollatzSequence result = processor.getBestResult();
    outputField->clear();
    outputField->appendPlainText("Number with longest Collatz sequence: "
        + QString::number(static_cast<qulonglong>(result.num)));

    outputField->appendPlainText("Sequence length: "
        + QString::number(static_cast<qulonglong>(result.sequence_l)));

    outputField->appendPlainText("Calculation time: "
        + QString::number(elapsedMilliseconds)+ " ms");
}