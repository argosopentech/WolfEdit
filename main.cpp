#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // Create the main window
    QMainWindow mainWindow;

    // Create a central widget (e.g., QTextEdit)
    QTextEdit *textEdit = new QTextEdit(&mainWindow);

    // Create a button
    QPushButton *button = new QPushButton("Click me", &mainWindow);

    // Create a vertical layout and add the widgets
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(textEdit);
    layout->addWidget(button);

    // Set the layout for the central widget
    QWidget *centralWidget = new QWidget(&mainWindow);
    centralWidget->setLayout(layout);

    // Set the central widget for the main window
    mainWindow.setCentralWidget(centralWidget);

    // Connect button click signal to a custom slot or function
    // QObject::connect(button, SIGNAL(clicked()), yourCustomSlotOrFunction);

    // Set the main window properties
    mainWindow.setWindowTitle("WolfEdit");
    mainWindow.show();

    return a.exec();
}

