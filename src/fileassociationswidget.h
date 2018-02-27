#ifndef EQUITWEBSERVER_FILEASSOCIATIONSWIDGET_H
#define EQUITWEBSERVER_FILEASSOCIATIONSWIDGET_H

#include <memory>

#include <QWidget>

#include "configuration.h"

class QSortFilterProxyModel;

namespace EquitWebServer {

	class Server;
	class ServerFileAssociationsModel;

	namespace Ui {
		class FileAssociationsWidget;
	}

	class FileAssociationsWidget : public QWidget {
		Q_OBJECT

	public:
		explicit FileAssociationsWidget(QWidget * parent = nullptr);
		explicit FileAssociationsWidget(Server * server, QWidget * parent = nullptr);
		~FileAssociationsWidget();

		void setServer(Server * server);

		bool hasExtension(const QString & ext) const;
		bool extensionHasMimeType(const QString & ext, const QString & mimeType) const;

		std::vector<QString> availableMimeTypes() const;

		QString defaultMimeType() const;

		QString currentExtension() const;
		QString selectedExtension() const;
		std::vector<QString> selectedExtensions() const;

		QString currentMimeType() const;
		QString selectedMimeType() const;
		std::vector<QString> selectedMimeTypes() const;

		void clear();

	Q_SIGNALS:
		void defaultMimeTypeChanged(const QString &);
		void currentExtensionChanged(const QString &);
		void currentExtensionMimeTypeChanged(const QString & ext, const QString & mimeType);

		void extensionAdded(const QString &);
		void extensionRemoved(const QString &);
		void extensionChanged(const QString & oldExt, const QString & newExt);

		void extensionMimeTypeAdded(const QString & ext, const QString & mimeType);
		void extensionMimeTypeRemoved(const QString & ext, const QString & mimeType);
		void extensionMimeTypeChanged(const QString & ext, const QString & oldMimeType, const QString & newMimeType);

	public Q_SLOTS:
		void addAvailableMimeType(const QString & mimeType);

		bool addExtension(const QString & ext);
		bool addExtensionMimeType(const QString & ext, const QString & mimeType);

		void removeExtension(const QString & ext);
		void removeExtensionMimeType(const QString & ext, const QString & mimeType);

		inline void removeCurrentExtension() {
			auto ext = currentExtension();

			if(ext.isEmpty()) {
				return;
			}

			removeExtension(ext);
		}

		inline void removeCurrentMimeType() {
			auto ext = currentExtension();

			if(ext.isEmpty()) {
				return;
			}

			auto mimeType = currentMimeType();

			if(mimeType.isEmpty()) {
				return;
			}

			removeExtensionMimeType(ext, mimeType);
		}

		bool setCurrentExtension(const QString & ext);
		bool setCurrentExtensionMimeType(const QString & ext, const QString & mimeType);
		void setDefaultMimeType(const QString & mimeType);

	private Q_SLOTS:
		void onFileExtensionsSelectionChanged();

	private:
		std::unique_ptr<ServerFileAssociationsModel> m_model;
		std::unique_ptr<Ui::FileAssociationsWidget> m_ui;
		Server * m_server;  // observed only
	};


}  // namespace EquitWebServer
#endif  // EQUITWEBSERVER_FILEASSOCIATIONSWIDGET_H
