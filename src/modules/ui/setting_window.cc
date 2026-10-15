// Setting Window - QDialog for editing user preferences.

#include "setting_window.hpp"

#include "../setting/setting.hpp"

#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QVBoxLayout>

SettingWindow::SettingWindow(QWidget *parent)
    : QDialog(parent) {
  setWindowTitle(tr("Gladys Settings"));
  setMinimumWidth(480);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  build_ui();
  load_from_setting();
}

void SettingWindow::build_ui() {
  auto *mainLayout = new QVBoxLayout(this);

  // --- Input Source ---
  auto *audioGroup = new QGroupBox(tr("Input Source"), this);
  auto *audioLayout = new QVBoxLayout(audioGroup);
  m_input_source_combo = new QComboBox(this);
  m_input_source_combo->addItems(Setting::available_input_sources());
  audioLayout->addWidget(m_input_source_combo);
  mainLayout->addWidget(audioGroup);

  // --- Dictionary (Aliases) ---
  auto *dictGroup = new QGroupBox(tr("Dictionary (Aliases)"), this);
  auto *dictLayout = new QVBoxLayout(dictGroup);

  m_alias_table = new QTableWidget(0, 2, this);
  m_alias_table->setHorizontalHeaderLabels({tr("From"), tr("To")});
  m_alias_table->horizontalHeader()->setStretchLastSection(true);
  m_alias_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  dictLayout->addWidget(m_alias_table);

  auto *aliasInputLayout = new QFormLayout();
  m_alias_from_edit = new QLineEdit(this);
  m_alias_from_edit->setPlaceholderText(tr("Original word"));
  m_alias_to_edit = new QLineEdit(this);
  m_alias_to_edit->setPlaceholderText(tr("Replacement"));
  aliasInputLayout->addRow(tr("From:"), m_alias_from_edit);
  aliasInputLayout->addRow(tr("To:"), m_alias_to_edit);
  dictLayout->addLayout(aliasInputLayout);

  auto *aliasBtnLayout = new QVBoxLayout();
  auto *addAliasBtn = new QPushButton(tr("Add Alias"), this);
  auto *removeAliasBtn = new QPushButton(tr("Remove Selected"), this);
  aliasBtnLayout->addWidget(addAliasBtn);
  aliasBtnLayout->addWidget(removeAliasBtn);
  dictLayout->addLayout(aliasBtnLayout);

  connect(addAliasBtn, &QPushButton::clicked, this, &SettingWindow::on_add_alias);
  connect(removeAliasBtn, &QPushButton::clicked, this, &SettingWindow::on_remove_alias);
  mainLayout->addWidget(dictGroup);

  // --- Style ---
  auto *styleGroup = new QGroupBox(tr("Style"), this);
  auto *styleLayout = new QVBoxLayout(styleGroup);
  m_markdown_check = new QCheckBox(tr("Markdown mode"), this);
  m_accent_check = new QCheckBox(tr("Allow accents"), this);
  m_punctuation_check = new QCheckBox(tr("Allow punctuation"), this);
  styleLayout->addWidget(m_markdown_check);
  styleLayout->addWidget(m_accent_check);
  styleLayout->addWidget(m_punctuation_check);
  mainLayout->addWidget(styleGroup);

  auto *applyBtn = new QPushButton(tr("Apply"), this);
  connect(applyBtn, &QPushButton::clicked, this, &SettingWindow::on_apply);
  mainLayout->addWidget(applyBtn);
}

void SettingWindow::load_from_setting() {
  Setting *s = Setting::instance();

  int idx = m_input_source_combo->findText(s->input_source());
  if (idx >= 0) {
    m_input_source_combo->setCurrentIndex(idx);
  }

  m_alias_table->setRowCount(0);
  const auto dict = s->dictionary();
  for (auto it = dict.cbegin(); it != dict.cend(); ++it) {
    int row = m_alias_table->rowCount();
    m_alias_table->insertRow(row);
    m_alias_table->setItem(row, 0, new QTableWidgetItem(it.key()));
    m_alias_table->setItem(row, 1, new QTableWidgetItem(it.value()));
  }

  m_markdown_check->setChecked(s->markdown_mode());
  m_accent_check->setChecked(s->allow_accents());
  m_punctuation_check->setChecked(s->allow_punctuation());
}

void SettingWindow::write_to_setting() {
  Setting *s = Setting::instance();

  s->set_input_source(m_input_source_combo->currentText());

  QMap<QString, QString> dict;
  for (int i = 0; i < m_alias_table->rowCount(); ++i) {
    auto *fromItem = m_alias_table->item(i, 0);
    auto *toItem = m_alias_table->item(i, 1);
    if (fromItem && !fromItem->text().isEmpty()) {
      dict.insert(fromItem->text(), toItem ? toItem->text() : QString());
    }
  }
  s->set_dictionary(dict);

  s->set_markdown_mode(m_markdown_check->isChecked());
  s->set_allow_accents(m_accent_check->isChecked());
  s->set_allow_punctuation(m_punctuation_check->isChecked());
}

void SettingWindow::on_apply() {
  write_to_setting();
  Setting::instance()->save();
}

void SettingWindow::on_add_alias() {
  QString from = m_alias_from_edit->text().trimmed();
  QString to = m_alias_to_edit->text().trimmed();
  if (from.isEmpty()) {
    return;
  }
  int row = m_alias_table->rowCount();
  m_alias_table->insertRow(row);
  m_alias_table->setItem(row, 0, new QTableWidgetItem(from));
  m_alias_table->setItem(row, 1, new QTableWidgetItem(to));
  m_alias_from_edit->clear();
  m_alias_to_edit->clear();
  m_alias_from_edit->setFocus();
}

void SettingWindow::on_remove_alias() {
  int row = m_alias_table->currentRow();
  if (row >= 0) {
    m_alias_table->removeRow(row);
  }
}
