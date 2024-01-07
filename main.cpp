#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QString>
#include <QTabWidget>
#include <QTextEdit>
#include <QTextStream>

class TextEditor : public QMainWindow {
  Q_OBJECT public : TextEditor(QWidget *parent = nullptr)
      : QMainWindow(parent) {
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    setCentralWidget(tabWidget);
    // Connect tabCloseRequested signal to a slot for handling tab closing
    connect(tabWidget, &QTabWidget::tabCloseRequested, this,
            &TextEditor::closeTab);
    // Add an initial empty tab
    addEmptyTab();

    createMenu();

    setWindowTitle("WolfEdit");
    resize(800, 600);
  }

  QTabWidget *getTabWidget() const { return tabWidget; }

protected:
  void closeEvent(QCloseEvent *event) override {
    // Iterate through all tabs and check for unsaved changes
    for (int i = 0; i < tabWidget->count(); i++) {
      QTextEdit *textEdit = qobject_cast<QTextEdit *>(tabWidget->widget(i));
      if (textEdit && textEdit->document()->isModified()) {
        tabWidget->setCurrentIndex(
            i); // Set the current tab to the one with unsaved changes

        // If there are unsaved changes, prompt the user
        QMessageBox::StandardButton button = QMessageBox::warning(
            this, "Unsaved Changes",
            "There are unsaved changes. Do you want to save before closing?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);

        if (button == QMessageBox::Save) {
          // Save the changes and continue closing
          saveFile();
        } else if (button == QMessageBox::Cancel) {
          // Cancel the close event
          event->ignore();
          return;
        }
      }
    }

    // Call the base class closeEvent to allow the event to be processed
    QMainWindow::closeEvent(event);
  }

private slots:
  void openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
                                                    tr("All Files (*)"));
    if (!fileName.isEmpty()) {
      QFile file(fileName);
      if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextEdit *textEdit = new QTextEdit(this);
        textEdit->setPlainText(QString::fromUtf8(file.readAll()));
        file.close();

        addTab(textEdit, fileName);
      }
    }
  }

  void saveFile() {
    if (tabWidget->count() > 0) {
      QTextEdit *currentTextEdit =
          qobject_cast<QTextEdit *>(tabWidget->currentWidget());
      if (currentTextEdit) {
        QString currentFilePath =
            tabWidget->tabToolTip(tabWidget->currentIndex());

        if (currentFilePath.isEmpty() || currentFilePath == "Untitled") {
          // If the file is untitled or not saved before, ask for a new file
          // name
          currentFilePath = QFileDialog::getSaveFileName(
              this, tr("Save File"), "",
              tr("Text Files (*.txt);;All Files (*)"));
          if (currentFilePath.isEmpty()) {
            return; // User canceled the save operation
          }
          // Update the tab title and tooltip
          tabWidget->setTabText(tabWidget->currentIndex(),
                                QFileInfo(currentFilePath).fileName());
          tabWidget->setTabToolTip(tabWidget->currentIndex(), currentFilePath);
        }

        saveFileWithDialog(currentTextEdit, currentFilePath);
      }
    }
  }

  void saveFileAs() {
    if (tabWidget->count() > 0) {
      QTextEdit *currentTextEdit =
          qobject_cast<QTextEdit *>(tabWidget->currentWidget());
      if (currentTextEdit) {
        QString currentFilePath =
            tabWidget->tabToolTip(tabWidget->currentIndex());

        // Ask for a new file name
        QString newFilePath = QFileDialog::getSaveFileName(
            this, tr("Save File As"), "",
            tr("Text Files (*.txt);;All Files (*)"));
        if (!newFilePath.isEmpty()) {
          // Update the tab title and tooltip
          tabWidget->setTabText(tabWidget->currentIndex(),
                                QFileInfo(newFilePath).fileName());
          tabWidget->setTabToolTip(tabWidget->currentIndex(), newFilePath);

          saveFileWithDialog(currentTextEdit, newFilePath);
        }
      }
    }
  }

  void newFile() {
    QTextEdit *textEdit = new QTextEdit(this);
    addTab(textEdit, "Untitled");
  }

  void closeTab(int index) {
    QTextEdit *textEdit = qobject_cast<QTextEdit *>(tabWidget->widget(index));
    if (textEdit && textEdit->document()->isModified()) {
      // Set the current tab to the one with unsaved changes
      tabWidget->setCurrentIndex(index);

      // If there are unsaved changes, prompt the user
      QMessageBox::StandardButton button = QMessageBox::warning(
          this, "Unsaved Changes",
          "There are unsaved changes. Do you want to save before closing?",
          QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
          QMessageBox::Save);

      if (button == QMessageBox::Save) {
        // Save the changes and continue closing the tab
        saveFile();
      } else if (button == QMessageBox::Cancel) {
        // Cancel closing the tab
        return;
      }
    }

    // Close the tab
    tabWidget->removeTab(index);
    delete textEdit;
  }

private:
  QTabWidget *tabWidget;

  void createMenu() {
    QMenu *fileMenu = menuBar()->addMenu(tr("File"));

    QAction *newAction = new QAction(tr("New"), this);
    connect(newAction, &QAction::triggered, this, &TextEditor::newFile);
    fileMenu->addAction(newAction);

    QAction *openAction = new QAction(tr("Open"), this);
    connect(openAction, &QAction::triggered, this, &TextEditor::openFile);
    fileMenu->addAction(openAction);

    QAction *saveAction = new QAction(tr("Save"), this);
    connect(saveAction, &QAction::triggered, this, &TextEditor::saveFile);
    fileMenu->addAction(saveAction);

    QAction *saveAsAction = new QAction(tr("Save As"), this);
    connect(saveAsAction, &QAction::triggered, this, &TextEditor::saveFileAs);
    fileMenu->addAction(saveAsAction);

    // Add an "About" action
    QAction *aboutAction = new QAction(tr("About"), this);
    connect(aboutAction, &QAction::triggered, this,
            &TextEditor::showAboutDialog);
    menuBar()->addAction(aboutAction);
  }

  // Add this function to handle the "About" action
  void showAboutDialog() {
    QMessageBox::about(
        this, tr("About WolfEdit"),
        tr("WolfEdit is a simple text editor.\n\n"
           "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
           "Praesent nec tellus id mi vehicula aliquet nec id ante. "
           "Quisque euismod, justo ut luctus dignissim, est diam suscipit "
           "orci, "
           "quis facilisis tortor risus id sapien."));
  }

  void addTab(QTextEdit *textEdit, const QString &filePath) {
    int tabIndex = tabWidget->addTab(textEdit, QFileInfo(filePath).fileName());
    tabWidget->setTabToolTip(tabIndex, filePath);
    tabWidget->setCurrentIndex(tabIndex);
  }

  void addEmptyTab() {
    QTextEdit *textEdit = new QTextEdit(this);
    int tabIndex = tabWidget->addTab(textEdit, "Untitled");
    tabWidget->setTabToolTip(tabIndex, "Untitled");
  }

  void saveFileWithDialog(QTextEdit *textEdit, const QString &filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream out(&file);
      out << textEdit->toPlainText();
      file.close();
    }
  }
};

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  TextEditor editor;
  editor.show();

  return app.exec();
}

#include "main.moc"
