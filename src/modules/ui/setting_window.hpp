// Setting Window - QDialog for editing user preferences.

#ifndef UI_SETTING_WINDOW_HPP
#define UI_SETTING_WINDOW_HPP

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>

class SettingWindow : public QDialog {
  Q_OBJECT

public:
  explicit SettingWindow(QWidget *parent = nullptr);

private slots:
  void on_apply();
  void on_add_alias();
  void on_remove_alias();

private:
  void build_ui();
  void load_from_setting();
  void write_to_setting();

  QComboBox *m_input_source_combo;
  QCheckBox *m_markdown_check;
  QCheckBox *m_accent_check;
  QCheckBox *m_punctuation_check;
  QTableWidget *m_alias_table;
  QLineEdit *m_alias_from_edit;
  QLineEdit *m_alias_to_edit;
};

#endif // UI_SETTING_WINDOW_HPP
