#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QTextStream>
#include <QString>

class TextEditor : public QMainWindow {
    Q_OBJECT

public:
    TextEditor(QWidget *parent = nullptr) : QMainWindow(parent) {
        textEdit = new QTextEdit(this);
        setCentralWidget(textEdit);

        createMenu();

        setWindowTitle("WolfEdit");
        resize(800, 600);
    }

private slots:
    void openFile() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Text Files (*.txt);;All Files (*)"));
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                textEdit->setPlainText(in.readAll());
                file.close();
            }
        }
    }

    void saveFile() {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Text Files (*.txt);;All Files (*)"));
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << textEdit->toPlainText();
                file.close();
            }
        }
    }

private:
    QTextEdit *textEdit;

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
    editor.show();

    return app.exec();
}

#include "main.moc"
