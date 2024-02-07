
#include <QApplication>
#include <QObject>

#include "src/WolfEdit.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  WolfEdit::WolfEdit *editor = new WolfEdit::WolfEdit();
  editor->show();

  return app.exec();
}

#include "main.moc"
