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
/// \par Changes
/// - (2018-02) first version of this file.

#include "mimeactionsitemdelegate.h"

#include <iostream>
#include <QLineEdit>

#include "configuration.h"
#include "filenamewidget.h"
#include "mimeactionswidget.h"
#include "webserveractioncombo.h"


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

		// TODO use column indices from model
		switch(index.column()) {
			case 0:
				return nullptr;

			case 1:
				return new WebServerActionCombo(parent);

			case 2:
				// TODO implement a filename widget - line edit and button to spawn file picker
				return new FileNameWidget(parent);
		}

		return nullptr;
	}


	void MimeActionsItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
		if(!index.isValid()) {
			return;
		}

		// TODO use column indices from model
		switch(index.column()) {
			case 1: {
				auto * combo = qobject_cast<WebServerActionCombo *>(editor);
				Q_ASSERT_X(combo, __PRETTY_FUNCTION__, "expected editor to be a WebServerActionCombo");
				std::cout << "setting server action combo data to " << static_cast<int>(index.data().value<Configuration::WebServerAction>()) << "\n"
							 << std::flush;
				combo->setWebServerAction(index.data().value<Configuration::WebServerAction>());
				break;
			}

			case 2: {
				auto * fileNameWidget = qobject_cast<FileNameWidget *>(editor);
				Q_ASSERT_X(fileNameWidget, __PRETTY_FUNCTION__, "expected editor to be a FileNameWidget");
				fileNameWidget->setFileName(index.data().value<QString>());
				break;
			}
		}

		QStyledItemDelegate::setEditorData(editor, index);
	}


	void MimeActionsItemDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const {
		if(!index.isValid()) {
			return;
		}

		// TODO use column indices from model
		switch(index.column()) {
			case 1: {
				auto * combo = qobject_cast<WebServerActionCombo *>(editor);
				Q_ASSERT_X(combo, __PRETTY_FUNCTION__, "expected editor to be a WebServerActionCombo");
				model->setData(index, QVariant::fromValue(combo->webServerAction()));
				break;
			}

			case 2: {
				auto * fileName = qobject_cast<FileNameWidget *>(editor);
				Q_ASSERT_X(fileName, __PRETTY_FUNCTION__, "expected editor to be a FileNameWidget");
				model->setData(index, fileName->fileName());
				break;
			}
		}
	}


	MimeActionsItemDelegate::~MimeActionsItemDelegate() = default;


}  // namespace EquitWebServer
