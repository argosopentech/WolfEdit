#include <atomic>

#include <QAction>
#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStandardPaths>
#include <QString>
#include <QTabWidget>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>

#include <fakevim/fakevimhandler.h>

#include "editor.h"

#include <iostream>

namespace WolfEdit {

static const QString APP_NAME = "WolfEdit";

static const QString ABOUT_TEXT =
    "WolfEdit is a free and open source text editor built on the Qt C++ "
    "framework. WolfEdit is designed to be used by programmers and people that "
    "frequently edit text files. WolfEdit is focused on simplicity and "
    "performance so that programmers can customize their editor by modifying "
    "the source directly and have a snappy user experience.";

static const QString FOOTER_TEXT = "© 2024 Argos Open Technologies, LLC";

class Tab : public QWidget {
  Q_OBJECT
public:
  Tab(QWidget *parent = nullptr) : QWidget(parent) {
    this->vimEditor = new VimEditor(this);
    connect(this->vimEditor, &VimEditor::requestSave, this, &Tab::requestSave);
    connect(this->vimEditor, &VimEditor::requestSaveAndQuit, this,
            &Tab::requestSaveAndQuit);
    connect(this->vimEditor, &VimEditor::requestQuit, this, &Tab::requestQuit);
    this->textEdit = this->vimEditor->textEdit;
    connect(this->textEdit, &QTextEdit::textChanged, this, &Tab::textModified);
    modified = false;
    layout = new QVBoxLayout(this);
    layout->addWidget(this->vimEditor);
    setLayout(layout);
    this->vimEditor->textEdit->setFocus();
  }

  VimEditor *vimEditor;
  QTextEdit *textEdit;
  QString filePath;
  QVBoxLayout *layout;
  std::atomic<bool> modified;
  QString getFilePath() const { return filePath; }
  void setFilePath(const QString &filePath) { this->filePath = filePath; }
  bool isModified() const { return modified; }
  void setModified(bool modified) { this->modified = modified; }
  bool unsavedChanges() const { return isModified(); }

  bool save() {
    if (filePath.isEmpty()) {
      return false;
    }
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream out(&file);
      out << textEdit->toPlainText();
      file.close();
      return true;
    }
    return false;
  }

signals:
  void requestSave();
  void requestSaveAndQuit();
  void requestQuit();

private slots:
  void textModified() { this->modified = true; }
};

class TabWidget : public QTabWidget {
  Q_OBJECT
public:
  TabWidget(QWidget *parent = nullptr) : QTabWidget(parent) {
    setTabsClosable(true);
  }
  Tab *getCurrentTab() const { return qobject_cast<Tab *>(currentWidget()); }
  int getCurrentTabIndex() const { return currentIndex(); }
  Tab *getTab(int index) const { return qobject_cast<Tab *>(widget(index)); }

signals:
  void requestSave();
  void requestSaveAndQuit();
  void requestQuit();
};

class WolfEdit : public QMainWindow {
  Q_OBJECT
public:
  WolfEdit(QWidget *parent = nullptr) : QMainWindow(parent) {
    tabWidget = new TabWidget(this);
    setCentralWidget(tabWidget);
    connect(tabWidget, &TabWidget::tabCloseRequested, this,
            &WolfEdit::closeTab);
    connect(tabWidget, &TabWidget::requestSave, this, &WolfEdit::saveFile);
    connect(tabWidget, &TabWidget::requestSaveAndQuit, this,
            &WolfEdit::saveAndQuit);
    connect(tabWidget, &TabWidget::requestQuit, this, &WolfEdit::quit);
    addEmptyTab();
    createMenu();
    setWindowTitle(APP_NAME);
    resize(800, 600);
  }

protected:
  void closeEvent(QCloseEvent *event) override {
    quit();
    // Call the base class closeEvent to allow the event to be processed
    QMainWindow::closeEvent(event);
  }

public slots:
  void openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
                                                    tr("All Files (*)"));
    addTab(fileName);
  }

  void saveFile() {
    if (tabWidget->count() > 0) {
      // TODO: Saving probably shouldn't necessarily be tied to the current tab
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

        currentTab->save();
        currentTab->setModified(false);
      }
    }
  }

  void saveAndQuit() {
    saveFile();
    quit();
  }

  void quit() {
    // Iterate through all tabs and check for unsaved changes
    for (int i = 0; i < tabWidget->count(); i++) {
      closeTab(i);
    }
  }

  void saveFileAs() {
    Tab *currentTab = tabWidget->getCurrentTab();
    if (currentTab) {
      QString currentFilePath = currentTab->getFilePath();

      QString newFilePath = QFileDialog::getSaveFileName(
          this, tr("Save File As"), currentFilePath,
          tr("Text Files (*.txt);;All Files (*)"));
      if (!newFilePath.isEmpty()) {
        tabWidget->getCurrentTab()->setFilePath(newFilePath);
        tabWidget->setTabText(tabWidget->currentIndex(),
                              QFileInfo(newFilePath).fileName());
        tabWidget->setTabToolTip(tabWidget->currentIndex(), newFilePath);
        currentTab->save();
        currentTab->setModified(false);
      }
    }
  }

  void newFile() {
    Tab *textEdit = new Tab(this);
    addTab("");
  }

  void closeTab(int index) {
    Tab *tab = tabWidget->getTab(index);
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
    connect(newAction, &QAction::triggered, this, &WolfEdit::newFile);
    fileMenu->addAction(newAction);

    QAction *openAction = new QAction(tr("Open"), this);
    connect(openAction, &QAction::triggered, this, &WolfEdit::openFile);
    fileMenu->addAction(openAction);

    QAction *saveAction = new QAction(tr("Save"), this);
    connect(saveAction, &QAction::triggered, this, &WolfEdit::saveFile);
    fileMenu->addAction(saveAction);

    QAction *saveAsAction = new QAction(tr("Save As"), this);
    connect(saveAsAction, &QAction::triggered, this, &WolfEdit::saveFileAs);
    fileMenu->addAction(saveAsAction);

    // Add an "About" action
    QAction *aboutAction = new QAction(tr("About"), this);
    connect(aboutAction, &QAction::triggered, this, &WolfEdit::showAboutDialog);
    menuBar()->addAction(aboutAction);
  }

  // Add this function to handle the "About" action
  void showAboutDialog() {
    QMessageBox::about(this, tr("About WolfEdit"),
                       ABOUT_TEXT + "\n\n" + FOOTER_TEXT);
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
    connect(tab, &Tab::requestSave, tabWidget, &TabWidget::requestSave);
    connect(tab, &Tab::requestSaveAndQuit, tabWidget,
            &TabWidget::requestSaveAndQuit);
    connect(tab, &Tab::requestQuit, tabWidget, &TabWidget::requestQuit);
  }

  void addEmptyTab() {
    Tab *tab = new Tab(this);
    int tabIndex = tabWidget->addTab(tab, "");
    tabWidget->setTabToolTip(tabIndex, "");
  }
};

} // namespace WolfEdit