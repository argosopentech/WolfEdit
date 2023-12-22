#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QTextStream>
#include <QString>
#include <QTabWidget>

class TextEditor : public QMainWindow {
    Q_OBJECT

public:
    TextEditor(QWidget *parent = nullptr) : QMainWindow(parent) {
        tabWidget = new QTabWidget(this);
        setCentralWidget(tabWidget);

        createMenu();

        setWindowTitle("WolfEdit");
        resize(800, 600);
    }

    QTabWidget* getTabWidget() const {
        return tabWidget;
    }

private slots:
    void openFile() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Text Files (*.txt);;All Files (*)"));
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextEdit *textEdit = new QTextEdit(this);
                textEdit->setPlainText(QString::fromUtf8(file.readAll()));
                file.close();

                int tabIndex = tabWidget->addTab(textEdit, QFileInfo(fileName).fileName());
                tabWidget->setCurrentIndex(tabIndex);
            }
        }
    }

    void saveFile() {
        if (tabWidget->count() > 0) {
            QTextEdit *currentTextEdit = qobject_cast<QTextEdit*>(tabWidget->currentWidget());
            if (currentTextEdit) {
                QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Text Files (*.txt);;All Files (*)"));
                if (!fileName.isEmpty()) {
                    QFile file(fileName);
                    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        QTextStream out(&file);
                        out << currentTextEdit->toPlainText();
                        file.close();
                    }
                }
            }
        }
    }

private:
    QTabWidget *tabWidget;

    void createMenu() {
        QMenu *fileMenu = menuBar()->addMenu(tr("File"));

        QAction *openAction = new QAction(tr("Open"), this);
        connect(openAction, &QAction::triggered, this, &TextEditor::openFile);
        fileMenu->addAction(openAction);

        QAction *saveAction = new QAction(tr("Save"), this);
        connect(saveAction, &QAction::triggered, this, &TextEditor::saveFile);
        fileMenu->addAction(saveAction);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    TextEditor editor;

    // Check if command line arguments are provided
    bool initialFilePathProvided  = false;
    if (argc > 1) {
        // Attempt to open the file specified in the command line argument
        QString fileName = argv[1];
        QFile file(fileName);
        if (file.exists()) {
            initialFilePathProvided = true;
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextEdit *textEdit = new QTextEdit(&editor);
                textEdit->setPlainText(QString::fromUtf8(file.readAll()));
                file.close();

                // Add the initial tab with the content of the specified file
                int tabIndex = editor.getTabWidget()->addTab(textEdit, QFileInfo(fileName).fileName());
                editor.getTabWidget()->setCurrentIndex(tabIndex);
            }
        }
    }

    // If no initial file path was provided, add an empty tab
    if (!initialFilePathProvided) {
        QTextEdit *textEdit = new QTextEdit(&editor);
        int tabIndex = editor.getTabWidget()->addTab(textEdit, "Untitled");
        editor.getTabWidget()->setCurrentIndex(tabIndex);
    }

    editor.show();

    return app.exec();
}


#include "main.moc"

