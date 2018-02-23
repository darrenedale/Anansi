#include "src/mimeactionswidget.h"
#include "ui_mimeactionswidget.h"

#include <iostream>

#include <QMenu>
#include <QPushButton>
#include <QWidgetAction>

#include "server.h"
#include "servermimeactionsmodel.h"
#include "mimetypecombo.h"
#include "mimetypecomboaction.h"
#include "mimeactionsitemdelegate.h"


Q_DECLARE_METATYPE(EquitWebServer::Configuration::WebServerAction)


namespace EquitWebServer {


	MimeActionsWidget::MimeActionsWidget(QWidget * parent)
	: QWidget(parent),
	  m_ui(std::make_unique<Ui::MimeActionsWidget>()) {
		m_ui->setupUi(this);
		m_ui->actions->setItemDelegate(new MimeActionsItemDelegate(this));


		auto * addEntryMenu = new QMenu(this);
		auto * action = new MimeTypeComboAction(this);
		addEntryMenu->addAction(action);
		// TODO add MIME types to combo
		m_ui->add->setMenu(addEntryMenu);

		connect(action, &MimeTypeComboAction::addMimeTypeClicked, [this](const QString & mimeType) {
			const auto idx = m_model->addMimeType(mimeType, m_ui->defaultAction->webServerAction(), {});

			if(!idx.isValid()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to add MIME type \"" << qPrintable(mimeType) << "\" with action = " << static_cast<int>(m_ui->defaultAction->webServerAction()) << " to MIME type actions list. is it already present?\n";
				return;
			}

			m_ui->actions->edit(idx);
		});

		connect(m_ui->remove, &QPushButton::clicked, [this]() {
			const auto idx = m_ui->actions->currentIndex();

			if(!idx.isValid()) {
				return;
			}

			const auto row = idx.row();
			const auto mimeType = m_model->index(row, ServerMimeActionsModel::MimeTypeColumnIndex).data().value<QString>();
			const auto action = m_model->index(row, ServerMimeActionsModel::ActionColumnIndex).data().value<Configuration::WebServerAction>();

			if(m_model->removeRows(idx.row(), 1, {})) {
				if(Configuration::WebServerAction::CGI == action) {
					Q_EMIT mimeTypeActionRemoved(mimeType, action, m_model->index(row, ServerMimeActionsModel::CgiColumnIndex).data().value<QString>());
				}
				else {
					Q_EMIT mimeTypeActionRemoved(mimeType, action);
				}
			}
		});

		connect(m_ui->defaultAction, &WebServerActionCombo::webServerActionChanged, this, &MimeActionsWidget::defaultMimeTypeActionChanged);
	}


	MimeActionsWidget::MimeActionsWidget(Server * server, QWidget * parent)
	: MimeActionsWidget(parent) {
		setServer(server);
	}


	MimeActionsWidget::~MimeActionsWidget() = default;


	void MimeActionsWidget::setServer(Server * server) {
		QSignalBlocker block(this);

		if(!server) {
			m_model.reset(nullptr);
			m_ui->defaultAction->setWebServerAction(Configuration::WebServerAction::Ignore);
		}
		else {
			m_model = std::make_unique<ServerMimeActionsModel>(server);
			m_ui->defaultAction->setWebServerAction(server->configuration().defaultAction());
		}

		m_ui->actions->setModel(m_model.get());
		m_ui->actions->resizeColumnToContents(ServerMimeActionsModel::MimeTypeColumnIndex);
		m_ui->actions->resizeColumnToContents(ServerMimeActionsModel::ActionColumnIndex);
		m_ui->actions->resizeColumnToContents(ServerMimeActionsModel::CgiColumnIndex);
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
