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
  void settings_applied();

private slots:
  void on_apply();
  void on_add_alias();
  void on_remove_alias();

private:
  void build_ui();
  void load_from_manager();
  void write_to_manager();

  QComboBox *m_input_source_combo;
  QCheckBox *m_markdown_check;
  QCheckBox *m_accent_check;
  QCheckBox *m_punctuation_check;
  QTableWidget *m_alias_table;
  QLineEdit *m_alias_from_edit;
  QLineEdit *m_alias_to_edit;
  QPushButton *m_add_alias_btn;
  QPushButton *m_remove_alias_btn;
  QDialogButtonBox *m_button_box;
};

#endif // SETTINGS_WINDOW_H
