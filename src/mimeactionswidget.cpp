#include "src/mimeactionswidget.h"
#include "ui_mimeactionswidget.h"

#include "servermimeactionsmodel.h"


namespace EquitWebServer {


	MimeActionsWidget::MimeActionsWidget(QWidget * parent)
	: QWidget(parent),
	  m_ui(std::make_unique<Ui::MimeActionsWidget>()) {
		m_ui->setupUi(this);
	}


	MimeActionsWidget::MimeActionsWidget(Server * server, QWidget * parent)
	: MimeActionsWidget(parent) {
		setServer(server);
	}


	MimeActionsWidget::~MimeActionsWidget() = default;


	void MimeActionsWidget::setServer(Server * server) {
		if(!server) {
			m_model.reset(nullptr);
		}
		else {
			m_model = std::make_unique<ServerMimeActionsModel>(server);
		}

		m_ui->actions->setModel(m_model.get());
	}

	void MimeActionsWidget::clear() {
		//		m_ui->fileExtensionMimeTypes->re
		//		for(int idx = m_ui->fileExtensionMimeTypes->topLevelItemCount() - 1; idx >= 0; --idx) {
		//			auto * item = m_ui->fileExtensionMimeTypes->takeTopLevelItem(idx);
		//			Q_ASSERT_X(item->type() == FileAssociationExtensionItem::ItemType, __PRETTY_FUNCTION__, "found top-level item that is not an extension type");
		//			Q_EMIT extensionRemoved(static_cast<FileAssociationExtensionItem *>(item)->extension());
		//		}
	}

}  // namespace EquitWebServer
