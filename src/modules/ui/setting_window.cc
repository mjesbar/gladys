// Setting Window - QDialog for editing user preferences.

#include "setting_window.hpp"

#include "../log/log.hpp"
#include "../setting/setting.hpp"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

// Dictionary table layout.
namespace {
enum AliasColumn {
  FromColumn,
  ToColumn,
  ActionColumn,
  AliasColumnCount,
};

constexpr int kAliasRowHeight = 48;
constexpr int kActionColumnWidth = 64;

// Editor covers the whole cell, so editing happens in place.
// Text margins come from the QSS rule for QTableWidget QLineEdit.
class InPlaceDelegate : public QStyledItemDelegate {
public:
  using QStyledItemDelegate::QStyledItemDelegate;

  QWidget *createEditor(QWidget *parent,
                        const QStyleOptionViewItem &option,
                        const QModelIndex &index) const override {
    Q_UNUSED(option);
    Q_UNUSED(index);
    auto *editor = new QLineEdit(parent);
    editor->setFrame(false);
    return editor;
  }

  void updateEditorGeometry(QWidget *editor,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const override {
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
  }
};
}  // namespace

SettingWindow::SettingWindow(QWidget *parent)
    : QDialog(parent) {
  setObjectName("settingWindow");
  setWindowTitle(tr("Gladys Settings"));
  setMinimumWidth(480);
  qCInfo(lcUi) << "settings window started";
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint & ~Qt::WindowMaximizeButtonHint);
  build_ui();
  load_from_setting();
}

void SettingWindow::build_ui() {
  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(32, 32, 32, 32);
  mainLayout->setSpacing(16);

  // --- Input Source ---
  auto *audioGroup = new QGroupBox(tr("Input Source"), this);
  auto *audioLayout = new QVBoxLayout(audioGroup);
  m_input_source_combo = new QComboBox(this);
  m_input_source_combo->addItems(Setting::available_input_sources());
  audioLayout->addWidget(m_input_source_combo);
  mainLayout->addWidget(audioGroup);

  // --- Dictionary ---
  auto *dictGroup = new QGroupBox(tr("Dictionary"), this);
  auto *dictLayout = new QVBoxLayout(dictGroup);

  m_alias_table = new QTableWidget(0, AliasColumnCount, this);
  m_alias_table->horizontalHeader()->setVisible(false);
  m_alias_table->horizontalHeader()->setStretchLastSection(false);
  m_alias_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  m_alias_table->horizontalHeader()->setSectionResizeMode(ActionColumn, QHeaderView::Fixed);
  m_alias_table->setColumnWidth(ActionColumn, kActionColumnWidth);
  m_alias_table->verticalHeader()->setVisible(false);
  m_alias_table->verticalHeader()->setDefaultSectionSize(kAliasRowHeight);
  m_alias_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_alias_table->setItemDelegate(new InPlaceDelegate(m_alias_table));
  dictLayout->addWidget(m_alias_table);

  auto *aliasInputLayout = new QHBoxLayout();
  m_alias_from_edit = new QLineEdit(this);
  m_alias_from_edit->setPlaceholderText(tr("Original word"));
  m_alias_to_edit = new QLineEdit(this);
  m_alias_to_edit->setPlaceholderText(tr("Replacement"));
  auto *chevron = new QLabel(this);
  chevron->setPixmap(QIcon(":/icons/chevron-right.svg").pixmap(18, 18));
  chevron->setAlignment(Qt::AlignCenter);
  auto *addAliasBtn = new QPushButton(QIcon(":/icons/plus.svg"), QString(), this);
  addAliasBtn->setObjectName("iconButton");
  addAliasBtn->setToolTip(tr("Add Alias"));
  aliasInputLayout->addWidget(m_alias_from_edit);
  aliasInputLayout->addWidget(chevron, 0, Qt::AlignCenter);
  aliasInputLayout->addWidget(m_alias_to_edit);
  aliasInputLayout->addWidget(addAliasBtn, 0, Qt::AlignCenter);
  dictLayout->addLayout(aliasInputLayout);

  connect(addAliasBtn, &QPushButton::clicked, this, &SettingWindow::on_add_alias);
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
  applyBtn->setObjectName("applyButton");
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
    add_alias_row(it.key(), it.value());
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
    auto *fromItem = m_alias_table->item(i, FromColumn);
    auto *toItem = m_alias_table->item(i, ToColumn);
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
  add_alias_row(from, to);
  m_alias_from_edit->clear();
  m_alias_to_edit->clear();
  m_alias_from_edit->setFocus();
}

// One row per mapping; the last column holds its trash button.
void SettingWindow::add_alias_row(const QString &from, const QString &to) {
  int row = m_alias_table->rowCount();
  m_alias_table->insertRow(row);
  for (int col : {FromColumn, ToColumn}) {
    auto *item = new QTableWidgetItem(col == FromColumn ? from : to);
    item->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_alias_table->setItem(row, col, item);
  }

  auto *removeBtn = new QPushButton(QIcon(":/icons/trash.svg"), QString(), m_alias_table);
  removeBtn->setObjectName("iconButton");
  removeBtn->setToolTip(tr("Remove"));
  removeBtn->setFixedSize(32, 32);
  auto *holder = new QWidget(m_alias_table);
  auto *holderLayout = new QHBoxLayout(holder);
  holderLayout->setContentsMargins(0, 0, 0, 0);
  holderLayout->addWidget(removeBtn, 0, Qt::AlignCenter);
  connect(removeBtn, &QPushButton::clicked, this, [this, holder]() {
    m_alias_table->removeRow(m_alias_table->indexAt(holder->pos()).row());
  });
  m_alias_table->setCellWidget(row, ActionColumn, holder);
}
