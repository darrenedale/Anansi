#include "src/mimetypeactionswidget.h"
#include "ui_mimetypeactionswidget.h"

#include <iostream>

#include <QMenu>
#include <QPushButton>
#include <QWidgetAction>

#include "server.h"
#include "servermimeactionsmodel.h"
#include "mimecombo.h"
#include "mimecombowidgetaction.h"
#include "mimetypeactionsdelegate.h"


Q_DECLARE_METATYPE(EquitWebServer::WebServerAction)


namespace EquitWebServer {


	MimeTypeActionsWidget::MimeTypeActionsWidget(QWidget * parent)
	: QWidget(parent),
	  m_model(nullptr),
	  m_ui(std::make_unique<Ui::MimeActionsWidget>()),
	  m_server(nullptr),
	  m_addMimeCombo(nullptr) {
		m_ui->setupUi(this);
		m_ui->actions->setItemDelegate(new MimeTypeActionsDelegate(this));

		auto * addEntryMenu = new QMenu(this);
		auto * action = new MimeComboWidgetAction(this);
		m_addMimeCombo = action->mimeCombo();
		addEntryMenu->addAction(action);
		m_ui->add->setMenu(addEntryMenu);

		connect(action, &MimeComboWidgetAction::addMimeTypeClicked, [this](const QString & mimeType) {
			const auto idx = m_model->addMimeType(mimeType, m_ui->defaultAction->webServerAction(), {});

			if(!idx.isValid()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to add MIME type \"" << qPrintable(mimeType) << "\" with action = " << enumeratorString(m_ui->defaultAction->webServerAction()) << " to MIME type actions list. is it already present?\n";
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
			const auto action = m_model->index(row, ServerMimeActionsModel::ActionColumnIndex).data().value<WebServerAction>();

			if(m_model->removeRows(idx.row(), 1, {})) {
				if(WebServerAction::CGI == action) {
					Q_EMIT mimeTypeActionRemoved(mimeType, action, m_model->index(row, ServerMimeActionsModel::CgiColumnIndex).data().value<QString>());
				}
				else {
					Q_EMIT mimeTypeActionRemoved(mimeType, action);
				}
			}
		});

		connect(m_ui->defaultAction, &WebServerActionCombo::webServerActionChanged, [this](WebServerAction action) {
			// can be null while setting up UI
			if(!m_server) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: server not yet set\n"
							 << std::flush;
				return;
			}

			m_server->configuration().setDefaultAction(action);
			Q_EMIT defaultActionChanged(action);
		});
	}


	MimeTypeActionsWidget::MimeTypeActionsWidget(Server * server, QWidget * parent)
	: MimeTypeActionsWidget(parent) {
		setServer(server);
	}


	MimeTypeActionsWidget::~MimeTypeActionsWidget() = default;


	void MimeTypeActionsWidget::setServer(Server * server) {
		std::array<QSignalBlocker, 2> blocks = {{QSignalBlocker(m_ui->defaultAction), QSignalBlocker(m_ui->actions)}};
		m_server = server;
		m_addMimeCombo->clear();

		if(!server) {
			m_model.reset(nullptr);
			m_ui->defaultAction->setWebServerAction(WebServerAction::Ignore);
		}
		else {
			m_model = std::make_unique<ServerMimeActionsModel>(server);
			m_ui->defaultAction->setWebServerAction(server->configuration().defaultAction());

			// TODO only add those not already in list?
			for(const auto & mimeType : server->configuration().allKnownMimeTypes()) {
				m_addMimeCombo->addMimeType(mimeType);
			}
		}

		auto * selectionModel = m_ui->actions->selectionModel();

		if(selectionModel) {
			selectionModel->disconnect(this);
		}

		m_ui->actions->setModel(m_model.get());
		selectionModel = m_ui->actions->selectionModel();

		if(selectionModel) {
			connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &MimeTypeActionsWidget::onActionsSelectionChanged, Qt::UniqueConnection);
		}

		m_ui->actions->resizeColumnToContents(ServerMimeActionsModel::MimeTypeColumnIndex);
		m_ui->actions->resizeColumnToContents(ServerMimeActionsModel::ActionColumnIndex);
		m_ui->actions->resizeColumnToContents(ServerMimeActionsModel::CgiColumnIndex);

		// edit combo needs a bit more space, usually
		m_ui->actions->setColumnWidth(ServerMimeActionsModel::ActionColumnIndex, m_ui->actions->columnWidth(ServerMimeActionsModel::ActionColumnIndex) + 25);
	}


	WebServerAction MimeTypeActionsWidget::defaultAction() const {
		return m_ui->defaultAction->webServerAction();
	}


	void MimeTypeActionsWidget::setDefaultAction(WebServerAction action) {
		if(action == defaultAction()) {
			return;
		}

		m_ui->defaultAction->setWebServerAction(action);
		Q_EMIT defaultActionChanged(action);
	}


	void MimeTypeActionsWidget::clear() {
		//		m_ui->fileExtensionMimeTypes->re
		//		for(int idx = m_ui->fileExtensionMimeTypes->topLevelItemCount() - 1; idx >= 0; --idx) {
		//			auto * item = m_ui->fileExtensionMimeTypes->takeTopLevelItem(idx);
		//			Q_ASSERT_X(item->type() == FileAssociationExtensionItem::ItemType, __PRETTY_FUNCTION__, "found top-level item that is not an extension type");
		//			Q_EMIT extensionRemoved(static_cast<FileAssociationExtensionItem *>(item)->extension());
		//		}
	}


	void MimeTypeActionsWidget::onActionsSelectionChanged() {
		auto * selectionModel = m_ui->actions->selectionModel();
		m_ui->remove->setEnabled(selectionModel && !selectionModel->selectedIndexes().isEmpty());
	}

}  // namespace EquitWebServer
