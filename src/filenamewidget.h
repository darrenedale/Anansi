/// \file filenamewidget.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the FileNameWidget class for EquitWebServer.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQUITWEBSERVER_FILENAMEWIDGET_H
#define EQUITWEBSERVER_FILENAMEWIDGET_H

#include <memory>

#include <QWidget>
#include <QString>

namespace EquitWebServer {

	namespace Ui {
		class FileNameWidget;
	}

	class FileNameWidget : public QWidget {
		Q_OBJECT

	public:
		explicit FileNameWidget(const QString & path, QWidget * parent = nullptr);
		explicit FileNameWidget(QWidget * parent = nullptr);
		~FileNameWidget();

		QString placeholderText() const;
		void setPlaceholderText(const QString &);

		inline void setDialogueCaption(const QString & caption) {
			m_dialogueCaption = caption;
		}

		inline const QString & dialogueCaption() const {
			return m_dialogueCaption;
		}

		void setFileName(const QString & path);
		QString fileName() const;

		inline void setFilter(const QString & filter) {
			m_dialogueFilter = filter;
		}

		inline const QString & filter() const {
			return m_dialogueFilter;
		}

	Q_SIGNALS:
		void fileNameChanged(const QString & path);

	public Q_SLOTS:
		void chooseFile(QString path = {});

	private:
		std::unique_ptr<Ui::FileNameWidget> m_ui;
		QString m_dialogueCaption;
		QString m_dialogueFilter;
	};


}  // namespace EquitWebServer
#endif  // EQUITWEBSERVER_FILENAMEWIDGET_H
