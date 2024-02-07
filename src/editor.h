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

#pragma once

#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QObject>
#include <QPaintEvent>
#include <QPainter>
#include <QStandardPaths>
#include <QTextEdit>
#include <QVBoxLayout>
#include <fakevim/fakevimhandler.h>

class QMainWindow;
class QTextDocument;
class QString;
class QWidget;
class QTextCursor;

class Proxy;

namespace FakeVim {
namespace Internal {
class FakeVimHandler;
struct ExCommand;
} // namespace Internal
} // namespace FakeVim

QWidget *createEditorWidget();
void initHandler(FakeVim::Internal::FakeVimHandler *handler);
void clearUndoRedo(QWidget *editor);
Proxy *connectSignals(FakeVim::Internal::FakeVimHandler *handler,
                      QWidget *editor, QLabel *statusBar);

class Proxy : public QObject {
  Q_OBJECT

public:
  explicit Proxy(QWidget *widget, QLabel *statusBar, QObject *parent = nullptr);
  void openFile(const QString &fileName);

  bool save(const QString &fileName);
  void cancel(const QString &fileName);

signals:
  void handleInput(const QString &keys);
  void requestSave();
  void requestSaveAndQuit();
  void requestQuit();
  void requestRun();

public slots:
  void changeStatusData(const QString &info);
  void highlightMatches(const QString &pattern);
  void changeStatusMessage(const QString &contents, int cursorPos);
  void changeExtraInformation(const QString &info);
  void updateStatusBar();
  void handleExCommand(bool *handled, const FakeVim::Internal::ExCommand &cmd);
  void requestSetBlockSelection(const QTextCursor &tc);
  void requestDisableBlockSelection();
  void updateBlockSelection();
  void requestHasBlockSelection(bool *on);
  void indentRegion(int beginBlock, int endBlock, QChar typedChar);
  void checkForElectricCharacter(bool *result, QChar c);

private:
  static int firstNonSpace(const QString &text);

  void updateExtraSelections();
  bool wantSaveAndQuit(const FakeVim::Internal::ExCommand &cmd);
  bool wantSave(const FakeVim::Internal::ExCommand &cmd);
  bool wantQuit(const FakeVim::Internal::ExCommand &cmd);
  bool wantRun(const FakeVim::Internal::ExCommand &cmd);

  void invalidate();
  bool hasChanges(const QString &fileName);

  QTextDocument *document() const;
  QString content() const;

  QWidget *m_widget;
  QLabel *statusBar;
  QString m_statusMessage;
  QString m_statusData;

  QList<QTextEdit::ExtraSelection> m_searchSelection;
  QList<QTextEdit::ExtraSelection> m_clearSelection;
  QList<QTextEdit::ExtraSelection> m_blockSelection;
};

class Editor : public QTextEdit {
public:
  explicit Editor(QWidget *parent = nullptr) : QTextEdit(parent) {
    QTextEdit::setCursorWidth(0);
  }

  void paintEvent(QPaintEvent *e) {
    QTextEdit::paintEvent(e);

    if (!m_cursorRect.isNull() && e->rect().intersects(m_cursorRect)) {
      QRect rect = m_cursorRect;
      m_cursorRect = QRect();
      QTextEdit::viewport()->update(rect);
    }

    // Draw text cursor.
    QRect rect = QTextEdit::cursorRect();
    if (e->rect().intersects(rect)) {
      QPainter painter(QTextEdit::viewport());

      if (QTextEdit::overwriteMode()) {
        QFontMetrics fm(QTextEdit::font());
        const int position = QTextEdit::textCursor().position();
        const QChar c = QTextEdit::document()->characterAt(position);
        rect.setWidth(fm.horizontalAdvance(c));
        painter.setPen(Qt::NoPen);
        painter.setBrush(QTextEdit::palette().color(QPalette::Base));
        painter.setCompositionMode(QPainter::CompositionMode_Difference);
      } else {
        rect.setWidth(QTextEdit::cursorWidth());
        painter.setPen(QTextEdit::palette().color(QPalette::Text));
      }

      painter.drawRect(rect);
      m_cursorRect = rect;
    }
  }

private:
  QRect m_cursorRect;
};

class VimEditor : public QWidget {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  FakeVim::Internal::FakeVimHandler *handler;
  QTextEdit *textEdit;
  QLabel *statusBar;
  VimEditor(QWidget *parent = nullptr) {
    textEdit = new Editor(this);
    textEdit->setCursorWidth(0);
    handler = new FakeVim::Internal::FakeVimHandler(this->textEdit, 0);
    statusBar = new QLabel(this);
    configureFont();
    layout = new QVBoxLayout(this);
    layout->addWidget(textEdit);
    layout->addWidget(statusBar);
    setLayout(layout);

    // Connect slots to FakeVimHandler signals.
    Proxy *proxy = connectSignals(handler, textEdit, statusBar);
    QObject::connect(
        proxy, &Proxy::handleInput, handler,
        [this](const QString &text) { this->handler->handleInput(text); });

    connect(proxy, &Proxy::requestSave, this, &VimEditor::requestSave);
    connect(proxy, &Proxy::requestSaveAndQuit, this,
            &VimEditor::requestSaveAndQuit);
    connect(proxy, &Proxy::requestQuit, this, &VimEditor::requestQuit);

    // Initialize FakeVimHandler.
    initHandler(handler);

    // Load vimrc if it exists
    QString vimrc =
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
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
    clearUndoRedo(textEdit);

    // TODO
    const QString fileToEdit = "";
    if (!fileToEdit.isEmpty()) {
      proxy->openFile(fileToEdit);
    }
  }
  ~VimEditor() { delete handler; }

signals:
  void requestSave();
  void requestSaveAndQuit();
  void requestQuit();

private:
  void configureFont() {
    if (textEdit == nullptr || statusBar == nullptr) {
      qWarning() << "textEdit or statusBar is null, make sure they are "
                    "initialized before calling configureFont()";
    }
    QFont font = QApplication::font();
    font.setFamily("Monospace");
    textEdit->setFont(font);
    statusBar->setFont(font);
  }
};
