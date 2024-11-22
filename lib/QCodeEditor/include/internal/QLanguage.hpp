#pragma once

// Qt
#include <QObject> // Required for inheritance
#include <QString>
#include <QMap>

class QIODevice;

/**
 * Class, that describes object for parsing
 * language file.
 */
class QLanguage : public QObject
{
  Q_OBJECT

public:
  /**
   * @brief Constructor.
   * @param parent Pointer to parent QObject.
   */
  explicit QLanguage(QIODevice *device = nullptr, QObject *parent = nullptr);

  /**
   * @brief Method for parsing.
   * @param device Pointer to device.
   * @return Success.
   */
  bool load(QIODevice *device);

  /**
   * @brief Method for getting available keys.
   */
  QStringList keys();

  /**
   * @brief Method for getting names
   * from key.
   * @param name
   * @return
   */
  QStringList names(const QString &key);

  /**
   * @brief Method for getting is object loaded.
   */
  bool isLoaded() const;

private:
  bool m_loaded;

  QMap<QString, QStringList> m_list;
};
