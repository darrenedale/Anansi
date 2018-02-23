/// \file mimeactionsitemdelegate.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Implementation of the MimeActionsItemDelegate class.
///
/// \dep
/// - mimeactionsitemdelegate.h
/// - QLineEdit
/// - configuration.h
/// - mimeactionswidget.h
/// - webserveractioncombo.h
///
/// \todo scoped enums and QVariants don't mix. need to do some ugly
/// casting to make them work together
///
/// \par Changes
/// - (2018-02) first version of this file.

#include "mimeactionsitemdelegate.h"

#include <iostream>
#include <QLineEdit>

#include "configuration.h"
#include "filenamewidget.h"
#include "mimeactionswidget.h"
#include "webserveractioncombo.h"
#include "servermimeactionsmodel.h"


Q_DECLARE_METATYPE(EquitWebServer::Configuration::WebServerAction);


namespace EquitWebServer {


	MimeActionsItemDelegate::MimeActionsItemDelegate(MimeActionsWidget * parent)
	: QStyledItemDelegate(parent),
	  m_parent(parent) {
	}


	QWidget * MimeActionsItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex & index) const {
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


	void MimeActionsItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
		if(!index.isValid()) {
			return;
		}

		switch(index.column()) {
			case ServerMimeActionsModel::ActionColumnIndex: {
				auto * combo = qobject_cast<WebServerActionCombo *>(editor);
				Q_ASSERT_X(combo, __PRETTY_FUNCTION__, "expected editor to be a WebServerActionCombo");
				combo->setWebServerAction(index.data(Qt::EditRole).value<Configuration::WebServerAction>());
				break;
			}

			case ServerMimeActionsModel::CgiColumnIndex: {
				auto * fileNameWidget = qobject_cast<FileNameWidget *>(editor);
				Q_ASSERT_X(fileNameWidget, __PRETTY_FUNCTION__, "expected editor to be a FileNameWidget");
				fileNameWidget->setFileName(index.data(Qt::EditRole).value<QString>());
				break;
			}
		}

		QStyledItemDelegate::setEditorData(editor, index);
	}


	void MimeActionsItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const {
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


	MimeActionsItemDelegate::~MimeActionsItemDelegate() = default;


}  // namespace EquitWebServer
