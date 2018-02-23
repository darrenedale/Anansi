#include "src/filenamewidget.h"
#include "ui_filenamewidget.h"

#include <QFileDialog>

namespace EquitWebServer {

	FileNameWidget::FileNameWidget(const QString & path, QWidget * parent)
	: FileNameWidget(parent) {
		setFileName(path);
	}


	FileNameWidget::FileNameWidget(QWidget * parent)
	: QWidget(parent),
	  m_ui(std::make_unique<Ui::FileNameWidget>()) {
		m_ui->setupUi(this);

		connect(m_ui->path, &QLineEdit::textEdited, this, &FileNameWidget::fileNameChanged);

		connect(m_ui->choose, &QPushButton::clicked, [this]() {
			chooseFile();
		});
	}


	QString FileNameWidget::placeholderText() const {
		return m_ui->path->placeholderText();
	}


	FileNameWidget::~FileNameWidget() = default;


	void FileNameWidget::setPlaceholderText(const QString & str) {
		m_ui->path->setPlaceholderText(str);
	}


	void FileNameWidget::setFileName(const QString & path) {
		if(path == m_ui->path->text()) {
			return;
		}

		m_ui->path->setText(path);
		Q_EMIT fileNameChanged(path);
	}


	QString FileNameWidget::fileName() const {
		return m_ui->path->text();
	}


	void FileNameWidget::chooseFile(QString path) {
		if(path.isEmpty()) {
			path = m_ui->path->text();
		}

		path = QFileDialog::getOpenFileName(this, (m_dialogueCaption.isEmpty() ? tr("Choose file") : m_dialogueCaption), path);

		if(path.isNull()) {
			return;
		}

		m_ui->path->setText(path);
		Q_EMIT fileNameChanged(path);
	}

}  // namespace EquitWebServer
