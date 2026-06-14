// Settings Window - QDialog for editing all user preferences.

#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

class SettingsWindow : public QDialog {
  Q_OBJECT

public:
  explicit SettingsWindow(QWidget *parent = nullptr);

signals:
  void settingsApplied();

private slots:
  void onApply();
  void onAddAlias();
  void onRemoveAlias();

private:
  void buildUi();
  void loadFromManager();
  void writeToManager();

  QComboBox *m_inputSourceCombo;
  QCheckBox *m_markdownCheck;
  QCheckBox *m_accentCheck;
  QCheckBox *m_punctuationCheck;
  QTableWidget *m_aliasTable;
  QLineEdit *m_aliasFromEdit;
  QLineEdit *m_aliasToEdit;
  QPushButton *m_addAliasBtn;
  QPushButton *m_removeAliasBtn;
  QDialogButtonBox *m_buttonBox;
};

#endif // SETTINGS_WINDOW_H
