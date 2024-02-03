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
#include <QVBoxLayout>
#include <atomic>

static const QString APP_NAME = "WolfEdit";

class Tab : public QWidget {
  Q_OBJECT public
      : Tab(QWidget *parent = nullptr, QTextEdit *textEdit = nullptr)
      : QWidget(parent) {
    if (textEdit) {
      this->textEdit = textEdit;
    } else {
      this->textEdit = new QTextEdit(this);
    }
    connect(this->textEdit, &QTextEdit::textChanged, this, &Tab::textModified);
    modified = false;
  }

private:
  std::atomic<bool> modified;

public:
  QTextEdit *textEdit;
  QString filePath;
  bool unsavedChanges() const { return isModified(); }
  QString getFilePath() const { return filePath; }
  void setFilePath(const QString &filePath) { this->filePath = filePath; }
  bool isModified() const { return modified; }
  void setModified(bool modified) { this->modified = modified; }

private slots:
  void textModified() { this->modified = true; }
};

class TabWidget : public QTabWidget {
  Q_OBJECT public : TabWidget(QWidget *parent = nullptr) : QTabWidget(parent) {
    setTabsClosable(true);
  }

  Tab *getCurrentTab() const { return qobject_cast<Tab *>(currentWidget()); }

  Tab *getTab(int index) const { return qobject_cast<Tab *>(widget(index)); }
};

class TextEditor : public QMainWindow {
  Q_OBJECT public : TextEditor(QWidget *parent = nullptr)
      : QMainWindow(parent) {
    tabWidget = new TabWidget(this);
    setCentralWidget(tabWidget);
    connect(tabWidget, &TabWidget::tabCloseRequested, this,
            &TextEditor::closeTab);
    addEmptyTab();
    createMenu();
    setWindowTitle(APP_NAME);
    resize(800, 600);
  }

protected:
  void closeEvent(QCloseEvent *event) override {
    // Iterate through all tabs and check for unsaved changes
    for (int i = 0; i < tabWidget->count(); i++) {
      Tab *textEdit = tabWidget->getTab(i);
      if (textEdit && textEdit->unsavedChanges()) {
        tabWidget->setCurrentIndex(i);

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
    addTab(fileName);
  }

  void saveFile() {
    if (tabWidget->count() > 0) {
      Tab *currentTab = tabWidget->getCurrentTab();
      if (currentTab) {
        QString currentFilePath = tabWidget->getCurrentTab()->getFilePath();

        if (currentFilePath.isEmpty()) {
          // If the file is untitled or not saved before, ask for a new file
          // name
          currentFilePath = QFileDialog::getSaveFileName(
              this, tr("Save File"), "",
              tr("Text Files (*.txt);;All Files (*)"));
          if (currentFilePath.isEmpty()) {
            return; // User canceled the save operation
          }
          QString filepath = QFileInfo(currentFilePath).fileName();
          tabWidget->getCurrentTab()->setFilePath(currentFilePath);
          tabWidget->setTabText(tabWidget->currentIndex(), filepath);
          tabWidget->setTabToolTip(tabWidget->currentIndex(), currentFilePath);
        }

        saveTab(currentTab, currentFilePath);
        currentTab->setModified(false);
      }
    }
  }

  void saveFileAs() {
    if (tabWidget->count() > 0) {
      Tab *currentTextEdit = tabWidget->getCurrentTab();
      if (currentTextEdit) {
        QString currentFilePath = currentTextEdit->getFilePath();

        // Ask for a new file name
        QString newFilePath = QFileDialog::getSaveFileName(
            this, tr("Save File As"), "",
            tr("Text Files (*.txt);;All Files (*)"));
        if (!newFilePath.isEmpty()) {
          // Update the tab title and tooltip
          tabWidget->getCurrentTab()->setFilePath(newFilePath);
          tabWidget->setTabText(tabWidget->currentIndex(),
                                QFileInfo(newFilePath).fileName());
          tabWidget->setTabToolTip(tabWidget->currentIndex(), newFilePath);

          saveTab(currentTextEdit, newFilePath);
          currentTextEdit->setModified(false);
        }
      }
    }
  }

  void newFile() {
    Tab *textEdit = new Tab(this);
    addTab("");
  }

  void closeTab(int index) {
    Tab *tab = qobject_cast<Tab *>(tabWidget->widget(index));
    if (tab && tab->isModified()) {
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
    delete tab;

    // If there are no tabs left close the main window
    if (tabWidget->count() == 0) {
      close();
    }
  }

private:
  TabWidget *tabWidget;

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

  void addTab(QString filePath) {
    Tab *tab = new Tab(this);
    if (!filePath.isEmpty()) {
      QFile file(filePath);
      if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        tab->textEdit->setPlainText(QString::fromUtf8(file.readAll()));
        file.close();
      }
    }
    int tabIndex = tabWidget->addTab(tab, QFileInfo(filePath).fileName());
    tabWidget->setTabToolTip(tabIndex, filePath);
    tabWidget->setCurrentIndex(tabIndex);
    tabWidget->getTab(tabIndex)->setFilePath(filePath);
  }

  void addEmptyTab() {
    Tab *textEdit = new Tab(this);
    int tabIndex = tabWidget->addTab(textEdit, "");
    tabWidget->setTabToolTip(tabIndex, "");
  }

  void saveTab(Tab *textEdit, const QString &filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream out(&file);
      out << textEdit->textEdit->toPlainText();
      file.close();
    }
  }
};

/*
    Copyright (c) 2017, Lukas Holecek <hluk@email.cz>

    This file is part of CopyQ.

    CopyQ is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CopyQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CopyQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "src/editor.h"
#include <fakevim/fakevimhandler.h>

#include <QApplication>
#include <QFile>
#include <QMainWindow>
#include <QStandardPaths>

int main(int argc, char *argv[]) {
  // QApplication app(argc, argv);

  // QMainWindow *vimClientWindow = VimClient::vim_edit();

  // TextEditor editor;
  // editor.show();

  // return app.exec();
  QApplication app(argc, argv);
  const QString fileToEdit = "";

  // QWidget *editor = createEditorWidget();
  VimEditor *editor = new VimEditor();

  FakeVim::Internal::FakeVimHandler *handler = editor->handler;

  // Create main window.
  QMainWindow *mainWindow = new QMainWindow();

  // Create layout
  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(editor);
  QWidget *centralWidget = new QWidget();
  centralWidget->setLayout(layout);
  mainWindow->setCentralWidget(centralWidget);
  mainWindow->resize(600, 650);
  mainWindow->move(0, 0);
  mainWindow->show();

  // Connect slots to FakeVimHandler signals.
  Proxy *proxy = connectSignals(handler, editor, editor->statusBar);

  QObject::connect(
      proxy, &Proxy::handleInput, handler,
      [handler](const QString &text) { handler->handleInput(text); });

  QString fileName = fileToEdit;
  QObject::connect(proxy, &Proxy::requestSave, proxy,
                   [proxy, fileName]() { proxy->save(fileName); });

  QObject::connect(proxy, &Proxy::requestSaveAndQuit, proxy,
                   [proxy, fileName]() {
                     if (proxy->save(fileName)) {
                       proxy->cancel(fileName);
                     }
                   });
  QObject::connect(proxy, &Proxy::requestQuit, proxy,
                   [proxy, fileName]() { proxy->cancel(fileName); });

  // Initialize FakeVimHandler.
  initHandler(handler);

  // Load vimrc if it exists
  QString vimrc = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
#ifdef Q_OS_WIN
                  + QLatin1String("/_vimrc");
#else
                  + QLatin1String("/.vimrc");
#endif
  if (QFile::exists(vimrc)) {
    handler->handleCommand(QLatin1String("source ") + vimrc);
  } else {
    // Set some Vim options.
    handler->handleCommand(QLatin1String("set expandtab"));
    handler->handleCommand(QLatin1String("set shiftwidth=8"));
    handler->handleCommand(QLatin1String("set tabstop=16"));
    handler->handleCommand(QLatin1String("set autoindent"));
    handler->handleCommand(QLatin1String("set smartindent"));
  }

  // Clear undo and redo queues.
  clearUndoRedo(editor);

  if (!fileToEdit.isEmpty()) {
    proxy->openFile(fileToEdit);
  }

  app.exec();
}

#include "main.moc"
