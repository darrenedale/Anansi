#ifndef EQUITWEBSERVER_FILEASSOCIATIONSWIDGET_H
#define EQUITWEBSERVER_FILEASSOCIATIONSWIDGET_H

#include <memory>

#include <QWidget>

#include "configuration.h"

namespace EquitWebServer {

	namespace Ui {
		class FileAssociationsWidget;
	}

	class FileAssociationExtensionItem;
	class FileAssociationMimeTypeItem;

	class FileAssociationsWidget : public QWidget {
		Q_OBJECT

	public:
		static constexpr const int DelegateItemTypeRole = Qt::UserRole + 9001;
		static constexpr const int DelegateItemDataRole = Qt::UserRole + 9002;
		static constexpr const int DelegateItemOldDataRole = Qt::UserRole + 9003;

		explicit FileAssociationsWidget(QWidget * parent = nullptr);
		~FileAssociationsWidget();

		inline bool hasExtension(const QString & ext) const {
			return nullptr != findExtensionItem(ext);
		}

		inline bool extensionHasMimeType(const QString & ext, const QString & mimeType) const {
			return nullptr != findMimeTypeItem(ext, mimeType);
		}

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
		void addExtension(const QString & ext);
		void addExtensionMimeType(const QString & ext, const QString & mimeType);

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

	private:
		FileAssociationExtensionItem * findExtensionItem(const QString & ext) const;
		FileAssociationMimeTypeItem * findMimeTypeItem(const QString & ext, const QString & mimeType) const;

		std::unique_ptr<Ui::FileAssociationsWidget> m_ui;
	};


}  // namespace EquitWebServer
#endif  // EQUITWEBSERVER_FILEASSOCIATIONSWIDGET_H
