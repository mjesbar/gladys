// Settings Window - QDialog for editing all user preferences.

#include "settings_window.h"
#include "settings_manager.h"

#include <QFormLayout>
#include <QHeaderView>
#include <QVBoxLayout>

SettingsWindow::SettingsWindow(QWidget *parent)
    : QDialog(parent), m_inputSourceCombo(nullptr), m_markdownCheck(nullptr),
      m_accentCheck(nullptr), m_punctuationCheck(nullptr),
      m_aliasTable(nullptr), m_aliasFromEdit(nullptr),
      m_aliasToEdit(nullptr), m_addAliasBtn(nullptr),
      m_removeAliasBtn(nullptr), m_buttonBox(nullptr) {
  setWindowTitle(tr("Gladys Settings"));
  setMinimumWidth(480);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  buildUi();
  loadFromManager();
}

void SettingsWindow::buildUi() {
  auto *mainLayout = new QVBoxLayout(this);

  // --- Input Source ---
  auto *audioGroup = new QGroupBox(tr("Input Source"), this);
  auto *audioLayout = new QVBoxLayout(audioGroup);
  m_inputSourceCombo = new QComboBox(this);
  m_inputSourceCombo->addItems(SettingsManager::availableInputSources());
  audioLayout->addWidget(m_inputSourceCombo);
  mainLayout->addWidget(audioGroup);

  // --- Dictionary (Aliases) ---
  auto *dictGroup = new QGroupBox(tr("Dictionary (Aliases)"), this);
  auto *dictLayout = new QVBoxLayout(dictGroup);

  m_aliasTable = new QTableWidget(0, 2, this);
  m_aliasTable->setHorizontalHeaderLabels({tr("From"), tr("To")});
  m_aliasTable->horizontalHeader()->setStretchLastSection(true);
  m_aliasTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  dictLayout->addWidget(m_aliasTable);

  auto *aliasInputLayout = new QFormLayout();
  m_aliasFromEdit = new QLineEdit(this);
  m_aliasFromEdit->setPlaceholderText(tr("Original word"));
  m_aliasToEdit = new QLineEdit(this);
  m_aliasToEdit->setPlaceholderText(tr("Replacement"));
  aliasInputLayout->addRow(tr("From:"), m_aliasFromEdit);
  aliasInputLayout->addRow(tr("To:"), m_aliasToEdit);
  dictLayout->addLayout(aliasInputLayout);

  auto *aliasBtnLayout = new QVBoxLayout();
  m_addAliasBtn = new QPushButton(tr("Add Alias"), this);
  m_removeAliasBtn = new QPushButton(tr("Remove Selected"), this);
  aliasBtnLayout->addWidget(m_addAliasBtn);
  aliasBtnLayout->addWidget(m_removeAliasBtn);
  dictLayout->addLayout(aliasBtnLayout);

  connect(m_addAliasBtn, &QPushButton::clicked, this,
          &SettingsWindow::onAddAlias);
  connect(m_removeAliasBtn, &QPushButton::clicked, this,
          &SettingsWindow::onRemoveAlias);

  mainLayout->addWidget(dictGroup);

  // --- Style ---
  auto *styleGroup = new QGroupBox(tr("Style"), this);
  auto *styleLayout = new QVBoxLayout(styleGroup);
  m_markdownCheck = new QCheckBox(tr("Markdown mode"), this);
  m_accentCheck = new QCheckBox(tr("Allow accents"), this);
  m_punctuationCheck = new QCheckBox(tr("Allow punctuation"), this);
  styleLayout->addWidget(m_markdownCheck);
  styleLayout->addWidget(m_accentCheck);
  styleLayout->addWidget(m_punctuationCheck);
  mainLayout->addWidget(styleGroup);

  // --- Apply button ---
  auto *btnLayout = new QVBoxLayout();
  auto *applyBtn = new QPushButton(tr("Apply"), this);
  btnLayout->addWidget(applyBtn);
  connect(applyBtn, &QPushButton::clicked, this, &SettingsWindow::onApply);
  mainLayout->addLayout(btnLayout);
}

void SettingsWindow::loadFromManager() {
  SettingsManager *mgr = SettingsManager::instance();

  // Input source
  int idx = m_inputSourceCombo->findText(mgr->inputSource());
  if (idx >= 0)
    m_inputSourceCombo->setCurrentIndex(idx);

  // Dictionary
  m_aliasTable->setRowCount(0);
  const auto dict = mgr->dictionary();
  for (auto it = dict.cbegin(); it != dict.cend(); ++it) {
    int row = m_aliasTable->rowCount();
    m_aliasTable->insertRow(row);
    m_aliasTable->setItem(row, 0, new QTableWidgetItem(it.key()));
    m_aliasTable->setItem(row, 1, new QTableWidgetItem(it.value()));
  }

  // Style
  m_markdownCheck->setChecked(mgr->markdownMode());
  m_accentCheck->setChecked(mgr->allowAccents());
  m_punctuationCheck->setChecked(mgr->allowPunctuation());
}

void SettingsWindow::writeToManager() {
  SettingsManager *mgr = SettingsManager::instance();

  mgr->setInputSource(m_inputSourceCombo->currentText());

  QMap<QString, QString> dict;
  for (int i = 0; i < m_aliasTable->rowCount(); ++i) {
    auto *fromItem = m_aliasTable->item(i, 0);
    auto *toItem = m_aliasTable->item(i, 1);
    if (fromItem && !fromItem->text().isEmpty()) {
      dict.insert(fromItem->text(),
                  toItem ? toItem->text() : QString());
    }
  }
  mgr->setDictionary(dict);

  mgr->setMarkdownMode(m_markdownCheck->isChecked());
  mgr->setAllowAccents(m_accentCheck->isChecked());
  mgr->setAllowPunctuation(m_punctuationCheck->isChecked());
}

void SettingsWindow::onApply() {
  writeToManager();
  emit settingsApplied();
}

void SettingsWindow::onAddAlias() {
  QString from = m_aliasFromEdit->text().trimmed();
  QString to = m_aliasToEdit->text().trimmed();
  if (from.isEmpty()) {
    return;
  }
  int row = m_aliasTable->rowCount();
  m_aliasTable->insertRow(row);
  m_aliasTable->setItem(row, 0, new QTableWidgetItem(from));
  m_aliasTable->setItem(row, 1, new QTableWidgetItem(to));
  m_aliasFromEdit->clear();
  m_aliasToEdit->clear();
  m_aliasFromEdit->setFocus();
}

void SettingsWindow::onRemoveAlias() {
  int row = m_aliasTable->currentRow();
  if (row >= 0) {
    m_aliasTable->removeRow(row);
  }
}
