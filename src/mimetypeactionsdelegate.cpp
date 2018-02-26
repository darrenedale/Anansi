/// \file mimetypeactionsdelegate.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Implementation of the MimeActionsItemDelegate class.
///
/// \dep
/// - mimetypeactionsdelegate.h
/// - QLineEdit
/// - configuration.h
/// - mimeactionswidget.h
/// - webserveractioncombo.h
///
/// \par Changes
/// - (2018-02) first version of this file.

#include "mimetypeactionsdelegate.h"

#include <iostream>
#include <QLineEdit>

#include "configuration.h"
#include "filenamewidget.h"
#include "mimetypeactionswidget.h"
#include "webserveractioncombo.h"
#include "servermimeactionsmodel.h"


Q_DECLARE_METATYPE(EquitWebServer::WebServerAction);


namespace EquitWebServer {


	MimeTypeActionsDelegate::MimeTypeActionsDelegate(MimeTypeActionsWidget * parent)
	: QStyledItemDelegate(parent),
	  m_parent(parent) {
	}


	QWidget * MimeTypeActionsDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex & index) const {
		if(!index.isValid()) {
			return nullptr;
		}

		switch(index.column()) {
			case ServerMimeActionsModel::MimeTypeColumnIndex:
				return nullptr;

			case ServerMimeActionsModel::ActionColumnIndex:
				return new WebServerActionCombo(parent);

			case ServerMimeActionsModel::CgiColumnIndex:
				return new FileNameWidget(parent);
		}

		return nullptr;
	}


	void MimeTypeActionsDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
		if(!index.isValid()) {
			return;
		}

		switch(index.column()) {
			case ServerMimeActionsModel::ActionColumnIndex: {
				auto * combo = qobject_cast<WebServerActionCombo *>(editor);
				Q_ASSERT_X(combo, __PRETTY_FUNCTION__, "expected editor to be a WebServerActionCombo");
				combo->setWebServerAction(index.data(Qt::EditRole).value<WebServerAction>());
				return;
			}

			case ServerMimeActionsModel::CgiColumnIndex: {
				auto * fileNameWidget = qobject_cast<FileNameWidget *>(editor);
				Q_ASSERT_X(fileNameWidget, __PRETTY_FUNCTION__, "expected editor to be a FileNameWidget");
				fileNameWidget->setFileName(index.data(Qt::EditRole).value<QString>());
				return;
			}
		}

		QStyledItemDelegate::setEditorData(editor, index);
	}


	void MimeTypeActionsDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const {
		if(!index.isValid()) {
			return;
		}

		switch(index.column()) {
			case ServerMimeActionsModel::ActionColumnIndex: {
				auto * combo = qobject_cast<WebServerActionCombo *>(editor);
				Q_ASSERT_X(combo, __PRETTY_FUNCTION__, "expected editor to be a WebServerActionCombo");
				model->setData(index, QVariant::fromValue(combo->webServerAction()));
				break;
			}

			case ServerMimeActionsModel::CgiColumnIndex: {
				auto * fileName = qobject_cast<FileNameWidget *>(editor);
				Q_ASSERT_X(fileName, __PRETTY_FUNCTION__, "expected editor to be a FileNameWidget");
				model->setData(index, fileName->fileName());
				break;
			}
		}
	}


	MimeTypeActionsDelegate::~MimeTypeActionsDelegate() = default;


}  // namespace EquitWebServer
